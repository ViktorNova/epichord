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

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <fltk/Group.h>
#include <fltk/Widget.h>
#include <fltk/events.h>

#include "ui.h"

#include "eventedit.h"

#include "uihelper.h"

extern struct conf config;

extern UI* ui;
extern std::vector<track*> tracks;

extern char controller_names[128][64];

using namespace fltk;

EventEdit::EventEdit(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {

  zoom = 30;

  event_type = MIDI_NOTE_ON;
  controller_type = 0;

}

int EventEdit::handle(int event){
  switch(event){
    case MOUSEWHEEL:
      if(event_dy() < 0){
        event_type_next();
      }
      else if(event_dy() > 0){
        event_type_prev();
      }
      redraw();
      return 1;
      break;
  }
  return 0;
}

void EventEdit::draw(){
  fltk::setcolor(fltk::GRAY05);
  fltk::fillrect(0,0,w(),h());

  fltk::push_clip(0,0,w(),h());

  fltk::setcolor(fltk::GRAY20);
  fltk::drawtext(event_type_name(), 2, h()-5);

  fltk::setcolor(fltk::GRAY20);
  fltk::fillrect(0,h()-3,w(),1);
  for(int i=zoom - scroll; i<w(); i+=zoom){
    fltk::drawline(i,0,i,h()-1);
  }

  fltk::setcolor(fltk::GRAY50);
  for(int i=zoom*4-scroll; i<w(); i+=zoom*4){
    fltk::drawline(i,0,i,h()-1);
  }

  fltk::setcolor(fltk::WHITE);
  int M = config.beats_per_measure;
  int I = 0;
  for(int i=1; I<w(); i++){
    I = i*zoom*4*M - scroll;
    fltk::fillrect(I,0,1,h());
  }

  fltk::setcolor(fltk::RED);
  int rightend = tick2xpix(cur_seqpat->dur)-scroll;
  fltk::fillrect(rightend,0,1,h());

  mevent* e = cur_seqpat->p->events->next;
  int v;
  while(e){
    if(e->type==event_type){
      if(e->type==MIDI_CONTROLLER_CHANGE){
        if(e->value1 == controller_type){
          v = e->value2;
        }
        else{
          e=e->next;
          continue;
        }
      }
      else{
        switch(e->type){
          case -1:
            e = e->next;
            continue;
          case MIDI_PROGRAM_CHANGE:
          case MIDI_CHANNEL_PRESSURE:
            v = e->value1;
            break;
          default:
            v = e->value2;
            break;
        }
      }
      int X = tick2xpix(e->tick) - scroll;
      int H = v*(h()-3)/127;
      fltk::setcolor(fltk::color(169,75,229));
      fltk::fillrect(X,h()-H+1-3,1,H-1+3);
      fltk::fillrect(X+1,h()-H-3,1,1);
      fltk::setcolor(fltk::color(95,58,119));
      fltk::fillrect(X+1,h()-H+1-3,1,H-1+3);
      fltk::setcolor(fltk::color(198,109,225));
      fltk::fillrect(X,h()-H-3,1,1);
    }
    e = e->next;
  }

  fltk::pop_clip();

}


void EventEdit::load(seqpat* s){
  cur_seqpat = s;
  cur_track = tracks[s->track];
}

int EventEdit::tick2xpix(int tick){
  return tick*zoom*4 / 128;
}

char* EventEdit::event_type_name(){
  switch(event_type){
    case MIDI_NOTE_OFF:
      return "note off velocity";
    case MIDI_NOTE_ON:
      return "note on velocity";
    case MIDI_AFTERTOUCH:
      return "polyphonic key pressure (aftertouch)";
    case MIDI_CONTROLLER_CHANGE:
      return controller_names[controller_type];
    case MIDI_PROGRAM_CHANGE:
      return "program change";
    case MIDI_CHANNEL_PRESSURE:
      return "channel pressure";
    case MIDI_PITCH_WHEEL:
      return "pitch wheel";
    default:
      return "booya";
  }
}

void EventEdit::event_type_next(){
  switch(event_type){
    case 0x80:
      event_type = 0xA0;
      break;
    case 0x90:
      event_type = 0x80;
      break;
    case 0xA0:
      event_type = 0xC0;
      break;
    case 0xB0:
      if(controller_type == 127){
        event_type = 0x90;
      }
      else{
        controller_type++;
      }
      break;
    case 0xE0:
      event_type = 0xB0;
      controller_type = 0;
      break;
    default:
      event_type += 0x10;
  }
}

void EventEdit::event_type_prev(){
  switch(event_type){
    case 0x80:
      event_type = 0x90;
      break;
    case 0x90:
      event_type = 0xB0;
      controller_type = 127;
      break;
    case 0xC0:
      event_type = 0xA0;
      break;
    case 0xB0:
      if(controller_type == 0){
        event_type = 0xE0;
      }
      else{
        controller_type--;
      }
      break;
    case 0xA0:
      event_type = 0x80;
      break;
    default:
      event_type -= 0x10;
  }
}

void EventEdit::set_event_type(int type, int controller){
  event_type = type;
  controller_type = controller;
}

