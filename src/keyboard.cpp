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

struct combo {
  unsigned int key;
  unsigned int mod;
  combo(unsigned zkey, unsigned zmod){key=zkey; mod=zmod;}
  combo(){key=' '; mod=0;}
  int operator==(combo c){
    return (c.key==key && c.mod==mod) ? 1 : 0;
  }
};

namespace keymap{

  combo lower[18];
  combo upper[21];
  combo octaveup;
  combo octavedown;
  combo zoomin;
  combo zoomout;

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

  keymap::lower[0] = combo('z',0);
  keymap::lower[1] = combo('s',0);
  keymap::lower[2] = combo('x',0);
  keymap::lower[3] = combo('d',0);
  keymap::lower[4] = combo('c',0);
  keymap::lower[5] = combo('v',0);
  keymap::lower[6] = combo('g',0);
  keymap::lower[7] = combo('b',0);
  keymap::lower[8] = combo('h',0);
  keymap::lower[9] = combo('n',0);
  keymap::lower[10] = combo('j',0);
  keymap::lower[11] = combo('m',0);
  keymap::lower[12] = combo(',',0);
  keymap::lower[13] = combo('l',0);
  keymap::lower[14] = combo('.',0);
  keymap::lower[15] = combo(';',0);
  keymap::lower[16] = combo('/',0);
  keymap::lower[17] = combo('\'',0);
  keymap::lower[18] = combo(' ',0);

  keymap::upper[0] = combo('q',0);
  keymap::upper[1] = combo('2',0);
  keymap::upper[2] = combo('w',0);
  keymap::upper[3] = combo('3',0);
  keymap::upper[4] = combo('e',0);
  keymap::upper[5] = combo('r',0);
  keymap::upper[6] = combo('5',0);
  keymap::upper[7] = combo('t',0);
  keymap::upper[8] = combo('6',0);
  keymap::upper[9] = combo('y',0);
  keymap::upper[10] = combo('7',0);
  keymap::upper[11] = combo('u',0);
  keymap::upper[12] = combo('i',0);
  keymap::upper[13] = combo(' ',0);
  keymap::upper[14] = combo('o',0);
  keymap::upper[15] = combo('0',0);
  keymap::upper[16] = combo('p',0);
  keymap::upper[17] = combo('[',0);
  keymap::upper[18] = combo('=',0);
  keymap::upper[19] = combo(']',0);
  keymap::upper[20] = combo('\\',0);

  keymap::octaveup = combo(']',fltk::SHIFT);
  keymap::octavedown = combo('[',fltk::SHIFT);
  keymap::zoomin = combo('=',fltk::SHIFT);
  keymap::zoomout = combo('-',0);

}



int keyboard_handler(int e, fltk::Window* w){
  combo E = combo(event_key(),event_state());
  switch(e){
    case fltk::SHORTCUT:
      if(fltk::event_key_repeated()){
        return 0;
      }
      if(E==combo(fltk::SpaceKey,0)){
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
      else if(E==keymap::upper[18])
        ui->keyboard->kb_play_note(30);
      else if(E==keymap::upper[19])
        ui->keyboard->kb_play_note(31);
      else if(E==keymap::upper[20])
        ui->keyboard->kb_play_note(32);


      return 1;
    case fltk::KEYUP:
      if(E==combo(fltk::SpaceKey,0)){
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
      else if(E==keymap::upper[18])
        ui->keyboard->kb_release_note(30);
      else if(E==keymap::upper[19])
        ui->keyboard->kb_release_note(31);
      else if(E==keymap::upper[20])
        ui->keyboard->kb_release_note(32);
      else if(E==keymap::upper[21])
        ui->keyboard->kb_release_note(33);
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
      if(event_y() > h() || event_y() < 12){
        return 1;
      }
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
  key = 'a';
  mod = 0;
  strncpy(str,get_keystring(key,mod),32);
  in_flag = 0;
}

int KeyGrabber::handle(int event){
int specialok=0;
  switch(event){
    case FOCUS:
      return 1;
    case PUSH:
      take_focus();
      in_flag = 1;
      set_key(' ',0);
      redraw();
      return 1;
      break;
    case KEY:
      if(in_flag){
        switch(event_key()){
          case fltk::Keypad:
          case fltk::KeypadEnter:
          case fltk::MultiplyKey:
          case fltk::AddKey:
          case fltk::SubtractKey:
          case fltk::DecimalKey:
          case fltk::DivideKey:
          case fltk::Keypad0:
          case fltk::Keypad1:
          case fltk::Keypad2:
          case fltk::Keypad3:
          case fltk::Keypad4:
          case fltk::Keypad5:
          case fltk::Keypad6:
          case fltk::Keypad7:
          case fltk::Keypad8:
          case fltk::Keypad9:
          case fltk::KeypadLast:
            specialok = 1;
            break;
        }
        if(specialok || event_key()<0x80){
          set_key(event_key(),event_state());
          do_callback();
          in_flag = 0;
          redraw();
          return 1;
        }
        return 0;
      }
      break;
  }
  return 0;
}

void KeyGrabber::draw(){
  draw_box();
  setfont(fltk::COURIER,10);
  fltk::push_clip(0,0,w(),h());

  if(in_flag){
    fltk::setcolor(fltk::color(128,128,255));
  }
  else{
    fltk::setcolor(fltk::WHITE);
  }
  fillrect(2,2,w()-4,h()-4);

  fltk::setcolor(fltk::BLACK);

  drawtext(str,2,12);
  pop_clip();
}

int KeyGrabber::set_key(int zkey, int zmod){
  key = zkey;
  mod = zmod;
  strncpy(str,get_keystring(key,mod),32);
  redraw();
}


void set_keymap(int which, int index, int key, int mod){
  switch(which){
    case 0://lower
      keymap::lower[index] = combo(key,mod);
      break;
    case 1://upper
      keymap::upper[index] = combo(key,mod);
      break;
    case 2://zoomin
      keymap::zoomin = combo(key,mod);
      break;
    case 3://zoomout
      keymap::zoomout = combo(key,mod);
      break;
    case 4://octave
      keymap::octaveup = combo(key,mod);
      break;
    case 5://octdown
      keymap::octavedown = combo(key,mod);
      break;

  }
}

char keystring[32] = "";
char* get_keystring(int key, int mod){
  keystring[0] = '\0';
  if(key == ' '){
    keystring[0] = '\0';
    return keystring;
  }
/*
  fltk::SHIFT, fltk::CAPSLOCK, fltk::CTRL, fltk::ALT,
  fltk::NUMLOCK, fltk::META, fltk::SCROLLLOCK, fltk::BUTTON1,
  fltk::BUTTON2, fltk::BUTTON3, fltk::ANY_BUTTON, fltk::ACCELERATOR,
  fltk::COMMAND, fltk::OPTION
*/


  if(mod&fltk::SHIFT) strcpy(keystring,"shft+");
  else if(mod&fltk::CTRL) strcpy(keystring,"ctrl+");
  else if(mod&fltk::ALT) strcpy(keystring,"alt+");
  else if(mod&fltk::META) strcpy(keystring,"meta+");
  else if(mod&fltk::OPTION) strcpy(keystring,"opt+");

  int N = strlen(keystring);
/*
 fltk::LeftButton, fltk::MiddleButton, fltk::RightButton, fltk::SpaceKey,
  fltk::BackSpaceKey, fltk::TabKey, fltk::ClearKey, fltk::ReturnKey,
  fltk::PauseKey, fltk::ScrollLockKey, fltk::EscapeKey, fltk::HomeKey,
  fltk::LeftKey, fltk::UpKey, fltk::RightKey, fltk::DownKey,
  fltk::PageUpKey, fltk::PageDownKey, fltk::EndKey, fltk::PrintKey,
  fltk::InsertKey, fltk::MenuKey, fltk::HelpKey, fltk::NumLockKey,
  fltk::Keypad, fltk::KeypadEnter, fltk::MultiplyKey, fltk::AddKey,
  fltk::SubtractKey, fltk::DecimalKey, fltk::DivideKey, fltk::Keypad0,
  fltk::Keypad1, fltk::Keypad2, fltk::Keypad3, fltk::Keypad4,
  fltk::Keypad5, fltk::Keypad6, fltk::Keypad7, fltk::Keypad8,
  fltk::Keypad9, fltk::KeypadLast, fltk::F0Key, fltk::F1Key,
  fltk::F2Key, fltk::F3Key, fltk::F4Key, fltk::F5Key,
  fltk::F6Key, fltk::F7Key, fltk::F8Key, fltk::F9Key,
  fltk::F10Key, fltk::F11Key, fltk::F12Key, fltk::LastFunctionKey,
  fltk::LeftShiftKey, fltk::RightShiftKey, fltk::LeftCtrlKey, fltk::RightCtrlKey,
  fltk::CapsLockKey, fltk::LeftMetaKey, fltk::RightMetaKey, fltk::LeftAltKey,
  fltk::RightAltKey, fltk::DeleteKey, LeftAccKey, RightAccKey,
  LeftCmdKey, RightCmdKey 
*/
  char k[4];
  k[0] = key;
  k[1] = '\0';
  switch(key){
    //case fltk::SpaceKey: strcpy(keystring+N,"spc"); break;
    //case fltk::BackSpaceKey: strcpy(keystring+N,"bksp"); break;
    case fltk::KeypadEnter: strcpy(keystring+N,"enter"); break;
    case fltk::MultiplyKey: strcpy(keystring+N,"mult"); break;
    case fltk::AddKey: strcpy(keystring+N,"plus"); break;
    case fltk::SubtractKey: strcpy(keystring+N,"minus"); break;
    case fltk::DecimalKey: strcpy(keystring+N,"dec"); break;
    case fltk::DivideKey: strcpy(keystring+N,"div"); break;
    case fltk::Keypad0: strcpy(keystring+N,"0"); break;
    case fltk::Keypad1: strcpy(keystring+N,"1"); break;
    case fltk::Keypad2: strcpy(keystring+N,"2"); break;
    case fltk::Keypad3: strcpy(keystring+N,"3"); break;
    case fltk::Keypad4: strcpy(keystring+N,"4"); break;
    case fltk::Keypad5: strcpy(keystring+N,"5"); break;
    case fltk::Keypad6: strcpy(keystring+N,"6"); break;
    case fltk::Keypad7: strcpy(keystring+N,"7"); break;
    case fltk::Keypad8: strcpy(keystring+N,"8"); break;
    case fltk::Keypad9: strcpy(keystring+N,"9"); break;
    default: strcpy(keystring+N,k); break;
  }

  return keystring;
}


void load_keymap(){

  //TODO load from file instead
  ui->kg_l0->set_key('z',0);
  ui->kg_l1->set_key('s',0);
  ui->kg_l2->set_key('x',0);
  ui->kg_l3->set_key('d',0);
  ui->kg_l4->set_key('c',0);
  ui->kg_l5->set_key('v',0);
  ui->kg_l6->set_key('g',0);
  ui->kg_l7->set_key('b',0);
  ui->kg_l8->set_key('h',0);
  ui->kg_l9->set_key('n',0);
  ui->kg_l10->set_key('j',0);
  ui->kg_l11->set_key('m',0);
  ui->kg_l12->set_key(',',0);
  ui->kg_l13->set_key('l',0);
  ui->kg_l14->set_key('.',0);
  ui->kg_l15->set_key(';',0);
  ui->kg_l16->set_key('/',0);

  ui->kg_u0->set_key('q',0);
  ui->kg_u1->set_key('2',0);
  ui->kg_u2->set_key('w',0);
  ui->kg_u3->set_key('3',0);
  ui->kg_u4->set_key('e',0);
  ui->kg_u5->set_key('r',0);
  ui->kg_u6->set_key('5',0);
  ui->kg_u7->set_key('t',0);
  ui->kg_u8->set_key('6',0);
  ui->kg_u9->set_key('y',0);
  ui->kg_u10->set_key('7',0);
  ui->kg_u11->set_key('u',0);
  ui->kg_u12->set_key('i',0);
  ui->kg_u13->set_key('9',0);
  ui->kg_u14->set_key('o',0);
  ui->kg_u15->set_key('0',0);
  ui->kg_u16->set_key('p',0);
  ui->kg_u17->set_key('[',0);
  ui->kg_u18->set_key('=',0);
  ui->kg_u19->set_key(']',0);
  ui->kg_u20->set_key('\\',0);

  ui->kg_zi->set_key('=',fltk::SHIFT);
  ui->kg_zo->set_key('-',0);
  ui->kg_ou->set_key(']',fltk::SHIFT);
  ui->kg_od->set_key('[',fltk::SHIFT);
}


int zoom_out_key(unsigned key,unsigned mod){
  if(combo(key,mod)==keymap::zoomout){return 1;}
  return 0;
}

int zoom_in_key(unsigned key,unsigned mod){
  if(combo(key,mod)==keymap::zoomin){return 1;}
  return 0;
}
