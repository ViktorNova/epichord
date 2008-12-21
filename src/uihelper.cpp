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

#include <stdlib.h>
#include <vector>

#include <math.h>

#include <fltk/run.h>

#include "seq.h"
#include "ui.h"
#include "backend.h"

#include "uihelper.h"

extern UI* ui;
extern std::vector<track*> tracks;

struct conf config;


void config_init(){
  //todo: load a config file instead
  config.beats_per_measure = 4;
  config.measures_per_phrase = 4;
  config.measures_until_record = 1;
  config.alwayscopy = 0;
  config.autotrackname = 0;
  config.passthru = 1;
  config.playinsert = 1;
  config.recordonchan = 0;
  config.playmove = 1;
  config.follow = 1;
  config.quantizedur = 1;
  config.recordmode = 0;
  config.robmode = 0;
}




void playing_timeout_cb(void* v){
  //if(!is_backend_playing()){
  //  return;
  //}

  int pos = get_play_position();
  ui->song_timeline->update(pos);
  ui->pattern_timeline->update(pos);
  ui->metronome->update(pos);
  if(config.follow){
    ui->arranger->update(pos);
    ui->piano_roll->update(pos);
  }

  //check for midi input
  int tick;
  int chan;
  int type;
  int val1;
  int val2;

  track* t = tracks[get_rec_track()];
  Command* c;
  seqpat* s;
  pattern* p;

  char report[256];

  while(recv_midi(&chan,&tick,&type,&val1,&val2)){

      if(config.recordonchan){
        for(int i=0; i<tracks.size(); i++){
          if(tracks[i]->chan == chan){
            t = tracks[i];
          }
        }
      }

      switch(type){
        case 0x80://note off
          snprintf(report,256,"%02x %02x %02x : note off - ch %d note %d vel %d\n",type|chan,val1,val2,chan,val1,val2);
          scope_print(report);

          if(!is_backend_recording())
            break;

          s = tfind<seqpat>(t->head,tick);
          if(s->tick+s->dur < tick){
            //printf("rec head outside block\n");
            continue;
          }
          p = s->p;
          c=new CreateNoteOff(p,val1,val2,tick-s->tick);
          set_undo(c);
          undo_push(1);
          if(ui->piano_roll->visible()){
            ui->piano_roll->redraw();
            ui->event_edit->redraw();
          }
          if(ui->arranger->visible())
            ui->arranger->redraw();
          break;
        case 0x90://note on
          snprintf(report,256,"%02x %02x %02x : note on - ch %d note %d vel %d\n",type|chan,val1,val2,chan,val1,val2);
          scope_print(report);

          if(!is_backend_recording())
            break;

          s = tfind<seqpat>(t->head,tick);
          if(s->tick+s->dur < tick){
            //printf("rec head outside block\n");
            continue;
          }
          p = s->p;
          c=new CreateNoteOn(p,val1,val2,tick-s->tick,16);
          set_undo(c);
          undo_push(1);
          if(ui->piano_roll->visible()){
            ui->piano_roll->redraw();
            ui->event_edit->redraw();
          }
          if(ui->arranger->visible())
            ui->arranger->redraw();
          break;
        case 0xa0://aftertouch
        case 0xb0://controller
        case 0xc0://program change
        case 0xd0://channel pressure
        case 0xe0://pitch wheel

          switch(type){
            case 0xa0:
              snprintf(report,256,"%02x %02x %02x : aftertouch - ch %d note %d %d\n",type|chan,val1,val2,chan,val1,val2);
              break;
            case 0xb0:
              snprintf(report,256,"%02x %02x %02x : controller change - ch %d cntr %d val %d\n",type|chan,val1,val2,chan,val1,val2);
              break;
            case 0xc0:
              snprintf(report,256,"%02x %02x    : program change - ch %d pgrm %d \n",type|chan,val1,chan,val1);
              break;
            case 0xd0:
              snprintf(report,256,"%02x %02x    : channel pressure - ch %d val %d \n",type|chan,val1,chan,val1);
              break;
            case 0xe0:
              snprintf(report,256,"%02x %02x %02x : pitch wheel - ch %d val %d \n",type|chan,val1,val2,chan,(val2<<7)|val1);
              break;
          }
          scope_print(report);

          if(!is_backend_recording())
            break;

          s = tfind<seqpat>(t->head,tick);
          if(s->tick+s->dur < tick){
            //scope_print("record head outside block\n");
            continue;
          }
          p = s->p;
          c=new CreateEvent(p,type,tick,val1,val2);
          set_undo(c);
          undo_push(1);
          if(ui->piano_roll->visible()){
            ui->piano_roll->redraw();
            ui->event_edit->redraw();
          }
          if(ui->arranger->visible())
            ui->arranger->redraw();
          break;
        case 0xf0:
          switch(chan){
            case 1://undefined (reserved) system common message
              snprintf(report,256,"%02x       : undefined (reserved) system common message\n",type|chan);
              break;
            case 2://song position pointer
              snprintf(report,256,"%02x %02x %02x : song position - %d \n",type|chan,val1,val2,(val2<<7)|val1);
              break;
            case 3://song select
              snprintf(report,256,"%02x %02x    : song select - %d \n",type|chan,val1,val1);
              break;
            case 4://undefined (reserved) system common message
            case 5://undefined (reserved) system common message
              snprintf(report,256,"%02x       : undefined (reserved) system common message\n",type|chan);
              break;
            case 6://tune request
              snprintf(report,256,"%02x       : tune request\n",type|chan);
              break;
            case 7://end of exclusive
              snprintf(report,256,"%02x       : end of exclusive\n",type|chan);
              break;
            case 8://timing clock
              snprintf(report,256,"%02x       : timing clock\n",type|chan);
              break;
            case 9://undefined (reserved) system common message
              snprintf(report,256,"%02x       : undefined (reserved) system common message\n",type|chan);
              break;
            case 10://start
              snprintf(report,256,"%02x       : start\n",type|chan);
              break;
            case 11://continue
              snprintf(report,256,"%02x       : continue\n",type|chan);
              break;
            case 12://stop
              snprintf(report,256,"%02x       : stop\n",type|chan);
              break;
            case 13://undefined
              snprintf(report,256,"%02x       : undefined (reserved) system common message\n",type|chan);
              break;
            case 14://active sensing
              snprintf(report,256,"%02x       : active sensing\n",type|chan);
              break;
            case 15://reset
              snprintf(report,256,"%02x       : reset\n",type|chan);
              break;
          }
          if(chan==0){
            snprintf(report,256,"%02x %02x    : system exclusive - id %d ; data follows\n",type|chan,val1,val1);
            scope_print(report);
            scope_print((char*)getsysexbuf());
            scope_print("\nf7       : end of sysex\n");
          }
          else{
            scope_print(report);
          }
      }

  }

  if(is_backend_playing()){
    fltk::repeat_timeout(0.01, playing_timeout_cb, NULL);
  }
  else{
    fltk::repeat_timeout(0.1, playing_timeout_cb, NULL);
  }
}

void start_monitor(){
  fltk::add_timeout(0.1, playing_timeout_cb, NULL);
}

void press_play(){
  if(!is_backend_playing()){
    start_backend();
    ui->play_button->label("@||");
    //fltk::add_timeout(0.01, playing_timeout_cb, NULL);
  }
  else{
    pause_backend();
    all_notes_off();
    ui->play_button->label("@>");
  }
}

void press_stop(){

  //stops playback and sets the play position to zero
  pause_backend();
  reset_backend(0);
  all_notes_off();

  ui->song_timeline->update(0);
  ui->pattern_timeline->update(0);

  ui->song_timeline->redraw();
  ui->pattern_timeline->redraw();

  //send program change on all channels
  //char buf[3];
  //for(int i = 0; i<tracks.size(); i++){
  //  program_change(i, tracks[i]->program);
  //}

  ui->play_button->label("@>");
  ui->play_button->redraw();

  ui->metronome->update(0);

}


void set_quant(int q){
  switch(q){
    case 0:
      ui->qbutton4->state(0);
      ui->qbutton8->state(0);
      ui->qbutton16->state(0);
      ui->qbutton32->state(0);
      ui->qbutton64->state(0);
      ui->qbutton128->state(0);
      ui->qbutton0->state(1);
      ui->piano_roll->set_qtick(1);
      break;
    case 4:
      ui->qbutton4->state(1);
      ui->qbutton8->state(0);
      ui->qbutton16->state(0);
      ui->qbutton32->state(0);
      ui->qbutton64->state(0);
      ui->qbutton128->state(0);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(128);
      break;
    case 8:
      ui->qbutton4->state(0);
      ui->qbutton8->state(1);
      ui->qbutton16->state(0);
      ui->qbutton32->state(0);
      ui->qbutton64->state(0);
      ui->qbutton128->state(0);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(64);
      break;
    case 16:
      ui->qbutton4->state(0);
      ui->qbutton8->state(0);
      ui->qbutton16->state(1);
      ui->qbutton32->state(0);
      ui->qbutton64->state(0);
      ui->qbutton128->state(0);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(32);
      break;
    case 32:
      ui->qbutton4->state(0);
      ui->qbutton8->state(0);
      ui->qbutton16->state(0);
      ui->qbutton32->state(1);
      ui->qbutton64->state(0);
      ui->qbutton128->state(0);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(16);
      break;
    case 64:
      ui->qbutton4->state(0);
      ui->qbutton8->state(0);
      ui->qbutton16->state(0);
      ui->qbutton32->state(0);
      ui->qbutton64->state(1);
      ui->qbutton128->state(0);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(8);
      break;
    case 128:
      ui->qbutton4->state(0);
      ui->qbutton8->state(0);
      ui->qbutton16->state(0);
      ui->qbutton32->state(0);
      ui->qbutton64->state(0);
      ui->qbutton128->state(1);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(4);
      break;
  }
}


void set_beats_per_measure(int n){
  config.beats_per_measure = n;
  ui->metronome->set_N(n);
  ui->piano_roll->redraw();
  ui->arranger->redraw();
  ui->arranger->q_tick = n*TICKS_PER_BEAT;
  ui->song_timeline->redraw();
  ui->pattern_timeline->redraw();
}

void set_measures_per_phrase(int n){
  config.measures_per_phrase = n;
  ui->piano_roll->redraw();
  ui->arranger->redraw();
  ui->song_timeline->redraw();
  ui->pattern_timeline->redraw();
}

void set_mur(int n){
  config.measures_until_record = n;
}

void set_alwayscopy(int v){
  config.alwayscopy = v;
}

void set_autotrackname(int v){
  config.autotrackname = v;
}

void set_passthru(int v){
  config.passthru = v;
  backend_set_passthru(v);
}

void set_playinsert(int v){
  config.playinsert = v;
}

void set_recordonchan(int v){
  config.recordonchan = v;
}

void set_playmove(int v){
  config.playmove = v;
}

void set_follow(int v){
  config.follow = v;
}

void set_quantizedur(int v){
  config.quantizedur = v;
}

void set_recordmode(int n){
  config.recordmode = n;
}

void set_robmode(int n){
  config.robmode = n;
}


void scope_print(char* text){
  ui->scope->append(text);
  int N = ui->scope->buffer()->length();
  ui->scope->scroll(N,0);
}

