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

#include "eventmenu.h"

#include "uihelper.h"


extern struct conf config;

extern UI* ui;
extern std::vector<track*> tracks;

extern char controller_names_short[128][64];

using namespace fltk;

EventMenu::EventMenu(int x, int y, int w, int h, const char* label) : fltk::Widget(x, y, w, h, label) {
  scroll = 0;
  scroll_orig = 0;
  drag_orig_x=0;
}

int EventMenu::handle(int event){
  int X,Y,i;
  switch(event){
    case FOCUS:
      return 1;
    case PUSH:
      X=event_x();
      Y=event_y();
      if(event_button()==1){//switch to event
        i = xy2menu_index(X,Y);
        switch(i){
          case 0: ui->event_edit->set_event_type(MIDI_NOTE_ON,0); break;
          case 1: ui->event_edit->set_event_type(MIDI_NOTE_OFF,0); break;
          case 2: ui->event_edit->set_event_type(MIDI_AFTERTOUCH,0); break;
          case 3: ui->event_edit->set_event_type(MIDI_PROGRAM_CHANGE,0); break;
          case 4: ui->event_edit->set_event_type(MIDI_CHANNEL_PRESSURE,0); break;
          case 5: ui->event_edit->set_event_type(MIDI_PITCH_WHEEL,0); break;
          default: 
            ui->event_edit->set_event_type(MIDI_CONTROLLER_CHANGE,i-6);
            break;
        }
        ui->event_edit->show();
        hide();
        ui->event_menu_button->state(0);
      }
      else if(event_button()==3){//erase events
        drag_orig_x = X;
        scroll_orig = scroll;
      }
      redraw();
      return 1;
      break;
    case DRAG:
      if(event_button()==3){
        scroll = scroll_orig + (drag_orig_x - event_x());
        if(scroll<0){scroll=0;}
        if(scroll>1000){scroll=1000;}
        redraw();
      }
      break;
  }
  return 0;
}

void EventMenu::draw(){
  fltk::setfont(fltk::COURIER,10);
  fltk::setcolor(fltk::GRAY05);
  fltk::fillrect(0,0,w(),h());

  fltk::push_clip(0,0,w(),h());

  fltk::Color c1 = fltk::GRAY50;
  fltk::Color c2 = fltk::GRAY80;

  char buf[16] = "booya";

  for(int i=0; i<134; i++){
    if(ui->event_edit->has[i])
      fltk::setcolor(c2);
    else
      fltk::setcolor(c1);
    switch(i){
      case 0: strcpy(buf,"noteon"); break;
      case 1: strcpy(buf,"noteoff"); break;
      case 2: strcpy(buf,"aftertouch"); break;
      case 3: strcpy(buf,"program"); break;
      case 4: strcpy(buf,"pressure"); break;
      case 5: strcpy(buf,"pitchwheel"); break;
      default: strncpy(buf,controller_names_short[i-6],16); break;
    }
    fltk::drawtext(buf,i/6 * 64-scroll,12+(12*(i % 6)));
  }

  fltk::pop_clip();

}

int EventMenu::xy2menu_index(int X, int Y){
  int W = 64;
  int H = 12;
  int I = (X+scroll)/W * 6 + Y*6/h();
  if(I>133){return 133;}
  else{return I;}
}

