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

#include <fltk/run.h>

#include "seq.h"
#include "ui.h"
#include "backend.h"


extern UI* ui;
extern std::vector<track*> tracks;



void playing_timeout_cb(void* v){
  if(!is_backend_playing()){
    return;
  }
  ui->song_timeline->update(get_play_position());
  ui->pattern_timeline->update(get_play_position());

  //check for midi input
  int tick;
  int chan;
  int type;
  int val1;
  int val2;
  
  while(recv_midi(&chan,&tick,&type,&val1,&val2)){
      printf("recording midi input\n");

      /*
note on - insert a note on event with dur 32
note off - insert a note off event, then resize previous note on
      */
      /*
      merge - insert events into selected track, current pattern

      overwrite - erase all notes in current pattern upon loop / new event

      layer - create new pattern and switch record head upon loop / new event

      */
  }
  fltk::repeat_timeout(0.01, playing_timeout_cb, NULL);
}

void press_play(){
  if(!is_backend_playing()){
    start_backend();
    ui->play_button->label("@||");
    fltk::add_timeout(0.01, playing_timeout_cb, NULL);
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
