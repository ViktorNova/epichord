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

#include <unistd.h>
#include <stdio.h>

#include <fltk/Group.h>
#include <fltk/Widget.h>
#include <fltk/events.h>

#include "ui.h"

#include "backend.h"

#include "util.h"

extern UI* ui;

using namespace fltk;


Keyboard::Keyboard(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {
  sustain = 0;
  cur_note = -1;
  fltk::add_event_handler(keyboard_handler);
}

int keyboard_handler(int e, fltk::Window* w){
  switch(e){
    case fltk::SHORTCUT:
      if(fltk::event_key_repeated()){
        return 0;
      }
      switch(event_key()){
        case fltk::SpaceKey:
          ui->keyboard->set_sustain(1);
          break;
        case '[':
          //printf("octave down\n");
          break;
        case ']':
          //printf("octave up\n");
          break;
        case 'q':
          //printf("C\n");
          break;
        case 'w':
          //printf("D\n");
          break;
        case 'e':
          //printf("E\n");
          break;
        case 'r':
          //printf("F\n");
          break;
        case 't':
          //printf("G\n");
          break;
      }
      return 1;
    case fltk::KEYUP:
      switch(event_key()){
        case fltk::SpaceKey:
          ui->keyboard->set_sustain(0);
          break;
        case 'q':
          //printf("C off\n");
          break;
        case 'w':
          //printf("D off\n");
          break;
        case 'e':
          //printf("E off\n");
          break;
        case 'r':
          //printf("F off\n");
          break;
        case 't':
          //printf("G off\n");
          break;
      }
      return 1;
  }
  return 0;
}


void note_on(int note, int vel, int channel){
  
}

void Keyboard::set_sustain(int state){
  sustain=state;
  if(state==0){
    for(int i=0; i<128; i++){
      if(ons[i]==1 && cur_note!=i){
        ons[i]=0;
        midi_note_off(i,cur_chan,cur_port);
      }
    }
    redraw();
  }
}

int Keyboard::handle(int event){
  char buf[16];
  int note;
  switch(event){
    case fltk::FOCUS:
      return 1;
    case fltk::PUSH:
      take_focus();
      note = ypix2note(event_y()+scroll, event_x() < w()/2 ? 1 : 0);
      play_note(note);
      redraw();
      return 1;
    case fltk::RELEASE:
      cur_note=-1;
      if(sustain == 0){
        cut_notes();
        redraw();
      }
      return 1;
    case fltk::DRAG:
      note = ypix2note(event_y()+scroll, event_x() < w()/2 ? 1 : 0);
      if(cur_note != note){
        if(sustain == 0){
          cut_notes();
        }
        play_note(note);
        redraw();
      }
      return 1;
  }
  return 0;
}

void Keyboard::play_note(int note){
  if(ons[note]==1){
    return;
  }
  cur_note = note;
  char buf[3];
  buf[0] = 0x90 | cur_chan;
  buf[1] = note;
  buf[2] = 0x7f;
  send_midi(buf,3,cur_port);

  ons[note] = 1;
}

void Keyboard::cut_notes(){
  for(int i=0; i<128; i++){
    if(ons[i]){
      ons[i] = 0;
      midi_note_off(i,cur_chan,cur_port);
    }
  }
}


void Keyboard::draw(){
  fltk::setcolor(fltk::WHITE);
  fltk::fillrect(0,0,w(),h());

  fltk::push_clip(0,0,w(),h());

  //draw held white notes
  int black;
  for(int i=0; i<128; i++){
    if(ons[i]){
      int Y = note2ypix(i,&black)-scroll;
      if(!black){
        fltk::setcolor(fltk::GRAY50);
        fltk::fillrect(0,Y,w(),12);
      }
    }
  }

  fltk::setcolor(fltk::BLACK);
  for(int i=-scroll; i<h(); i+=12){
    fltk::drawline(0,i,w(),i);
  }

  int j;
  for(int i=0; i<11; i++){
    j = 900 - 12 - i*12*7 - scroll;
    fltk::fillrect(0,j-3,w()/2,7);
    fltk::fillrect(0,j-12-3,w()/2,7);

    fltk::fillrect(0,j-12*3-3,w()/2,7);
    if(i<10){
      fltk::fillrect(0,j-12*4-3,w()/2,7);
      fltk::fillrect(0,j-12*5-3,w()/2,7);
    }
  }

  //draw held black notes
  for(int i=0; i<128; i++){
    if(ons[i]){
      int Y = note2ypix(i,&black)-scroll;
      if(black){
        fltk::setcolor(fltk::GRAY70);
        fltk::fillrect(1,Y+1,w()/2 - 2,5);
      }
    }
  }

  fltk::pop_clip();
}

