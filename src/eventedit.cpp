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


#define MAG_MAX 16383

extern struct conf config;

extern UI* ui;
extern std::vector<track*> tracks;

extern char controller_names[128][64];

using namespace fltk;

EventEdit::EventEdit(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {

  zoom = 30;

  event_type = MIDI_NOTE_ON;
  controller_type = 0;


  label_flag = 0;
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
    case PUSH:
      if(event_button()==1){
        //actually, check for
        //shift box select
        //control insert event
        line_flag=1;
        line_orig_x=event_x();
        line_orig_y=event_y();
        line_x = event_x();
        line_y = event_y();
        //more state
        redraw();
        return 1;
      }
      else if(event_button()==2){//paste
      }
      else if(event_button()==3){//delete
      }
      break;
    case DRAG:
      if(line_flag){
        line_x = event_x();
        line_y = event_y();



      }
      redraw();
      break;
    case RELEASE:
      line_flag=0;
      int t1 = ui->piano_roll->xpix2tick(line_orig_x+scroll);
      int t2 = ui->piano_roll->xpix2tick(line_x+scroll);
      int v1 = ypix2mag(line_orig_y);
      int v2 = ypix2mag(line_y);
      if(t1>t2){
        int tmp = t2;
        t2 = t1;
        t1 = tmp;
        tmp = v2;
        v2 = v1;
        v1 = tmp;
      }
      apply_line(t1,t2,v1,v2);
      redraw();
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

  while(e){
    if(e->type==event_type){
      if(e->type==MIDI_CONTROLLER_CHANGE){
        if(e->value1 == controller_type){
          M = val2mag(e->value2);
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
            M = val2mag(e->value1);
            break;
          default:
            M = val2mag(e->value2);
            break;
        }
      }
      int T1 = ui->piano_roll->xpix2tick(line_orig_x+scroll);
      int T2 = ui->piano_roll->xpix2tick(line_x+scroll);
      int M1 = ypix2mag(line_orig_y);
      int M2 = ypix2mag(line_y);
      if(T1>T2){
        int tmp = T2;
        T2 = T1;
        T1 = tmp;
        tmp = M2;
        M2 = M1;
        M1 = tmp;
      }
      if(line_flag && e->tick > T1 && e->tick < T2){
        float m = (float)(M2-M1)/(T2-T1);
        float b = M1 - T1*m;
        M = (int)(m*e->tick + b);
        if(M<0){M=0;}
        if(M>MAG_MAX){M=MAG_MAX;}
      }
      int X = tick2xpix(e->tick) - scroll;
      int Y = mag2ypix(M);
      int H = h()-Y;
      fltk::setcolor(fltk::color(169,75,229));
      fltk::fillrect(X,Y+1,1,H);
      fltk::fillrect(X+1,Y,1,1);
      fltk::setcolor(fltk::color(95,58,119));
      fltk::fillrect(X+1,Y+1,1,H);
      fltk::setcolor(fltk::color(198,109,225));
      fltk::fillrect(X,Y,1,1);
      if(label_flag){
        fltk::setcolor(fltk::color(169,75,229));
        char buf[16];
        if(e->type == MIDI_PITCH_WHEEL){
          snprintf(buf,16,"%d",M);
        }
        else{
          snprintf(buf,16,"%d",mag2val(M));
        }
        fltk::drawtext(buf,X+2,Y+12<h()-3?Y+12:h()-3);
      }
    }
    e = e->next;
  }

  if(line_flag){
    fltk::setcolor(fltk::BLUE);
    fltk::drawline(line_orig_x,line_orig_y,line_x,line_y);
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


int EventEdit::ypix2mag(int ypix){
  int H = h()-3;
  int R = ypix*MAG_MAX/H;
  //if(R>MAG_MAX){return 0;}
  //if(R<0){return MAG_MAX;}
  return MAG_MAX-R;
}

int EventEdit::mag2ypix(int mag){
  int H = mag*(h()-3)/MAG_MAX;
  return h()-H-3;
}

int EventEdit::mag2val(int mag){
  return mag*127/MAG_MAX;
}

int EventEdit::val2mag(int val){
  return val*MAG_MAX/127;
}

void EventEdit::apply_line(int t1, int t2, int M1, int M2){
  mevent* e = cur_seqpat->p->events;
  Command* c;
  int N = 0;
  while(e->tick < t1){
    e = e->next;
    if(!e){
      return;
    }
  }
  while(e){
    if(e->tick > t2){
      break;
    }
    if(match_event_type(e)){
      float m = (float)(M2-M1)/(t2-t1);
      float b = M1 - m*t1;
      int M = (int)(m*e->tick + b);
      int V1, V2;
      if(M<0){M=0;}
      if(M>MAG_MAX){M=MAG_MAX;}
      switch(e->type){
        case MIDI_NOTE_OFF:
        case MIDI_NOTE_ON:
        case MIDI_AFTERTOUCH:
        case MIDI_CONTROLLER_CHANGE:
          V1 = e->value1;
          V2 = mag2val(M);
          break;
        case MIDI_PROGRAM_CHANGE:
        case MIDI_CHANNEL_PRESSURE:
          V1 = mag2val(M);
          break;
        case MIDI_PITCH_WHEEL:
          V1 = M&0x7f;
          V2 = (M&0x3f80) >> 7;
          break;
      }
      c = new ChangeEvent(e,V1,V2);
      set_undo(c);
      N++;
    }
    e = e->next;
  }
  undo_push(N);
}

int EventEdit::match_event_type(mevent* e){
  if(e->type == event_type){
    if(e->type == MIDI_CONTROLLER_CHANGE){
      if(e->value1 == controller_type){
        return 1;
      }
    }
    else{
      return 1;
    }
  }
  return 0;
 }

