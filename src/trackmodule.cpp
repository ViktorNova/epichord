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
#include <stdio.h>
#include <string.h>
#include <fltk/Group.h>
#include <fltk/Widget.h>

#include <fltk/draw.h>
#include <fltk/run.h>

#include <vector>

#include "ui.h"
#include "backend.h"
#include "uihelper.h"

extern std::vector<track*> tracks;
extern UI* ui;

extern char gm_names[128][64];

extern struct conf config;

void portcb(fltk::Widget* w, long i){
  //fltk::ValueInput* o = (fltk::ValueInput*)w;
  Gauge* o = (Gauge*)w;
  track* t = tracks[i];
  int old_port = t->port;
  t->port = (int)o->value;
  midi_channel_off(t->chan,old_port);
}

void chancb(fltk::Widget* w, long i){
  //fltk::ValueInput* o = (fltk::ValueInput*)w;
  Gauge* o = (Gauge*)w;
  track* t = tracks[i];
  int old_chan = t->chan;
  t->chan = (int)o->value;
  midi_channel_off(old_chan,t->port);
}

void progcb(fltk::Widget* w, long i){
  //fltk::ValueInput* o = (fltk::ValueInput*)w;
  Gauge* o = (Gauge*)w;
  track* t = tracks[i];
  int prog = (int)o->value;
  t->prog = prog;
  program_change(i, prog);
  if(config.autotrackname){
    strncpy(t->name,gm_names[prog],64);
    ui->track_info->update();
  }
}

void bankcb(fltk::Widget* w, long i){
  //fltk::ValueInput* o = (fltk::ValueInput*)w;
  Gauge* o = (Gauge*)w;
  track* t = tracks[i];
  int bank = (int)o->value;
  t->bank = bank;
  midi_bank_controller(i, bank);
}

void namecb(fltk::Widget* w, long i){
  fltk::Input* o = (fltk::Input*)w;
  track* t = tracks[i];
  strncpy(t->name,o->text(),MAX_TRACK_NAME);
}

void mutecb(fltk::Widget* w, long i){
  Toggle* o = (Toggle*)w;
  track* t = tracks[i];

  if(t->mute == 0){
    t->mute = 1;
    midi_track_off(i);
    o->set(1);
  }
  else{
    t->mute = 0;
    o->set(0);
  }
  o->redraw();
}

void solocb(fltk::Widget* w, long i){
  Toggle* o = (Toggle*)w;
  track* t = tracks[i];
  if(t->solo == 0){
    ui->track_info->unset_solo();
    t->solo = 1;
    set_solo(1);
    all_notes_off();
    o->set(1);
  }
  else{
    t->solo = 0;
    set_solo(0);
    o->set(0);
  }
  o->redraw();
}

void reccb(fltk::Widget* w, long i){
  Toggle* o = (Toggle*)w;
  track* t = tracks[i];
  if(o->state == 0){
    ui->track_info->set_rec(i);
    ui->keyboard->cur_chan = t->chan;
    ui->keyboard->cur_port = t->port;
    set_rec_track(i);
  }
  o->redraw();
}

void volcb(fltk::Widget* w, long i){
  Gauge* o = (Gauge*)w;
  track* t = tracks[i];

  t->vol = o->value;
  midi_volume_controller(i, t->vol);
  //perhaps record this
}

void pancb(fltk::Widget* w, long i){
  Gauge* o = (Gauge*)w;
  track* t = tracks[i];

  t->pan = o->value;
  midi_pan_controller(i, t->pan);
}

TrackModule::TrackModule(int x, int y, int w, int h, int i, const char* label) :
  fltk::Group(x, y, w, h, label),
  rec(5,5,20,20),
  name(25,5,145,20),
  volume(170,5,20,20),
  pan(190,5,20,20),
  solo(210,5,20,20),
  mute(230,5,20,20),
  port(230,5,20,20),
  chan(210,5,20,20),
  prog(170,5,20,20),
  bank(190,5,20,20)
  {

  port.hide();
  chan.hide();
  prog.hide();
  bank.hide();

  prog.r = 0x3b;
  prog.g = 0x5a;
  prog.b = 0x88;
  prog.R = 0x94;
  prog.G = 0xbf;
  prog.B = 0xff;
  prog.value = 0;
  prog.last_value = 0;
  prog.label_always = 1;
  prog.gauge_off = 1;
  prog.tooltip("program");

  bank.r = 0x2c;
  bank.g = 0x40;
  bank.b = 0x5b;
  bank.R = 0x5a;
  bank.G = 0x83;
  bank.B = 0xba;
  bank.value = 0;
  bank.last_value = 0;
  bank.label_always = 1;
  bank.gauge_off = 1;
  bank.tooltip("bank");

  chan.r = 0x8c;
  chan.g = 0xaf;
  chan.b = 0xb6;
  chan.R = 0xeb;
  chan.G = 0xef;
  chan.B = 0xf0;
  chan.max = 15;
  chan.label_always = 1;
  chan.label_plusone = 1;
  chan.label_hex = 0;
  chan.gauge_off = 1;
  chan.tooltip("channel");

  port.r = 0xc0;
  port.g = 0xc0;
  port.b = 0xc0;
  port.R = 0xff;
  port.G = 0xff;
  port.B = 0xff;
  port.max = 7;
  port.value = 0;
  port.last_value = 0;
  port.label_always = 1;
  port.gauge_off = 1;
  port.tooltip("port");


  port.callback(portcb, i);
  chan.callback(chancb, i);
  prog.callback(progcb, i);
  bank.callback(bankcb, i);
  rec.callback(reccb, i);
  name.callback(namecb, i);
  volume.callback(volcb, i);
  pan.callback(pancb, i);
  solo.callback(solocb, i);
  mute.callback(mutecb, i);

  rec.tooltip("record on this track");
  volume.tooltip("volume");
  pan.tooltip("pan");
  solo.tooltip("solo");
  mute.tooltip("mute");

  rec.key_flag = 1;
  solo.c[0] = '1';
  solo.r = 127;
  solo.g = 0; 
  solo.b = 127;
  solo.R = 255;
  solo.G = 0; 
  solo.B = 255;
  mute.c[0] = '.';
  mute.c[1] = '.';
  mute.c[2] = '.';
  mute.c[3] = '\0';
  mute.r = 0;
  mute.g = 0;
  mute.b = 127;
  mute.R = 0;
  mute.G = 0; 
  mute.B = 255;

  begin();

  add(rec);//toggle recording
  add(name);//change track name

  add(volume);//set track volume
  add(pan);//set track pan
  add(solo);//set solo to this track
  add(mute);//mute or unmute track

  add(port);//change track port
  add(chan);//change track channel
  add(prog);//change track program
  add(bank);

  end();

  index = i;
  settings_shown = 0;

}

int TrackModule::handle(int e){
  return fltk::Group::handle(e);
}

void TrackModule::toggle(){
  if(!settings_shown){
    volume.hide();
    pan.hide();
    solo.hide();
    mute.hide();
    port.show();
    chan.show();
    prog.show();
    bank.show();
    settings_shown = 1;
  }
  else{
    volume.show();
    pan.show();
    solo.show();
    mute.show();
    port.hide();
    chan.hide();
    prog.hide();
    bank.hide();
    settings_shown = 0;
  }
}


void TrackModule::set_channel(int i){
  chan.value = i;
  chan.last_value = i;
}

void TrackModule::unset_solo(){
  if(tracks[index]->solo){
    tracks[index]->solo = 0;
    solo.set(0);
    solo.redraw();
  }
}

void TrackModule::unset_rec(){
  rec.set(0);
  rec.redraw();
}

void TrackModule::set_rec(){
  rec.set(1);
  rec.redraw();
}


void TrackModule::update(){
  track* t = tracks[index];
  name.text(t->name);
  port.value = t->port;
  chan.value = t->chan;
  prog.value = t->prog;
  bank.value = t->bank;

  if(t->mute){
    mute.set(1);
  }
  else{
    mute.set(0);
  }

  if(t->solo){
    solo.set(1);
  }
  else{
    solo.set(0);
  }

  mute.redraw();
  solo.redraw();
}



void gauge_temp_cb(void* v){
  Gauge* g = (Gauge*)v;

  if(g->label_temp==0){
    return;
  }
  g->label_temp--;
  if(g->label_temp==0){
    g->redraw();
  }
  else{
    //fltk::repeat_timeout(0.5, gauge_temp_cb, v);
  }
}

Gauge::Gauge(int x, int y, int w, int h, const char* label) :
  fltk::Widget(x, y, w, h, label){
  max = 127;
  value = 64;
  last_value = 64;
  label_flag = 0;
  label_always = 0;
  label_plusone = 0;
  label_hex = 1;
  gauge_off = 0;
  label_temp=0;
  sens=1;
}

VGauge::VGauge(int x, int y, int w, int h, const char* label) :
  Gauge(x, y, w, h, label){
  r = 127;
  g = 0;
  b = 0;
  R = 255;
  G = 0;
  B = 0;
}
int last;
int VGauge::handle(int e){
  if(e == fltk::MOUSEWHEEL){
    value-=fltk::event_dy();
    if(value > max){value = max;}
    if(value < 0){value = 0;}
    last_value = value;
    label_temp++;
    fltk::add_timeout(1,gauge_temp_cb,this);
    do_callback();
    redraw();
    return 1;
  }
  if(e == fltk::PUSH){
    last = fltk::event_y();
    label_flag = 1;
    redraw();
    return 1;
  }
  if(e == fltk::DRAG){
    value += (last - fltk::event_y());
    if(value > max){value = max;}
    if(value < 0){value = 0;}
    if(value != last_value){
      last_value = value;
      do_callback();
    }
    last = fltk::event_y();
    redraw();
    return 1;
  }
  if(e == fltk::RELEASE){
    label_flag = 0;
    redraw();
  }
  return fltk::Widget::handle(e);
}


HGauge::HGauge(int x, int y, int w, int h, const char* label) :
  Gauge(x, y, w, h, label){
  r = 0;
  g = 127;
  b = 0;
  R = 0;
  G = 255;
  B = 0;
}

int HGauge::handle(int e){
  if(e == fltk::MOUSEWHEEL){
    value-=fltk::event_dy();
    if(value > max){value = max;}
    if(value < 0){value = 0;}
    last_value = value;
    label_temp++;
    fltk::add_timeout(1,gauge_temp_cb,this);
    do_callback();
    redraw();
    return 1;
  }
  if(e == fltk::PUSH){
    last = fltk::event_x();
    label_flag = 1;
    redraw();
    return 1;
  }
  if(e == fltk::DRAG){
    value += fltk::event_x() - last;
    if(value > max){value = max;}
    if(value < 0){value = 0;}
    if(value != last_value){
      last_value = value;
      do_callback();
    }
    last = fltk::event_x();
    redraw();
    return 1;
  }
  if(e == fltk::RELEASE){
    label_flag = 0;
    redraw();
  }
  return fltk::Widget::handle(e);
}

void VGauge::draw(){
  draw_box();
  fltk::setcolor(fltk::color(r,g,b));
  fltk::fillrect(2,2,w()-4,h()-4);

  if(!gauge_off){
    fltk::setcolor(fltk::color(R,G,B));
  }
  int H = value * (h()-4) / max;
  fltk::fillrect(2,h()-2-H,w()-4,H);

  if(label_flag || label_always || label_temp){
    char buf[3];
    int V = label_plusone ? value + 1 : value;
    if(label_hex){
      snprintf(buf,3,"%x",V);
    }
    else{
      snprintf(buf,3,"%d",V);
    }


    fltk::push_clip(2,2,w()-4,h()-4 - H);
    fltk::setcolor(fltk::color(R,G,B));
    fltk::setfont(fltk::HELVETICA,12);
    int W = (int)fltk::getwidth(buf);
    fltk::drawtext(buf,(w()-W)/2,h()-fltk::getascent()/2);
    fltk::pop_clip();


    fltk::push_clip(2,h()-2-H,w()-4,H);
    if(!gauge_off){
      fltk::setcolor(fltk::color(r,g,b));
    }
    fltk::setfont(fltk::HELVETICA,12);
    fltk::drawtext(buf,(w()-W)/2,h()-fltk::getascent()/2);
    fltk::pop_clip();
  }
}

void HGauge::draw(){
  draw_box();
  fltk::setcolor(fltk::color(r,g,b));
  fltk::fillrect(2,2,w()-4,h()-4);

  fltk::setcolor(fltk::color(R,G,B));
  int V = value * (h()-4) / max;
  fltk::fillrect(2,2,V,h()-4);

  if(label_flag || label_always || label_temp){
    char buf[3];
    snprintf(buf,3,"%x",value);

    fltk::push_clip(V+2,2,w()-4-V,h()-4);
    fltk::setcolor(fltk::color(R,G,B));
    fltk::setfont(fltk::HELVETICA,12);
    int W = (int)fltk::getwidth(buf);
    fltk::drawtext(buf,(w()-W)/2,h()-fltk::getascent()/2);
    fltk::pop_clip();

    fltk::push_clip(2,2,V,h()-4);
    fltk::setcolor(fltk::color(r,g,b));
    fltk::setfont(fltk::HELVETICA,12);
    fltk::drawtext(buf,(w()-W)/2,h()-fltk::getascent()/2);
    fltk::pop_clip();
  }
}

Toggle::Toggle(int x, int y, int w, int h, const char* label) :
  fltk::Button(x, y, w, h, label){
  buttonbox(fltk::DOWN_BOX);
  when(fltk::WHEN_NEVER);
  c[0] = 'A';
  c[1] = '\0';
  r = 127;
  g = 0;
  b = 0;
  R = 255;
  G = 0;
  B = 0;
  state = 0;
}

int Toggle::handle(int e){
  if(e==fltk::PUSH){
    do_callback();
    return 1;
  }
  return fltk::Button::handle(e);
}

void Toggle::draw(){
  draw_box();

  if(key_flag){ //instead of drawing normally do something completely different
    //redirect complaints to /dev/null
    if(state){
      fltk::setcolor(fltk::BLACK);
      fltk::fillrect(2,h()-3,w()-4,1);
      fltk::fillrect(w()-3,2,1,h()-4);
      fltk::fillrect(1,1,w()-4,1);
      fltk::fillrect(1,1,1,h()-4);

      fltk::fillrect(6,2,1,h()-4);
      fltk::fillrect(12,2,1,h()-4);

      fltk::fillrect(5,2,3,9);
      fltk::fillrect(11,2,3,9);
    }
    else{
      fltk::Button::draw(0);
    }
    return;
  }

  if(state){
     fltk::setcolor(fltk::color(R,G,B));
  }
  else{
     fltk::setcolor(fltk::color(r,g,b));
  }
  fltk::fillrect(2,2,w()-4,h()-4);

  fltk::setcolor(fltk::BLACK);
  fltk::setfont(fltk::HELVETICA,12);
  int W = (int)fltk::getwidth(c);
  fltk::drawtext(c,(w()-W)/2,h()-fltk::getascent()/2);
}


void Toggle::set(int s){
  state = s;
}
