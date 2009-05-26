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
#include <fstream>
#include <string.h>
#include <math.h>

#include <limits>

#include <fltk/run.h>

#include "seq.h"
#include "ui.h"
#include "backend.h"

#include "uihelper.h"


#define CONFIG_FILENAME ".epichordrc"

extern UI* ui;
extern std::vector<track*> tracks;

struct conf config;

using namespace std;

char* config_filename;



void load_config(){

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
  config.recordmode = 0;
  config.robmode = 0;
  config.defaultvelocity = 96;
  config.trackinit = 1;

  //linux dependent
  char* homepath = getenv("HOME");
  asprintf(&config_filename,"%s/"CONFIG_FILENAME,homepath);

  fstream f;
  f.open(config_filename,fstream::in);
  if(!f.is_open()){
    printf("load_config: Unable to open config file for reading.\n");

    load_default_keymap();
    update_config_gui();
    return;
  }

  config.beats_per_measure = 4;
  config.measures_per_phrase = 4;

  std::string word;

  while(!f.eof()){
    word = "";
    f >> word;
    if(word == "leadin"){f>>config.measures_until_record;}
    else if(word == "alwayscopy"){f>>config.alwayscopy;}
    else if(word == "autotrackname"){f>>config.autotrackname;}
    else if(word == "passthru"){f>>config.passthru;}
    else if(word == "playinsert"){f>>config.playinsert;}
    else if(word == "recordonchan"){f>>config.recordonchan;}
    else if(word == "playmove"){f>>config.playmove;}
    else if(word == "follow"){f>>config.follow;}
    else if(word == "recordmode"){f>>config.recordmode;}
    else if(word == "robmode"){f>>config.robmode;}
    else if(word == "keymap"){load_keymap(f);}
    else if(word == "defaultvelocity"){f>>config.defaultvelocity;}
    else if(word == "trackinit"){f>>config.trackinit;}
    else{
      f.ignore(std::numeric_limits<streamsize>::max(),'\n');
    }
  }

  backend_set_trackinit(config.trackinit);

  update_config_gui();
  f.close();
}

void save_config(){
  fstream f;
  f.open(config_filename,fstream::out);
  if(!f.is_open()){
    printf("save_config: Unable to open config file %s for saving.\n", config_filename);
    return;
  }

  f << "leadin " << config.measures_until_record << endl;
  f << "alwayscopy " << config.alwayscopy << endl;
  f << "autotrackname " << config.autotrackname << endl;
  f << "passthru " << config.passthru << endl;
  f << "playinsert " << config.playinsert << endl;
  f << "recordonchan " << config.recordonchan << endl;
  f << "playmove " << config.playmove << endl;
  f << "follow " << config.follow << endl;
  //f << "quantizedur " << config.quantizedur << endl;
  f << "recordmode " << config.recordmode << endl;
  f << "robmode " << config.robmode << endl;
  f << "defaultvelocity " << config.defaultvelocity << endl;
  f << "trackinit " << config.trackinit << endl;
  f << endl;
  save_keymap(f);
  f.close();
}

void update_config_gui(){
  ui->beats_per_measure->value(config.beats_per_measure);
  ui->measures_per_phrase->value(config.measures_per_phrase);
  ui->measures_until_record->value(config.measures_until_record);

  ui->bpm_wheel->value(config.beats_per_minute);
  ui->bpm_output->value(config.beats_per_minute);

  ui->check_alwayscopy->state(config.alwayscopy);
  ui->check_autotrackname->state(config.autotrackname);
  ui->check_passthru->state(config.passthru);
  ui->check_playinsert->state(config.playinsert);
  ui->check_recordonchan->state(config.recordonchan);
  ui->check_playmove->state(config.playmove);
  ui->check_follow->state(config.follow);

  ui->menu_recordmode->value(config.recordmode);
  ui->menu_rob->value(config.robmode);

  ui->default_velocity->value(config.defaultvelocity);

  ui->check_trackinit->value(config.trackinit);

  ui->config_window->redraw();
}



seqpat* rob_check(seqpat* s){
  seqpat* prev = s->prev;
  Command* c;
  if(config.robmode == 0){
    return NULL;
  }
  else if(config.robmode == 1 || prev == NULL){
    int pos = get_play_position();
    int M = config.measures_per_phrase;
    if(M!=0){
      M = M*config.beats_per_measure*TICKS_PER_BEAT;
    }
    else{
      M = 4*config.beats_per_measure*TICKS_PER_BEAT;
    }
    int P1 = pos/M*M;
    int P2 = P1 + M;
    int T = P1;
    int R = s->tick+s->dur;
    if(R > P1){
      T = R;
    }
    int W = P2 - T;
    if(s->next){
      int L = s->next->tick;
      if(L < P2){
        W = L - T;
      }
    }
    c = new CreateSeqpatBlank(s->track,T,W);
    set_undo(c);
    undo_push(1);
    return s->next;
  }
  else if(config.robmode == 2){
    int pos = get_play_position();
    int M = config.measures_per_phrase;
    if(M!=0){
      M = M*config.beats_per_measure*TICKS_PER_BEAT;
    }
    else{
      M = 4*config.beats_per_measure*TICKS_PER_BEAT;
    }
    int P = pos/M*M + M;//tick at next phrase boundary
    int W = P - s->tick;
    if(s->next){
      int W2 = s->next->tick - s->tick;
      if(W2 < W){
        W=W2;
      }
    }
    c = new ResizeSeqpat(s,W);
    set_undo(c);
    undo_push(1);
    return prev->next;
  }
}



int last_pos=0;
void playing_timeout_cb(void* v){
  int pos = get_play_position();

  if(pos < last_pos){
    reset_record_flags();
  }
  last_pos = pos;

  if(config.follow){
    ui->arranger->update(pos);
    ui->piano_roll->update(pos);
  }
  ui->song_timeline->update(pos);
  ui->pattern_timeline->update(pos);
  ui->metronome->update(pos);

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
            s = rob_check(s);
            if(!s){continue;}
          }

          //if(s->record_flag==1 && config.recordmode>0){show_song_edit();}
          s->record_check(config.recordmode);
          p = s->p;
          c=new CreateNoteOff(p,val1,val2,tick-s->tick);
          set_undo(c);
          undo_push(1);
          if(ui->piano_roll->visible()){
            ui->piano_roll->redraw();
            ui->event_edit->redraw();
            if(ui->event_edit->cur_seqpat == s){ui->event_edit->has[1]=1;}
            ui->event_menu->redraw();
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
            s = rob_check(s);
            if(!s){continue;}
          }

         // if(s->record_flag==1 && config.recordmode>0){show_song_edit();}
          s->record_check(config.recordmode);
          p = s->p;
          c=new CreateNoteOn(p,val1,val2,tick-s->tick,16);
          set_undo(c);
          undo_push(1);
          if(ui->piano_roll->visible()){
            ui->piano_roll->redraw();
            ui->event_edit->redraw();
            if(ui->event_edit->cur_seqpat == s){ui->event_edit->has[0]=1;}
            ui->event_menu->redraw();
          }
          if(ui->arranger->visible())
            ui->arranger->redraw();
          break;
        case 0xa0://aftertouch
        case 0xb0://controller
        case 0xc0://program change
        case 0xd0://channel pressure
        case 0xe0://pitch wheel

          s = tfind<seqpat>(t->head,tick);
          if(s->tick+s->dur < tick){
            s = rob_check(s);
            if(!s){continue;}
          }

          switch(type){
            case 0xa0:
              snprintf(report,256,"%02x %02x %02x : aftertouch - ch %d note %d %d\n",type|chan,val1,val2,chan,val1,val2);
              if(ui->event_edit->cur_seqpat == s){ui->event_edit->has[2]=1;}
              break;
            case 0xb0:
              snprintf(report,256,"%02x %02x %02x : controller change - ch %d cntr %d val %d\n",type|chan,val1,val2,chan,val1,val2);
              if(ui->event_edit->cur_seqpat == s){
                ui->event_edit->has[val1+6]=1;
              }
              break;
            case 0xc0:
              snprintf(report,256,"%02x %02x    : program change - ch %d pgrm %d \n",type|chan,val1,chan,val1);
              if(ui->event_edit->cur_seqpat == s){ui->event_edit->has[3]=1;}
              break;
            case 0xd0:
              snprintf(report,256,"%02x %02x    : channel pressure - ch %d val %d \n",type|chan,val1,chan,val1);
              if(ui->event_edit->cur_seqpat == s){ui->event_edit->has[4]=1;}
              break;
            case 0xe0:
              snprintf(report,256,"%02x %02x %02x : pitch wheel - ch %d val %d \n",type|chan,val1,val2,chan,(val2<<7)|val1);
              if(ui->event_edit->cur_seqpat == s){ui->event_edit->has[5]=1;}
              break;
          }
          scope_print(report);

          if(!is_backend_recording())
            break;

        //  if(s->record_flag==1 && config.recordmode>0){show_song_edit();}
          s->record_check(config.recordmode);
          p = s->p;
          c=new CreateEvent(p,type,tick,val1,val2);
          set_undo(c);
          undo_push(1);
          if(ui->piano_roll->visible()){
            ui->piano_roll->redraw();
            ui->event_edit->redraw();
            ui->event_menu->redraw();
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
            scope_print(getsysexbuf());
            scope_print("\nf7       : end of sysex\n");
          }
          else{
            scope_print(report);
          }
      }

  }


  //handle session events (LASH)
  int ret;
  char* session_string;
  char* filename_string;
  ret=backend_session_process();
  while(ret != SESSION_NOMORE){
    session_string=get_session_string();
    filename_string = (char*)malloc(strlen(session_string)+16);
    strcpy(filename_string,session_string);
    strcat(filename_string,"/song.epi");
    switch(ret){
      case SESSION_SAVE: save(filename_string); break;
      case SESSION_LOAD: load(filename_string); break;
      case SESSION_QUIT: shutdown_gui(); break;
      case SESSION_UNHANDLED: break;
    }
    free(session_string);
    ret=backend_session_process();
  }


  //maybe do dynamic update of controller widgets
  ui->track_info->dynamic_update();


  if(is_backend_playing()){
    fltk::repeat_timeout(0.005, playing_timeout_cb, NULL);
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

  int left = get_loop_start();
  if(get_play_position()==left || get_play_position()==0){
    left=0;
  }

  pause_backend();

  all_notes_off();

  reset_backend(left);




  ui->song_timeline->update(left);
  ui->pattern_timeline->update(left);

  ui->song_timeline->redraw();
  ui->pattern_timeline->redraw();

  ui->play_button->label("@>");
  ui->play_button->redraw();

  ui->metronome->update(left);

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
      ui->piano_roll->set_qtick(TICKS_PER_BEAT);
      break;
    case 8:
      ui->qbutton4->state(0);
      ui->qbutton8->state(1);
      ui->qbutton16->state(0);
      ui->qbutton32->state(0);
      ui->qbutton64->state(0);
      ui->qbutton128->state(0);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(TICKS_PER_BEAT/2);
      break;
    case 16:
      ui->qbutton4->state(0);
      ui->qbutton8->state(0);
      ui->qbutton16->state(1);
      ui->qbutton32->state(0);
      ui->qbutton64->state(0);
      ui->qbutton128->state(0);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(TICKS_PER_BEAT/4);
      break;
    case 32:
      ui->qbutton4->state(0);
      ui->qbutton8->state(0);
      ui->qbutton16->state(0);
      ui->qbutton32->state(1);
      ui->qbutton64->state(0);
      ui->qbutton128->state(0);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(TICKS_PER_BEAT/8);
      break;
    case 64:
      ui->qbutton4->state(0);
      ui->qbutton8->state(0);
      ui->qbutton16->state(0);
      ui->qbutton32->state(0);
      ui->qbutton64->state(1);
      ui->qbutton128->state(0);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(TICKS_PER_BEAT/16);
      break;
    case 128:
      ui->qbutton4->state(0);
      ui->qbutton8->state(0);
      ui->qbutton16->state(0);
      ui->qbutton32->state(0);
      ui->qbutton64->state(0);
      ui->qbutton128->state(1);
      ui->qbutton0->state(0);
      ui->piano_roll->set_qtick(TICKS_PER_BEAT/32);
      break;
  }
}

void set_songtool(int i){
  switch(i){
    case 0:
      ui->edit_button->state(1);
      ui->color_button->state(0);
      ui->unclone_button->state(0);
      ui->split_button->state(0);
      ui->join_button->state(0);
      ui->arranger->color_flag = 0;
      ui->arranger->unclone_flag = 0;
      ui->arranger->split_flag = 0;
      ui->arranger->join_flag = 0;
      break;
    case 1:
      ui->edit_button->state(0);
      ui->color_button->state(1);
      ui->unclone_button->state(0);
      ui->split_button->state(0);
      ui->join_button->state(0);
      ui->arranger->color_flag = 1;
      ui->arranger->unclone_flag = 0;
      ui->arranger->split_flag = 0;
      ui->arranger->join_flag = 0;
      break;
    case 2:
      ui->edit_button->state(0);
      ui->color_button->state(0);
      ui->unclone_button->state(1);
      ui->split_button->state(0);
      ui->join_button->state(0);
      ui->arranger->color_flag = 0;
      ui->arranger->unclone_flag = 1;
      ui->arranger->split_flag = 0;
      ui->arranger->join_flag = 0;
      break;
    case 3:
      ui->edit_button->state(0);
      ui->color_button->state(0);
      ui->unclone_button->state(0);
      ui->split_button->state(1);
      ui->join_button->state(0);
      ui->arranger->color_flag = 0;
      ui->arranger->unclone_flag = 0;
      ui->arranger->split_flag = 1;
      ui->arranger->join_flag = 0;
      break;
    case 4:
      ui->edit_button->state(0);
      ui->color_button->state(0);
      ui->unclone_button->state(0);
      ui->split_button->state(0);
      ui->join_button->state(1);
      ui->arranger->color_flag = 0;
      ui->arranger->unclone_flag = 0;
      ui->arranger->split_flag = 0;
      ui->arranger->join_flag = 1;
      break;
  }
}


void set_trip(int v){
  ui->piano_roll->set_trip(v);
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

void set_beats_per_minute(int n){
  config.beats_per_minute = n;
  set_bpm(n);
}

void set_measures_until_record(int n){
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

void set_recordmode(int n){
  config.recordmode = n;
}

void set_robmode(int n){
  config.robmode = n;
}

void set_defaultvelocity(int n){
  config.defaultvelocity = n;
}

void set_trackinit(int n){
  config.trackinit = n;
  backend_set_trackinit(n);
}


int scopeon=0;
void turnonscope(){
  scopeon=1;
}

void turnoffscope(){
  scopeon=0;
  fltk::TextBuffer* ptr = ui->scope->buffer();
  ptr->remove(0,ptr->length());
 // ui->redraw();
}

void scope_print(const char* text){
  if(scopeon){
    ui->scope->append(text);
    int N = ui->scope->buffer()->length();
    ui->scope->scroll(N,0);
  }
}



void show_song_edit(){
  ui->pattern_edit->hide();
  ui->pattern_buttons->hide();
  ui->song_edit->activate();
  ui->song_edit->show();
  ui->song_edit->take_focus();
  ui->song_buttons->show();
}

void show_pattern_edit(){
  ui->song_edit->hide();
  ui->song_edit->deactivate();
  ui->song_buttons->hide();
  ui->pattern_edit->take_focus();
  ui->pattern_edit->show();
  ui->pattern_buttons->show();
}


static int tool = 0;
//switch between normal, note off, portamento, and aftertouch
void toggle_tool(){
  switch(tool){
    case 0:
      tool=1;
      ui->tool_button->copy_label("80");
      ui->tool_button->state(1);
      break;
    case 1:
      tool=2;
      ui->tool_button->copy_label("A0");
      break;
    case 2:
      tool=3;
      ui->tool_button->copy_label("po");
      break;
    case 3:
      tool=0;
      ui->tool_button->copy_label("tool");
      ui->tool_button->state(0);
      break;
  }
}



void reset_song(){
  clear();

  track* t;
  for(int i=0; i<16; i++){
    t = new track();
    t->head->track = i;
    t->chan = i;
    add_track(t);
  }

  set_rec_track(0);
  ui->track_info->set_rec(0);
  ui->track_info->update();
  ui->action_window->hide();
}



void add_track(track* t){
  tracks.push_back(t);
  ui->track_info->add_track();
}

void remove_track(int n){

}


void dump_pattern(){
  pattern* p = ui->piano_roll->cur_seqpat->p;
  mevent* e = p->events;

  printf("dump of pattern %p\n",p);
  printf("time type value1 value2\n");
int this_t = 0;
  while(e){
    int last_t = this_t;
    this_t = e->tick;
    if(this_t < last_t){printf("WARNING\n");}
    switch(e->type){
      case -1:
        printf("%d DUMMY %d %d\n",e->tick,e->value1,e->value2);
        break;
      case MIDI_NOTE_OFF:
        printf("%d NOTEOFF %d %d\n",e->tick,e->value1,e->value2);
        break;
      case MIDI_NOTE_ON:
        printf("%d NOTEON %d %d\n",e->tick,e->value1,e->value2);
        break;
      case MIDI_AFTERTOUCH:
        printf("%d AFTERTOUCH %d %d\n",e->tick,e->value1,e->value2);
        break;
      case MIDI_CONTROLLER_CHANGE:
        printf("%d CC %d %d\n",e->tick,e->value1,e->value2);
        break;
      case MIDI_PROGRAM_CHANGE:
        printf("%d PROGRAM %d %d\n",e->tick,e->value1,e->value2);
        break;
      case MIDI_CHANNEL_PRESSURE:
        printf("%d CHANNELPRESSURE %d %d\n",e->tick,e->value1,e->value2);
        break;
      case MIDI_PITCH_WHEEL:
        printf("%d PITCHWHEEL %d %d\n",e->tick,e->value1,e->value2);
        break;
    }
    e=e->next;
  }
  printf("\n");
}


void init_gui(){

  ui->arranger->layout();
  ui->song_vscroll->slider_size(60);
  ui->song_vscroll->value(0);

  ui->pattern_timeline->edit_flag = 1;
  ui->pattern_timeline->zoom = 15;
  ui->pattern_vscroll->minimum(12*75);
  ui->pattern_vscroll->maximum(0);
  ui->pattern_vscroll->value(300);
  ui->pattern_vscroll->slider_size(50);
  ui->pattern_hscroll->value(0);

}

void shutdown_gui(){
  ui->main_window->hide();
  ui->config_window->hide();
  ui->help_window->hide();
  ui->action_window->hide();
  ui->scope_window->hide();
}



