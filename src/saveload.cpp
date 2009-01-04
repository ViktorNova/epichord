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
#include <list>

#include <arpa/inet.h>
#include <string.h>

#include <fltk/file_chooser.h>
#include <fltk/filename.h>

#include "ui.h"

#include "backend.h"

#include "saveload.h"

#include "util.h"

#include "uihelper.h"

#include "backend.h"

extern UI* ui;

extern std::vector<track*> tracks;


extern struct conf config;

using namespace std;

std::string last_filename;
char last_dir[1024] = "";


void nextline(ifstream& f){
  char dummy[512];
  f.getline(dummy,512);
  printf("%s\n",dummy);
}


std::list<pattern*> collectpatterns(){
  std::list<pattern*> patlist;

  for(int i=0; i<tracks.size(); i++){
    seqpat* s = tracks[i]->head->next;
    while(s){
      if(s->layers){
        for(int j=0; j<s->layers->total; j++){
          pattern* p = s->layers->array[j];
          patlist.push_back(p);
        }
      }
      else{
        patlist.push_back(s->p);
      }
      s=s->next;
    }
  }

  patlist.sort();
  patlist.unique();

  return patlist;
}

int findpatternindex(pattern* p, std::list<pattern*>& patlist){
  std::list<pattern*>::iterator i = patlist.begin();
  int q = 0;
  while(i != patlist.end()){
    if(*i==p){
      return q;
    }
    q++;
    i++;
  }
  return -1;
}

pattern* findpatternbyindex(int index, std::list<pattern*>& patlist){
  std::list<pattern*>::iterator i = patlist.begin();
  int q = 0;
  while(i != patlist.end()){
    if(q==index){
      return *i;
    }
    q++;
    i++;
  }
  return NULL;
}


int clear(){

  pause_backend();
  reset_backend(0);

  last_filename = "";

  set_beats_per_measure(4);
  set_measures_per_phrase(4);
  update_config_gui();

  track* t;
  int total = tracks.size();

  for(int i=0; i<total; i++){
    t = tracks[tracks.size()-1];
    tracks.pop_back();
    delete t;
  }

  set_bpm(120);

  ui->title_text->text("");
  ui->author_text->text("");

  fltk::TextBuffer* tb = ui->info_text->buffer();
  tb->text("");

  ui->track_info->clear_tracks();

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

  if(filename == NULL){
    return -1;
  }

  //create file
  fstream file;
  file.open(filename, fstream::out);

  if(!file.is_open()){
    printf("error, cant open file for saving\n");
    return -1;
  }

  last_filename = filename;
  set_last_dir(filename);

  //header to protect against accidentally opening wrong file
  file << "J2ULJwCgwHA" << endl;
  file << "epichord" << endl;
  file << "fileversion " << FILE_VERSION << endl;
  file << "ticksperbeat " << TICKS_PER_BEAT << endl << endl;

  //write song info
  file << "title " << ui->title_text->size() << " "
                   << ui->title_text->text() << endl;
  file << "author " << ui->author_text->size() << " "
                   << ui->author_text->text() << endl;
  file << "info " << ui->info_text->size() << " "
                   << ui->info_text->text() << endl;

  file << "bpm " << ui->bpm_wheel->value() << endl;
  file << "beatspermeasure " << config.beats_per_measure << endl;
  file << "measuresperphrase " << config.measures_per_phrase << endl;

  file << "loopstart " << get_loop_start() << endl;
  file << "loopend " << get_loop_end() << endl;

  file << endl;


  //collect all visible patterns
  std::list<pattern*> patlist = collectpatterns();

  //for each pattern
  std::list<pattern*>::iterator p = patlist.begin();
  mevent* e;
  int P = 0;
  while(p != patlist.end()){
    file << "pattern " << endl;
    file << (*p)->h << " " << (*p)->s << " " << (*p)->v << endl;
    e = (*p)->events->next;
    mevent* eoff;
    int n = 0;
    int m = n;
    int last_tick = 0;
    int delta;
    while(e){
      delta = e->tick - last_tick;
      last_tick = e->tick;
      file << delta << " ";
      file << e->type << " ";
      file << e->value1 << " ";
      file << e->value2 << endl;
      //file << e->dur << endl;
      n++;
      e = e->next;
    }
    P++;
    file << -1 << endl << endl;
    p++;
  }

  seqpat* s;
  for(int i=0; i<tracks.size(); i++){
    file << endl << endl << "track " << endl;
    file << "port " << tracks[i]->port << endl;
    file << "chan " << tracks[i]->chan << endl;
    file << "prog " << tracks[i]->prog << endl;
    file << "bank " << tracks[i]->bank << endl;
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
      file << s->scrollx << " " << s->scrolly << endl;

      if(s->layers){
        file << s->layers->total << " ";
        for(int j=0; j<s->layers->total; j++){

          int index = findpatternindex(s->layers->array[j],patlist);
          if(index < 0){
            printf("save: pattern not found, cannot save\n");
            file.close();
            return -1;
          }
          file << index << " ";

        }
        file << s->layers->index;
      }
      else{
          file << 1 << " ";
          int index = findpatternindex(s->p,patlist);
          if(index < 0){
            printf("save: pattern not found, cannot save\n");
            file.close();
            return -1;
          }
        file << index;
      }

      file << endl;
      m++;
      s = s->next;
    }
    file << "kcart";

  }


  file.close();

  return 0;
}




int load(const char* filename){


  if(filename == NULL){
    return -1;
  }

  ifstream file;
  string str;
  file.open(filename, fstream::in);

  if(!file.is_open()){
    printf("error, cant open file for saving\n");
    return -1;
  }

  show_song_edit();

  clear();
  last_filename = filename;
  set_last_dir(filename);

  //pattern* pend = patterns;
  std::list<pattern*> patlist;


  //sanity check
  file >> str;
  if(str != "J2ULJwCgwHA"){
    printf("load: this is definitely not a valid file (missing magic)\n");
    file.close();
    return -1;
  }
  file >> str;
  if(str != "epichord"){
    printf("load: this is definitely not a valid file (missing magic)\n");
    file.close();
    return -1;
  }
  file >> str;
  if(str != "fileversion"){
    printf("load: this is definitely not a valid file\n");
    file.close();
    return -1;
  }
  int M;
  file >> M;
  if(M!=FILE_VERSION){
    printf("load: file has wrong version %d (need %d).\n",M,FILE_VERSION);
  }
  file >> str;
  if(str != "ticksperbeat"){
    printf("load: file is broken. (missing ticks per beat)\n");
    file.close();
    return -1;
  }
  int file_tpb;
  file >> file_tpb;
  if(file_tpb < 1){
    printf("load: file is broken. (bad ticks per beat %d)\n",file_tpb);
    file.close();
    return -1;
  }


  file >> str;
  while(!file.eof()){

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
    else if(str == "beatspermeasure"){
      int N;
      file >> N;
      set_beats_per_measure(N);
    }
    else if(str == "measuresperphrase"){
      int N;
      file >> N;
      set_measures_per_phrase(N);
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
      file >> p->h;
      file >> p->s;
      file >> p->v;
      p->regen_colors();
      mevent* eend = p->events;
      int off_index;
      int type;
      mevent* e;
      int tick = 0;
      int delta;
      while(1){
        e = new mevent();

        file >> delta;
        if(delta == -1){delete e; break;}
        tick += delta;
        e->tick = tick;

        file >> e->type;
        file >> e->value1;
        file >> e->value2;
        //file >> e->dur;
        e->prev = eend;
        eend->next = e;
        eend = e;
      }
      p->fixdur();
      patlist.push_back(p);
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
        else if(key == "bank"){ file >> t->bank; }
        else if(key == "mute"){ file >> t->mute; }
        else if(key == "solo"){ file >> t->solo; }
        else if(key == "vol"){ file >> t->vol; }
        else if(key == "pan"){ file >> t->pan; }
        else if(key == "name"){
          file >> n;
          file.get();
          char* buf = (char*)malloc(n+1);
          file.read(buf,n);
          buf[n] = '\0';
          t->name = buf;
        }
        else if(key == "alive"){ file >> t->alive; }
        else if(key == "seqpat"){
          int pattern_number;
          seqpat* s = new seqpat();
          s->track = tracks.size();
          file >> s->tick;
          file >> s->dur;
          int total_layers;
          int index;
          pattern* p;

          file >> s->scrollx >> s->scrolly;
          file >> total_layers;

          if(total_layers == 1){
            file >> index;
            p = findpatternbyindex(index, patlist);
            s->p = p;
            s->layers = NULL;
          }
          else if(total_layers > 1){
            file >> index;
            p = findpatternbyindex(index, patlist);
            layerstack* layers = new layerstack(p);

            for(int j=1; j<total_layers; j++){
              file >> index;
              p = findpatternbyindex(index,patlist);
              layers->push_new(p);
            }

            file >> layers->index;
            s->p = layers->array[layers->index];
            s->layers = layers;
            layers->ref_c = 1;
          }
          else{
            printf("load: bad number of layers\n");
            file.close();
            return -1;
          }

          s->prev = send;
          send->next = s;
          send = s;
        }
        file >> key;
      }
      tracks.push_back(t);
      ui->track_info->add_track();
    }
    else{
      file.ignore(std::numeric_limits<streamsize>::max(),'\n');
    }

    file >> str;
  }

  set_rec_track(0);
  ui->track_info->set_rec(0);
  ui->track_info->update();

  ui->arranger->reset_handle();
  ui->arranger->redraw();

  reset_backend(0);

  file.close();

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

  if(filename == NULL){
    return -1;
  }

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

  file.close();

}



int getdelta(std::fstream& f){
  unsigned char a,b,c,d;

  f.read((char*)&a,1);
  if(a<0x80){return a;}

  f.read((char*)&b,1);
  if(b<0x80){return ((a&0x7f)<<7) | b;}

  f.read((char*)&c,1);
  if(c<0x80){return ((a&0x7f)<<14) | ((b&0x7f)<<7) | c;}

  f.read((char*)&d,1);
  return ((a&0x7f)<<21) | ((b&0x7f)<<14) | ((c&0x7f)<<7) | d;
}






char fifths[15][8] = {
"Cb",
"Gb",
"Db",
"Ab",
"Eb",
"Bb",
"F",
"C",
"G",
"D",
"A",
"E",
"B",
"F#",
"C#"
};


int loadsmf(const char* filename){
  if(filename == NULL){
    return -1;
  }

  fstream file;
  file.open(filename, fstream::in);

  if(!file.is_open()){
    printf("error, cant open file for saving\n");
    return -1;
  }

  last_filename = filename;
  set_last_dir(filename);

  clear();

  unsigned char buf[64];
  char* abuf;
  char sbuf[256];
  char* tbuf;
  uint32_t size;
  uint32_t micros;
  int N = 0;
  int tempo_flag = 0;

  std::list<pattern*> patlist;
  std::vector<int> chanlist;
  std::vector<int> proglist;
  std::vector<int> banklist;

  std::vector<track*> extratracks;

  std::vector<char*> tracknames;

  int maxblockdur = 0;

  for(int i=0; i<16; i++){
    extratracks.push_back(NULL);
  }

  while(!file.eof()){

    file.read((char*)buf,4);
    if(memcmp(buf,"MThd",4)){
      printf("missing header, probably not a midi file\n");
      file.close();
      return -1;
    }
    //printf("MThd\n");
    scope_print("Standard Midi File\n");

    file.read((char*)buf,4);
    if(buf[3] != 6){
      printf("header has wrong size, probably a broken midi file\n");
      scope_print("error: bad header size");
      file.close();
      return -1;
    }
    //printf("header size\n");

    file.read((char*)buf,2);
    if(buf[0] != 0){
      printf("bad smf type code, probably a broken midi file\n");
      scope_print("error: bad smf type");
      file.close();
      return -1;
    }


    int smftype;
    switch(buf[1]){
      case 0: smftype=0; break;
      case 1: smftype=1; break;
      case 2: smftype=2; break;
      default: 
        printf("bad smf type code, probably a broken midi file\n");
        scope_print("error: bad smf type");
        file.close();
        return -1;
    }

    snprintf(sbuf,256," type: %d\n",smftype);
    scope_print(sbuf);

    file.read((char*)buf,2);
    size = ntohs(*(unsigned short*)buf);
    if(size==0){
      printf("track count of zero, probably a broken midi file\n");
      scope_print("error: zero tracks");
      file.close();
      return -1;
    }
    int ntracks = size;

    snprintf(sbuf,256," tracks: %d\n",ntracks);
    scope_print(sbuf);


    int tpb = TICKS_PER_BEAT;
    file.read((char*)buf,2);
    size = ntohs(*(unsigned short*)buf);
    if(size >> 15 == 0){
      tpb = size&0x7fff;
      snprintf(sbuf,256," time division: %d ticks per beat\n",tpb);
      scope_print(sbuf);
    }
    else{
      int fps = size&0x7fff;
      snprintf(sbuf,256," time division: %d frames per second (wrong)\n",fps);
      scope_print(sbuf);
      scope_print("error: smpte time division not support\n");
      file.close();
      return -1;
    }


    int trackindex = 0;
    /*** read tracks ***/
    file.read((char*)buf,4);
    while(!file.eof()){

      if(memcmp(buf,"MTrk",4)){
        printf("bad track header, probably a broken midi file\n");
        file.close();
        return -1;
      }

      pattern* p = new pattern();
      mevent* e;

      trackindex++;
      snprintf(sbuf,256," track %d\n",trackindex);
      scope_print(sbuf);

      file.read((char*)buf,4);
      size = ntohl(*(unsigned long*)buf);
      if(size==0){
        printf("empty track\n");
        file.close();
        return -1;
      }

      int time = 0;
      int tick = 0;
      int endtrack=0;

      chanlist.push_back(-1);
      banklist.push_back(-1);
      proglist.push_back(-1);

      tracknames.push_back(NULL);

      /***read events***/
      while(!endtrack){

        int delta = getdelta(file);
        if(delta < 0){
          printf("bad delta time, broken midi file\n");
          file.close();
          return -1;
        }
        time += delta;
        tick = time*TICKS_PER_BEAT/tpb;

        if(tick > maxblockdur){
          maxblockdur = tick;
        }

        int last_byte0;
        file.read((char*)buf,1);
        int byte0 = buf[0];
        int byte1 = -1;

        if(byte0 < 0x80){//implicit byte0
          byte1 = byte0;
          byte0 = last_byte0;
        }
        last_byte0 = byte0;

        if(byte0 < 0xf0){//channel event

          int type = byte0&0xf0;
          int chan = byte0&0x0f;

          if(chanlist[N]==-1){
            chanlist[N]=chan;
          }

          if(byte1<0){//didnt read byte1 yet
            file.read((char*)buf,1);
            byte1 = buf[0];
          }



          int val1 = byte1;
          int val2;

          if(!(type == 0xC0 || type == 0xD0)){
              file.read((char*)buf,1);
              val2 = buf[0];
          }

          e = new mevent(type,tick,val1);


          switch(type){
            case 0x90://note on
              if(val2==0){//fake note off
                e->type = 0x80;
              }
              e->value2 = val2;
              break;
            case 0x80://note off
            case 0xA0://aftertouch
            case 0xB0://controller change
            case 0xE0://pitchbend
              if(type==0xB0 && val1==0x00 && banklist[N]==-1){
                banklist[N]=val2;
              }
              e->value2 = val2;
              break;

            case 0xC0://program change
              if(proglist[N]==-1){
                proglist[N]=val1;
              }
              break;
            case 0xD0://channel pressure
              break;
            default:
              printf("unrecognized channel event %d\n",type);
              file.close();
              return -1;
          }

          if(chan!=chanlist[N]){//put event in the a misfit track
            //printf("mistfit N=%d chan=%d type=%d\n",chanlist[N],chan,type);
            if(extratracks[chan] == NULL){
              track* t = new track();
              t->chan = chan;
              t->prog = -1;
              t->port = 0;
              t->bank = -1;

              extratracks[chan] = t;
              seqpat* s = new seqpat();
              t->head->next = s;
              s->prev = t->head;
              s->p = new pattern();
              //more track setup
            }
            extratracks[chan]->head->next->p->insert(e,tick);
          }
          else{//put it in a normal track
            p->append(e);
          }

        }
        else{/*** not a channel event ***/



          if(byte0 == 255){//meta events

            file.read((char*)buf,1);
            int meta = buf[0];


            size = getdelta(file);
            if(size < 0){
              printf("bad delta time\n");
              file.close();
              return -1;
            }


            abuf = new char[size+1];

            switch(meta){
              case 0://sequence number

                snprintf(sbuf,256,"  %d sequence: ? ?\n",time);
                scope_print(sbuf);
                if(size != 2){
                  printf("bad sequence number data length: %d\n",size);
                  file.close();
                  return -1;
                }
                file.read((char*)buf,2);
                break;
              case 1://text event
                file.read(abuf,size);
                abuf[size]='\0';
                asprintf(&tbuf,"  %d text: \"%s\"\n",time,abuf);
                scope_print(tbuf);
                free(tbuf);

                ui->info_text->buffer()->append(abuf);
                ui->info_text->buffer()->append("\n");

                break;
              case 2://copyright notice
                file.read((char*)abuf,size);
                abuf[size]='\0';
                asprintf(&tbuf,"  %d copyright: \"%s\"\n",time,abuf);
                scope_print(tbuf);
                free(tbuf);
                break;
              case 3://track name
                file.read((char*)abuf,size);
                abuf[size]='\0';
                asprintf(&tbuf,"  %d track name: \"%s\"\n",time,abuf);
                scope_print(tbuf);
                free(tbuf);

                tracknames[N] = (char*)malloc(sizeof(char)*(size+1));
                strncpy(tracknames[N],abuf,size+1);

                break;
              case 4://instrument name
                file.read((char*)abuf,size);
                abuf[size]='\0';
                asprintf(&tbuf,"  %d instrument name: \"%s\"\n",time,abuf);
                scope_print(tbuf);
                free(tbuf);
                break;
              case 5://lyrics
                file.read((char*)abuf,size);
                abuf[size]='\0';
                asprintf(&tbuf,"  %d lyrics: \"%s\"\n",time,abuf);
                scope_print(tbuf);
                free(tbuf);
                break;
              case 6://marker
                file.read((char*)abuf,size);
                abuf[size]='\0';
                asprintf(&tbuf,"  %d marker: \"%s\"\n",time,abuf);
                scope_print(tbuf);
                free(tbuf);
                break;
              case 7://cue point
                file.read((char*)abuf,size);
                abuf[size]='\0';
                asprintf(&tbuf,"  %d cue point: \"%s\"\n",time,abuf);
                scope_print(tbuf);
                free(tbuf);
                break;
              case 32://channel prefix
                if(size!=1){
                  printf("bad channel prefix data size: %d\n",size);
                  file.close();
                  return -1;
                }
                file.read((char*)buf,1);
                asprintf(&tbuf,"  %d channel prefix: %d\n",time,buf[0]);
                scope_print(tbuf);
                free(tbuf);
                break;
              case 47://end of track
                if(size!=0){
                  printf("end of track has non zero data size: %d\n",size);
                  file.close();
                  return -1;
                }
                endtrack=1;
                break;
              case 81://set tempo
                if(size!=3){
                  printf("set tempo has non-3 data size: %d\n",size);
                  file.close();
                  return -1;
                }
                file.read((char*)buf,3);

                buf[3] = buf[2];
                buf[2] = buf[1];
                buf[1] = buf[0];
                buf[0] = 0;

                micros = *(unsigned*)buf;
                micros = ntohl(micros);

                asprintf(&tbuf,"  %d set tempo: %d us/quarter\n",time,micros);
                scope_print(tbuf);
                free(tbuf);

                if(time==0){
                  tempo_flag = 1;
                  set_beats_per_minute(60000000/micros);
                }

                break;
              case 84://smpte offset
                if(size!=5){
                  printf("smpte offset has non-5 data size: %d\n",size);
                  file.close();
                  return -1;
                }
                file.read((char*)buf,5);
                asprintf(&tbuf,"  %d smpte offset: ?\n",time);
                scope_print(tbuf);
                free(tbuf);
                break;
              case 88://time signature
                if(size!=4){
                  printf("time signature has non-4 data size: %d\n",size);
                  file.close();
                  return -1;
                }
                file.read((char*)buf,4);
                asprintf(&tbuf,"  %d time signature: %d/%d\n",time,buf[0],1<<buf[1]);
                scope_print(tbuf);
                free(tbuf);
                break;
              case 89://key signature
                if(size!=2){
                  printf("key signature has non-2 data size: %d\n",size);
                  file.close();
                  return -1;
                }
                file.read((char*)buf,2);
                asprintf(&tbuf,"  %d key signature: %s %s\n",time,fifths[(signed char)buf[0]+7],buf[1]?"minor":"major");
                scope_print(tbuf);
                free(tbuf);
                break;
              case 127://sequencer specific
                file.read((char*)abuf,size);
                abuf[size]='\0';
                asprintf(&tbuf,"  %d sequencer specific: \"%s\"\n",time,abuf);
                scope_print(tbuf);
                free(tbuf);
                break;
              default://unknown meta event
                file.read((char*)abuf,size);
                abuf[size]='\0';
                asprintf(&tbuf,"  %d meta %d: \"%s\"\n",time,meta,abuf);
                scope_print(tbuf);
                free(tbuf);
            }
            free(abuf);
          }
          else if(byte0 == 240){//sysex event
            size = getdelta(file);
            if(size < 0){
              printf("bad delta time\n");
              return -1;
            }
            abuf = new char[size+1];
            file.read((char*)abuf,size);
            abuf[size]='\0';

            asprintf(&tbuf,"  %d sysex: \"%s\"\n",time,abuf);
            scope_print(tbuf);
            free(tbuf);

            file.read((char*)buf,1);
            if(buf[0]!=0xf7){
              file.putback(buf[0]);
            }
            else{
              scope_print("   end of sysex\n");
            }



            free(abuf);
          }
          else if(byte0 == 247){//divided sysex event
            size = getdelta(file);
            if(size < 0){
              printf("bad delta time\n");
              return -1;
            }
            abuf = new char[size+1];
            file.read((char*)abuf,size);
            abuf[size]='\0';

            asprintf(&tbuf,"  %d sysex fragment: \"%s\"\n",time,abuf);
            scope_print(tbuf);
            free(tbuf);

            file.read((char*)buf,1);
            if(buf[0]!=0xf7){
              file.putback(buf[0]);
            }
            else{
              scope_print("   end of sysex\n");
            }
            free(abuf);
          }
          else{
            printf("bad event type %d, broken midi file\n",byte0);
            file.close();
            return -1;
          }

        }

      }

      if(proglist[N]==-1){
        proglist[N]=0;
      }
      if(banklist[N]==-1){
        banklist[N]=0;
      }
      if(chanlist[N]==-1){
        chanlist[N]=0;
      }
      N++;


      p->fixdur();

      patlist.push_back(p);

      file.read((char*)buf,4);//read first byte of next track or EOF
    }

  }

  scope_print("End Of File\n\n");
  file.close();

  //TODO set up track settings using data remembered from the reading
  std::list<pattern*>::iterator p = patlist.begin();
  int i=0;
  while(p!=patlist.end()){
    track* t = new track();
    seqpat* s = new seqpat(i,0,128*64,*p);
    int Q = TICKS_PER_BEAT;
    s->dur = maxblockdur / Q * Q + Q;
    t->head->next = s;
    s->prev = t->head;
    t->skip = s;

    s->p->h = 360*i / 16;
    s->p->v = 0.8;
    while(s->p->h > 360){s->p->h -= 360;}
    s->p->regen_colors();

    t->chan = chanlist[i];
    t->prog = proglist[i];
    t->bank = banklist[i];
    t->port = 0;

    if(tracknames[i]){
      t->name = tracknames[i];
    }
    else{
      t->name = (char*)malloc(8);
      t->name[0] = '\0';
    }

    add_track(t);
    p++;
    i++;
  }

  if(tempo_flag == 0){
    set_beats_per_minute(120);
  }

  set_rec_track(0);
  ui->track_info->set_rec(0);
  ui->track_info->update();

  ui->arranger->reset_handle();
  ui->arranger->redraw();

  update_config_gui();

  reset_backend(0);

  return 0;
}

