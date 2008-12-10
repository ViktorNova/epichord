/*
   Epichord - a midi sequencer
   Copyright (C) 2008  Evan Rinehart

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to

   The Free Software Foundation, Inc.
   51 Franklin Street, Fifth Floor
   Boston, MA  02110-1301, USA
*/

#include <iostream>
#include <fstream>
#include <vector>

#include <arpa/inet.h>
#include <string.h>

#include <fltk/file_chooser.h>
#include <fltk/filename.h>

#include "ui.h"

#include "backend.h"

#include "saveload.h"

extern UI* ui;

extern std::vector<track*> tracks;
extern pattern* patterns;

using namespace std;

std::string last_filename;
char last_dir[1024] = "";


int clear(){

  pause_backend();
  reset_backend(0);

  last_filename = "";

  track* t;
  int total = tracks.size();
  for(int i=0; i<total; i++){
    t = tracks[tracks.size()-1];
    tracks.pop_back();
    delete t;
  }
  pattern* p = patterns;
  pattern* next;
  while(p){
    next = p->next;
    delete p;
    p = next;
  }

  set_bpm(120);

  ui->title_text->text("");
  ui->author_text->text("");

  fltk::TextBuffer* tb = ui->info_text->buffer();
  tb->text("");

  ui->main_window->redraw();

}

void set_last_dir(const char* name){
  const char* theend = fltk::filename_name(name);
  int n = (int)(theend - name) / sizeof(char);
  memcpy(last_dir,name,n);
  last_dir[n] = '\0';
}

const char* get_last_dir(){
  return last_dir;
}

int save(){
  if(last_filename != ""){
    return save(last_filename.c_str());
  }
  else{
    return save(fltk::file_chooser("save file",NULL,last_dir));
  }
}



int save(const char* filename){

  //create file
  fstream file;
  file.open(filename, fstream::out);

  if(!file.is_open()){
    printf("error, cant open file for saving\n");
    return -1;
  }

  last_filename = filename;
  set_last_dir(filename);

  //write song info
  file << "title " << ui->title_text->size() << " "
                   << ui->title_text->text() << endl;
  file << "author " << ui->author_text->size() << " "
                   << ui->author_text->text() << endl;
  file << "info " << ui->info_text->size() << " "
                   << ui->info_text->text() << endl;

  file << "bpm " << ui->bpm_wheel->value() << endl;
  //file << "beatspermeasure" << 4 << endl;
  //file << "measuresperphrase" << 4 << endl;

  file << "loopstart " << get_loop_start() << endl;
  file << "loopend " << get_loop_end() << endl;

  file << endl;

  //for each pattern
  pattern* p = patterns->next;
  mevent* e;
  int P = 0;
  while(p){
    file << "pattern " << endl;
    e = p->events->next;
    mevent* eoff;
    int n = 0;
    int m = n;
    while(e){
      file << e->type << " ";
      file << e->tick << " ";
      file << e->value1 << " ";
      file << e->value2 << " ";
      file << e->dur << endl;
      /*if(e->off){
        eoff = p->events->next;
        m = 0;
        while(eoff != e->off){
          m++;
          eoff = eoff->next;
        }
        file << m << endl;
      }
      else{
        file << -1 << endl;
      }*/
      n++;
      e = e->next;
    }
    P++;
    file << -1 << endl << endl;
    p = p->next;
  }

  seqpat* s;
  for(int i=0; i<tracks.size(); i++){
    file << endl << endl << "track " << endl;
    file << "port " << tracks[i]->port << endl;
    file << "chan " << tracks[i]->chan << endl;
    file << "prog " << tracks[i]->prog << endl;
    file << "mute " << tracks[i]->mute << endl;
    file << "solo " << tracks[i]->solo << endl;
    file << "vol " << tracks[i]->vol << endl;
    file << "pan " << tracks[i]->pan << endl;
    file << "name " << strlen(tracks[i]->name) << " " 
                    << tracks[i]->name << endl;
    file << "alive " << tracks[i]->alive << endl;
    s = tracks[i]->head->next;
    int m = 0;
    while(s){
      file << endl << endl << "seqpat " << endl;
      file << s->tick << " " << s->dur << endl;
      for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
          file << (int)s->color[i][j] << " ";
        }
      }
      file << endl;
      file << s->scrollx << " " << s->scrolly << endl;
      p = patterns->next;
      int q = 0;
      while(p != s->p){
        q++;
        p = p->next;
      }
      file << q << endl;
      m++;
      s = s->next;
    }
    file << "kcart";

  }


  file.close();

  return 0;
}



//noteoff pointers are broken after loading, fix them
void repair(){
  seqpat* s;
  mevent* e1;
  mevent* e2;
  mevent* prev;

  pattern* p = patterns->next;

  int n,m;

  while(p){
    e1 = p->events;
    prev = e1;
    n=-1;
    while(e1){
      prev = e1;
      /*if(e1->type == MIDI_NOTE_ON){
        e2 = e1;
        for(m=n; m < e1->off_index; m++){
          e2 = e2->next;
        }
        e1->off = e2;
      }*/
      n++;
      e1 = e1->next;
      if(e1){
        e1->prev = prev;
      }
    }
    p = p->next;
  }

  seqpat* sprev;
  for(int i=0; i<tracks.size(); i++){
    s = tracks[i]->head;
    while(s){
      sprev = s;
      s = s->next;
      if(s){
        s->prev = sprev;
      }
    }
  }

}


int load(const char* filename){

  ifstream file;
  string str;
  file.open(filename, fstream::in);

  if(!file.is_open()){
    printf("error, cant open file for saving\n");
    return -1;
  }

  clear();
  last_filename = filename;
  set_last_dir(filename);


  patterns = new pattern();

  pattern* pend = patterns;

  while(!file.eof()){
    file >> str;
    int n;

    if(str == "title"){
      file >> n;
      file.get();
      char buf[256];
      file.read(buf,n);
      buf[n] = '\0';
      ui->title_text->text(buf);
    }
    else if(str == "author"){
      file >> n;
      file.get();
      char buf[256];
      file.read(buf,n);
      buf[n] = '\0';
      ui->author_text->text(buf);
    }
    else if(str == "info"){
      file >> n;
      file.get();
      fltk::TextBuffer* tb = ui->info_text->buffer();
      char buf[1024];
      file.read(buf,n);
      buf[n] = '\0';
      tb->text(buf);
    }
    else if(str == "bpm"){
      int bpm;
      file >> bpm;
      set_bpm(bpm);
      ui->bpm_wheel->value(bpm);
      ui->bpm_output->value(bpm);
    }
    else if(str == "loopstart"){
      int ls;
      file >> ls;
      set_loop_start(ls);
    }
    else if(str == "loopend"){
      int le;
      file >> le;
      set_loop_end(le);
    }
    else if(str == "pattern"){
      pattern* p = new pattern();
      mevent* eend = p->events;
      int off_index;
      int type;
      mevent* e;
      while(1){
        file >> type;
        if(type == -1){break;}
        e = new mevent();
        e->type = type;
        file >> e->tick;
        file >> e->value1;
        file >> e->value2;
        file >> e->dur;
        //file >> e->off_index;
        eend->next = e;
        eend = e;
      }
      pend->next = p;
      pend = p;
    }
    else if(str == "track"){
      int track_number;
      string key;
      track* t = new track();
      t->head->track = tracks.size();
      seqpat* send = t->head;
      file >> key;

      while(key != "kcart"){
        if(key == "port"){ file >> t->port; }
        else if(key == "chan"){ file >> t->chan; }
        else if(key == "prog"){ file >> t->prog; }
        else if(key == "mute"){ file >> t->mute; }
        else if(key == "solo"){ file >> t->solo; }
        else if(key == "vol"){ file >> t->vol; }
        else if(key == "pan"){ file >> t->pan; }
        else if(key == "name"){
          file >> n;
          file.get();
          char buf[256];
          file.read(buf,n);
          buf[n] = '\0';
          strncpy(t->name,buf,256);
        }
        else if(key == "alive"){ file >> t->alive; }
        else if(key == "seqpat"){

          int pattern_number;
          seqpat* s = new seqpat();
          s->track = tracks.size();
          file >> s->tick;
          file >> s->dur;
          int C;
          for(int i=0; i<3; i++){
            for(int j=0; j<3; j++){
              file >> C;
              s->color[i][j] = C;
            }
          }
          file >> s->scrollx >> s->scrolly;
          file >> pattern_number;
          pattern* p = patterns->next;
          int n = 0;
          while(n++ < pattern_number){
            p = p->next;
          }
          s->p = p;
          send->next = s;
          send = s;
        }
        file >> key;
      }
      tracks.push_back(t);
    }
  }

  repair();

  ui->track_info->update();

  ui->arranger->redraw();

  reset_backend(0);

  return 0;

}



int tick2delta(unsigned tick, vector<unsigned char>& vbuf){
  unsigned char buf[4];
  uint32_t n;
  if(tick < 128){
    vbuf.push_back(tick);
    return 1;
  }
  else if(tick < 16383){
    n = htonl(tick);
    memcpy(buf,&n,4);
    vbuf.push_back(0x80|((buf[0]<<1)|(buf[1]&0x80)>>7));
    vbuf.push_back(buf[1]&0x7f);
    return 2;
  }
  else if(tick < 2097151){
    n = htonl(tick);
    memcpy(buf,&n,4);
    vbuf.push_back(0x80|((buf[0]<<2)|(buf[1]&0xC0)>>6));
    vbuf.push_back(0x80|((buf[1]<<1)|(buf[2]&0x80)>>7));
    vbuf.push_back(buf[2]&0x7f);
    return 3;
  }
  else{
    n = htonl(tick);
    memcpy(buf,&n,4);
    vbuf.push_back(0x80|((buf[0]<<3)|(buf[1]&0xE0)>>5));
    vbuf.push_back(0x80|((buf[1]<<2)|(buf[2]&0xC0)>>6));
    vbuf.push_back(0x80|((buf[2]<<1)|(buf[3]&0x80)>>7));
    vbuf.push_back(buf[3]&0x7f);
    return 4;
  }
}

int savesmf(const char* filename){

  fstream file;
  file.open(filename, fstream::out);

  if(!file.is_open()){
    printf("error, cant open file for saving\n");
    return -1;
  }

  unsigned char buf[64];

  //chunk id
  buf[0] = 'M';
  buf[1] = 'T';
  buf[2] = 'h';
  buf[3] = 'd';

  //chunk size
  buf[4] = 0x00;
  buf[5] = 0x00;
  buf[6] = 0x00;
  buf[7] = 0x06;

  //type 1 midi file
  buf[8] = 0x00;
  buf[9] = 0x01;

  //number of tracks
  uint16_t L = tracks.size();
  L = htons(L);
  memcpy(buf+10,&L,2);

  //128 ticks per beat, 128 | 0x0000
  buf[12] = 0x00;
  buf[13] = 0x80;

  file.write((const char*)buf,14);

  vector<unsigned char> vbuf;
  int last_tick = 0;
  uint32_t N = 0;
  int MAX = tracks.size();

  for(int i=0; i<MAX; i++){

    //chunk id
    N = 0;
    last_tick = 0;
    vbuf.clear();
    vbuf.push_back('M');
    vbuf.push_back('T');
    vbuf.push_back('r');
    vbuf.push_back('k');

    //chunk size (yet unknown)
    
    vbuf.push_back(0x00);
    vbuf.push_back(0x00);
    vbuf.push_back(0x00);
    vbuf.push_back(0x00);

    track* t = tracks[i];
    seqpat* s;
    pattern* p;
    mevent* e;

    //change program for this track

    //write events
    s = t->head->next;

    while(s){
      e = s->p->events->next;

      while(e){

        if(e->tick > s->dur){
          break;
        }
        switch(e->type){
          case 0x80://note off
            N += tick2delta(e->tick+s->tick - last_tick,vbuf);
            last_tick = e->tick+s->tick;
            vbuf.push_back(0x80 | t->chan);
            vbuf.push_back(e->value1);
            vbuf.push_back(e->value2);
            N += 3;
            break;
          case 0x90://note on
            N += tick2delta(e->tick+s->tick - last_tick,vbuf);
            last_tick = e->tick+s->tick;
            vbuf.push_back(0x90 | t->chan);
            vbuf.push_back(e->value1);
            vbuf.push_back(e->value2);
            N += 3;
            break;
          case 0xA0://after touch
            N += tick2delta(e->tick+s->tick - last_tick,vbuf);
            last_tick = e->tick+s->tick;
            vbuf.push_back(0xA0 | t->chan);
            vbuf.push_back(e->value1);
            vbuf.push_back(e->value2);
            N += 3;
            break;
          case 0xB0://control change
            N += tick2delta(e->tick+s->tick - last_tick,vbuf);
            last_tick = e->tick+s->tick;
            vbuf.push_back(0xB0 | t->chan);
            vbuf.push_back(e->value1);
            vbuf.push_back(e->value2);
            N += 3;
            break;
          case 0xC0://program change
            N += tick2delta(e->tick+s->tick - last_tick,vbuf);
            last_tick = e->tick+s->tick;
            vbuf.push_back(0xC0 | t->chan);
            vbuf.push_back(e->value1);
            N += 2;
            break;
          case 0xD0://channel pressure
            N += tick2delta(e->tick+s->tick - last_tick,vbuf);
            last_tick = e->tick+s->tick;
            vbuf.push_back(0xD0 | t->chan);
            vbuf.push_back(e->value1);
            N += 2;
            break;
          case 0xE0://pitch wheel
            N += tick2delta(e->tick+s->tick - last_tick,vbuf);
            last_tick = e->tick+s->tick;
            vbuf.push_back(0xE0 | t->chan);
            vbuf.push_back(e->value1);
            vbuf.push_back(e->value2);
            N += 3;
            break;
        }
        e = e->next;
        
      }
      s = s->next;
    }

    //end of track meta event
    vbuf.push_back(0x00);
    vbuf.push_back(0xff);
    vbuf.push_back(0x2f);
    vbuf.push_back(0x00);
    N += 4;

    N = htonl(N);
    memcpy(buf,&N,4);
    vbuf[4] = buf[0];
    vbuf[5] = buf[1];
    vbuf[6] = buf[2];
    vbuf[7] = buf[3];

    for(int i=0; i<vbuf.size(); i++){
      buf[0] = vbuf[i];
      file.write((const char*)buf,1);
    }
  }

}


