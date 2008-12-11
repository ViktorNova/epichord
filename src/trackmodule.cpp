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

#include <vector>

#include "ui.h"
#include "backend.h"

extern std::vector<track*> tracks;
extern UI* ui;

void portcb(fltk::Widget* w, long i){
  fltk::ValueInput* o = (fltk::ValueInput*)w;
  track* t = tracks[i];
  int old_port = t->port;
  t->port = (int)o->value();
  midi_channel_off(t->chan,old_port);
}

void chancb(fltk::Widget* w, long i){
  fltk::ValueInput* o = (fltk::ValueInput*)w;
  track* t = tracks[i];
  int old_chan = t->chan;
  t->chan = (int)o->value();
  midi_channel_off(old_chan,t->port);
}

void progcb(fltk::Widget* w, long i){
  fltk::ValueInput* o = (fltk::ValueInput*)w;
  track* t = tracks[i];
  int prog = (int)o->value();
  t->prog = prog;
  program_change(i, prog);
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

void volcb(fltk::Widget* w, long i){
  Gauge* o = (Gauge*)w;
  track* t = tracks[i];

  t->vol = o->value;
}

void pancb(fltk::Widget* w, long i){
  Gauge* o = (Gauge*)w;
  track* t = tracks[i];

  t->pan = o->value;
}

TrackModule::TrackModule(int x, int y, int w, int h, int i, const char* label) :
  fltk::Group(x, y, w, h, label),
  multi(5,5,10,20),
  name(20,5,150,20),
  volume(170,5,20,20),
  pan(190,5,20,20),
  solo(210,5,20,20),
  mute(230,5,20,20),
  port(50,5,40,20),
  chan(130,5,40,20),
  prog(210,5,40,20)
   {

  port.hide();
  chan.hide();
  prog.hide();

  port.label("port");
  chan.label("chan");
  prog.label("prog");

  port.maximum(7);
  port.minimum(0);
  port.step(1);
  port.value(1);//make it do value_damage()
  port.value(0);
  chan.maximum(15);
  chan.minimum(0);
  chan.step(1);
  chan.value(1);
  chan.value(0);
  prog.maximum(127);
  prog.minimum(0);
  prog.step(1);
  prog.value(1);
  prog.value(0);

  port.callback(portcb, i);
  chan.callback(chancb, i);
  prog.callback(progcb, i);
  //multi.callback();
  name.callback(namecb, i);
  volume.callback(volcb, i);
  pan.callback(pancb, i);
  solo.callback(solocb, i);
  mute.callback(mutecb, i);

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

  add(multi);//delete track
  add(name);//change track name

  add(volume);//set track volume
  add(pan);//set track pan
  add(solo);//set solo to this track
  add(mute);//mute or unmute track

  add(port);//change track port
  add(chan);//change track channel
  add(prog);//change track program

  end();

  index = i;
  settings_shown = 0;

}

int TrackModule::handle(int e){
  return fltk::Group::handle(e);
}

void TrackModule::toggle(){
  if(!settings_shown){
    name.hide();
    volume.hide();
    pan.hide();
    solo.hide();
    mute.hide();
    port.show();
    chan.show();
    prog.show();
    settings_shown = 1;
  }
  else{
    name.show();
    volume.show();
    pan.show();
    solo.show();
    mute.show();
    port.hide();
    chan.hide();
    prog.hide();
    settings_shown = 0;
  }
}


void TrackModule::set_channel(int i){
  chan.value(i);
}

void TrackModule::unset_solo(){
  if(tracks[index]->solo){
    tracks[index]->solo = 0;
    solo.set(0);
    solo.redraw();
  }
}

void TrackModule::update(){
  track* t = tracks[index];
  name.text(t->name);
  port.value(t->port);
  chan.value(t->chan);
  prog.value(t->prog);

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

Gauge::Gauge(int x, int y, int w, int h, const char* label) :
  fltk::Widget(x, y, w, h, label){
  max = 127;
  value = 64;
  label_flag = 0;
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
  if(e == fltk::PUSH){
    last = fltk::event_y();
    label_flag = 1;
    redraw();
    return 1;
  }
  if(e == fltk::DRAG){
    value += last - fltk::event_y();
    do_callback();
    last = fltk::event_y();
    if(value > max){value = max;}
    if(value < 0){value = 0;}
    redraw();
    return 1;
  }
  if(e == fltk::RELEASE){
    label_flag = 0;
    redraw();
  }
  return 0;
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
  if(e == fltk::PUSH){
    last = fltk::event_x();
    label_flag = 1;
    redraw();
    return 1;
  }
  if(e == fltk::DRAG){
    value += fltk::event_x() - last;
    do_callback();
    last = fltk::event_x();
    if(value > max){value = max;}
    if(value < 0){value = 0;}
    redraw();
    return 1;
  }
  if(e == fltk::RELEASE){
    label_flag = 0;
    redraw();
  }
  return 0;
}

void VGauge::draw(){
  draw_box();
  fltk::setcolor(fltk::color(r,g,b));
  fltk::fillrect(2,2,w()-4,h()-4);

  fltk::setcolor(fltk::color(R,G,B));
  int H = value * (h()-4) / max;
  fltk::fillrect(2,h()-2-H,w()-4,H);

  if(label_flag){
    char buf[3];
    snprintf(buf,3,"%x",value);

    fltk::push_clip(2,2,w()-4,h()-4 - H);
    fltk::setcolor(fltk::color(R,G,B));
    fltk::setfont(fltk::HELVETICA,12);
    int W = (int)fltk::getwidth(buf);
    fltk::drawtext(buf,(w()-W)/2,h()-fltk::getascent()/2);
    fltk::pop_clip();

    fltk::push_clip(2,h()-2-H,w()-4,H);
    fltk::setcolor(fltk::color(r,g,b));
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

  if(label_flag){
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
  fltk::Widget(x, y, w, h, label){
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
  return 0;
}

void Toggle::draw(){
  draw_box();
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
