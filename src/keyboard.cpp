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

namespace keymap{

  int lower[18];
  int upper[18];
  int octaveup;
  int octavedown;
  int zoomin;
  int zoomout;

};

Keyboard::Keyboard(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {
  sustain = 0;
  cur_note = -1;
  fltk::add_event_handler(keyboard_handler);

  for(int i=0; i<128; i++){
    ons[i]=0;
    helds[i]=0;
  }

  cur_port = 0;
  cur_chan = 0;

  octave = 4;

  keymap::lower[0] = 'z';
  keymap::lower[1] = 's';
  keymap::lower[2] = 'x';
  keymap::lower[3] = 'd';
  keymap::lower[4] = 'c';
  keymap::lower[5] = 'v';
  keymap::lower[6] = 'g';
  keymap::lower[7] = 'b';
  keymap::lower[8] = 'h';
  keymap::lower[9] = 'n';
  keymap::lower[10] = 'j';
  keymap::lower[11] = 'm';
  keymap::lower[12] = ',';
  keymap::lower[13] = 'l';
  keymap::lower[14] = '.';
  keymap::lower[15] = ';';
  keymap::lower[16] = '/';
  keymap::lower[17] = '\'';
  keymap::lower[18] = ' ';

  keymap::upper[0] = 'q';
  keymap::upper[1] = '2';
  keymap::upper[2] = 'w';
  keymap::upper[3] = '3';
  keymap::upper[4] = 'e';
  keymap::upper[5] = 'r';
  keymap::upper[6] = '5';
  keymap::upper[7] = 't';
  keymap::upper[8] = '6';
  keymap::upper[9] = 'y';
  keymap::upper[10] = '7';
  keymap::upper[11] = 'u';
  keymap::upper[12] = 'i';
  keymap::upper[13] = ' ';
  keymap::upper[14] = 'o';
  keymap::upper[15] = '0';
  keymap::upper[16] = 'p';
  keymap::upper[17] = ' ';

  keymap::octaveup = ']';
  keymap::octavedown = '[';
  keymap::zoomin = '=';
  keymap::zoomout = '-';

}



int keyboard_handler(int e, fltk::Window* w){
  int E = event_key();

  switch(e){
    case fltk::SHORTCUT:
      if(fltk::event_key_repeated()){
        return 0;
      }
      if(E==fltk::SpaceKey){
        ui->keyboard->set_sustain(1);
      }
      else if(E==keymap::octavedown){
        ui->keyboard->octave_down();
      }
      else if(E==keymap::octaveup){
        ui->keyboard->octave_up();
      }
      else if(E==keymap::lower[0])
        ui->keyboard->kb_play_note(0);
      else if(E==keymap::lower[1])
        ui->keyboard->kb_play_note(1);
      else if(E==keymap::lower[2])
        ui->keyboard->kb_play_note(2);
      else if(E==keymap::lower[3])
        ui->keyboard->kb_play_note(3);
      else if(E==keymap::lower[4])
        ui->keyboard->kb_play_note(4);
      else if(E==keymap::lower[5])
        ui->keyboard->kb_play_note(5);
      else if(E==keymap::lower[6])
        ui->keyboard->kb_play_note(6);
      else if(E==keymap::lower[7])
        ui->keyboard->kb_play_note(7);
      else if(E==keymap::lower[8])
        ui->keyboard->kb_play_note(8);
      else if(E==keymap::lower[9])
        ui->keyboard->kb_play_note(9);
      else if(E==keymap::lower[10])
        ui->keyboard->kb_play_note(10);
      else if(E==keymap::lower[11])
        ui->keyboard->kb_play_note(11);
      else if(E==keymap::lower[12])
        ui->keyboard->kb_play_note(12);
      else if(E==keymap::lower[13])
        ui->keyboard->kb_play_note(13);
      else if(E==keymap::lower[14])
        ui->keyboard->kb_play_note(14);
      else if(E==keymap::lower[15])
        ui->keyboard->kb_play_note(15);
      else if(E==keymap::lower[16])
        ui->keyboard->kb_play_note(16);
      else if(E==keymap::lower[17])
        ui->keyboard->kb_play_note(17);
      else if(E==keymap::upper[0])
        ui->keyboard->kb_play_note(12);
      else if(E==keymap::upper[1])
        ui->keyboard->kb_play_note(13);
      else if(E==keymap::upper[2])
        ui->keyboard->kb_play_note(14);
      else if(E==keymap::upper[3])
        ui->keyboard->kb_play_note(15);
      else if(E==keymap::upper[4])
        ui->keyboard->kb_play_note(16);
      else if(E==keymap::upper[5])
        ui->keyboard->kb_play_note(17);
      else if(E==keymap::upper[6])
        ui->keyboard->kb_play_note(18);
      else if(E==keymap::upper[7])
        ui->keyboard->kb_play_note(19);
      else if(E==keymap::upper[8])
        ui->keyboard->kb_play_note(20);
      else if(E==keymap::upper[9])
        ui->keyboard->kb_play_note(21);
      else if(E==keymap::upper[10])
        ui->keyboard->kb_play_note(22);
      else if(E==keymap::upper[11])
        ui->keyboard->kb_play_note(23);
      else if(E==keymap::upper[12])
        ui->keyboard->kb_play_note(24);
      else if(E==keymap::upper[13])
        ui->keyboard->kb_play_note(25);
      else if(E==keymap::upper[14])
        ui->keyboard->kb_play_note(26);
      else if(E==keymap::upper[15])
        ui->keyboard->kb_play_note(27);
      else if(E==keymap::upper[16])
        ui->keyboard->kb_play_note(28);
      else if(E==keymap::upper[17])
        ui->keyboard->kb_play_note(29);


      return 1;
    case fltk::KEYUP:
      if(E==fltk::SpaceKey){
        ui->keyboard->set_sustain(0);
      }
      else if(E==keymap::lower[0])
        ui->keyboard->kb_release_note(0);
      else if(E==keymap::lower[1])
        ui->keyboard->kb_release_note(1);
      else if(E==keymap::lower[2])
        ui->keyboard->kb_release_note(2);
      else if(E==keymap::lower[3])
        ui->keyboard->kb_release_note(3);
      else if(E==keymap::lower[4])
        ui->keyboard->kb_release_note(4);
      else if(E==keymap::lower[5])
        ui->keyboard->kb_release_note(5);
      else if(E==keymap::lower[6])
        ui->keyboard->kb_release_note(6);
      else if(E==keymap::lower[7])
        ui->keyboard->kb_release_note(7);
      else if(E==keymap::lower[8])
        ui->keyboard->kb_release_note(8);
      else if(E==keymap::lower[9])
        ui->keyboard->kb_release_note(9);
      else if(E==keymap::lower[10])
        ui->keyboard->kb_release_note(10);
      else if(E==keymap::lower[11])
        ui->keyboard->kb_release_note(11);
      else if(E==keymap::lower[12])
        ui->keyboard->kb_release_note(12);
      else if(E==keymap::lower[13])
        ui->keyboard->kb_release_note(13);
      else if(E==keymap::lower[14])
        ui->keyboard->kb_release_note(14);
      else if(E==keymap::lower[15])
        ui->keyboard->kb_release_note(15);
      else if(E==keymap::lower[16])
        ui->keyboard->kb_release_note(16);
      else if(E==keymap::lower[17])
        ui->keyboard->kb_release_note(17);
      else if(E==keymap::upper[0])
        ui->keyboard->kb_release_note(12);
      else if(E==keymap::upper[1])
        ui->keyboard->kb_release_note(13);
      else if(E==keymap::upper[2])
        ui->keyboard->kb_release_note(14);
      else if(E==keymap::upper[3])
        ui->keyboard->kb_release_note(15);
      else if(E==keymap::upper[4])
        ui->keyboard->kb_release_note(16);
      else if(E==keymap::upper[5])
        ui->keyboard->kb_release_note(17);
      else if(E==keymap::upper[6])
        ui->keyboard->kb_release_note(18);
      else if(E==keymap::upper[7])
        ui->keyboard->kb_release_note(19);
      else if(E==keymap::upper[8])
        ui->keyboard->kb_release_note(20);
      else if(E==keymap::upper[9])
        ui->keyboard->kb_release_note(21);
      else if(E==keymap::upper[10])
        ui->keyboard->kb_release_note(22);
      else if(E==keymap::upper[11])
        ui->keyboard->kb_release_note(23);
      else if(E==keymap::upper[12])
        ui->keyboard->kb_release_note(24);
      else if(E==keymap::upper[13])
        ui->keyboard->kb_release_note(25);
      else if(E==keymap::upper[14])
        ui->keyboard->kb_release_note(26);
      else if(E==keymap::upper[15])
        ui->keyboard->kb_release_note(27);
      else if(E==keymap::upper[16])
        ui->keyboard->kb_release_note(28);
      else if(E==keymap::upper[17])
        ui->keyboard->kb_release_note(29);
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
      if(ons[i]==1 && helds[i]==0){
        //midi_note_off(i,cur_chan,cur_port);
        release_note(i,1);
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
      cur_note = note;
      helds[note] = 1;
      play_note(note,1);
      return 1;
    case fltk::RELEASE:
      if(cur_note!=-1){
        helds[cur_note]=0;
      }
      cur_note=-1;
      if(sustain == 0){
        cut_notes(1);
        redraw();
      }
      return 1;
    case fltk::DRAG:
      note = ypix2note(event_y()+scroll, event_x() < w()/2 ? 1 : 0);
      if(cur_note != note){
        helds[cur_note]=0;
        cur_note = note;
        if(sustain == 0){
          cut_notes(1);
        }
        helds[note]=1;
        play_note(note,1);
      }
      return 1;
  }
  return 0;
}

void Keyboard::play_note(int note, int rec){
  if(ons[note]==1){
    return;
  }

  char buf[3];
  buf[0] = 0x90 | cur_chan;
  buf[1] = note;
  buf[2] = 0x7f;
  send_midi(buf,3,cur_port);

  ons[note] = 1;
  helds[note] = 1;

  if(rec && is_backend_recording()){
    send_midi_local(buf,3);
  }

  redraw();
}

void Keyboard::release_note(int note, int rec){
  if(ons[note]==0){
    return;
  }

  helds[note]=0;

  if(sustain==1){
    return;
  }

  char buf[3];
  buf[0] = 0x80 | cur_chan;
  buf[1] = note;
  buf[2] = 0x00;
  send_midi(buf,3,cur_port);

  ons[note] = 0;
  if(rec && is_backend_recording()){
    send_midi_local(buf,3);
  }

  redraw();
}

void Keyboard::kb_play_note(int note){
  int raw = octave*12 + note;
  if(raw < 128 && raw >= 0){
    play_note(raw,1);
  }
}

void Keyboard::kb_release_note(int note){
  int raw = octave*12 + note;
  if(raw < 128 && raw >= 0){
    release_note(raw,1);
  }
}

void Keyboard::octave_up(){
  if(octave < 9){
    cut_notes(1);
    octave++;
  }
}

void Keyboard::octave_down(){
  if(octave > 0){
    cut_notes(1);
    octave--;
  }
}

void Keyboard::cut_notes(int rec){
  for(int i=0; i<128; i++){
    if(ons[i]){
      release_note(i,rec);
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


KeyGrabber::KeyGrabber(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {
  key = ' ';
}

int KeyGrabber::handle(int event){
  return 0;
}

void KeyGrabber::draw(){
  draw_box();
}

