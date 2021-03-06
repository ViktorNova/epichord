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
#include <unistd.h>

#include <fltk/Group.h>
#include <fltk/Widget.h>
#include <fltk/events.h>



#include "ui.h"
extern UI* ui;

#include "backend.h"

#include "config.h"

#include "uihelper.h"

extern struct conf config;

using namespace fltk;

Timeline::Timeline(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {
  hand = fltk::SharedImage::get(ROOT_DATA_DIR"gfx/hand.gif");
  if(!hand){
    printf("Timeline: gfx files not found, installed correctly?\n");
  }
  scale = 1;
  label_scale = 1;
  zoom = 30;
  ticks_offset = 0;
  edit_flag = 0;

  px_last = 0;

  scroll=0;
}

int Timeline::handle(int event){

  int tick;

  switch(event){
    case fltk::FOCUS:
      return 1;
    case fltk::PUSH:
      take_focus();
      if(event_button() == 1){//set left limit
        //pixel -> tick -> quantize -> global tick
        tick = quantize(xpix2tick(event_x())*scale/4);
        //printf("%d %d %d\n",tick,loop_start,loop_end);
        if(tick < get_loop_end()){
          set_loop_start(tick);
        }
      }
      else if(event_button() == 2){//set song position
        tick = quantize(xpix2tick(event_x())*scale/4);
        //printf("%d %d %d\n",tick,loop_start,loop_end);
        //pointer_x = tick2xpix(tick);
        ui->song_timeline->update(tick);
        ui->pattern_timeline->update(tick);
        all_notes_off();
        reset_backend(tick);
        ui->song_timeline->redraw();
        ui->pattern_timeline->redraw();
      }
      else if(event_button() == 3){//set right limit
        tick = quantize(xpix2tick(event_x())*scale/4);
        //printf("%d %d %d\n",tick,loop_start,loop_end);
        if(tick > get_loop_start()){
          set_loop_end(tick);
        }
      }
      redraw();
      return 1;
  }
  return 0;
}

void Timeline::draw(){

  fltk::setfont(fltk::HELVETICA,12);
  fltk::setcolor(fltk::WHITE);
  fltk::fillrect(0,0,w(),h());

  fltk::push_clip(0,0,w(),h());
  fltk::setcolor(fltk::BLACK);
  fltk::drawline(0,h()-1,w()-1,h()-1);

  if(edit_flag){//draw pattern timeline

    int M = config.beats_per_measure;

    int I=0;
    for(int i=0; I<w(); i++){
      I = i*zoom - scroll;
      if(I>=0){
        fltk::fillrect(I,h()/2,1,h()/2);
      }
    }

    fltk::setcolor(fltk::BLACK);

    char buf[64];
    int j = 0;

    I=0;
    for(int i=0; I<w(); i++){
      I = i*zoom*4 - scroll;
      if(I>=0){
        fltk::fillrect(I,0,1,h()/2);
        sprintf(buf,"%u",j*label_scale);
        fltk::drawtext(buf,I+3,h()-3);
      }
      j++;
    }

  }
  else{//draw song timeline

    int M = config.beats_per_measure;
    int P = config.measures_per_phrase;
    int I=0;
    for(int i=0; I<w(); i++){
      I = i*zoom*M/4 - scroll;
      if(I>=0){
        fltk::fillrect(I,h()/2,1,h()/2);
      }
    }

    fltk::setcolor(fltk::BLACK);

    char buf[64];
    int j = 0;

    int p = P>0 ? P : 4;
    I=0;
    for(int i=0; I<w(); i++){
      I = i*zoom*4*p*M/4/4 - scroll;
      if(I>=0){
        fltk::fillrect(I,0,1,h()/2);
        sprintf(buf,"%u",j*label_scale*p/4);
        fltk::drawtext(buf,I+3,h()-3);
      }
      j++;
    }

  }

  int X = tick2xpix(get_loop_start()*4/scale);
  if(X >= -10){
    fltk::setcolor(fltk::color(0,0,255));
    fillrect(X+0,0,1,h()-1);
    fltk::setcolor(fltk::color(85,85,255));
    fillrect(X+1,0,1,h()-1);
    fltk::setcolor(fltk::color(170,170,255));
    fillrect(X+2,0,1,h()-1);
  }

  X = tick2xpix(get_loop_end()*4/scale);
  if(X >= -10){
    fltk::setcolor(fltk::color(128,0,0));
    fillrect(X+0,0,1,h()-1);
    fltk::setcolor(fltk::color(170,42,42));
    fillrect(X-1,0,1,h()-1);
    fltk::setcolor(fltk::color(255,170,170));
    fillrect(X-2,0,1,h()-1);
  }

  if(hand && pointer_x*4/scale-scroll-10 >= -20){
    hand->draw(pointer_x*4/scale-scroll-10,h()-19);
  }

  fltk::pop_clip();

}


void Timeline::update(int ticks){
  //printf("%d\n",ticks);
  int T = TICKS_PER_BEAT / 8; /* value was 16 when TICKS_PER_BEAT was 128 */
  pointer_x = (ticks/T - ticks_offset/T) * zoom / 8;
  if(pointer_x != px_last && ticks != get_loop_end()){
    px_last = pointer_x;
    redraw();
  }
}

int Timeline::tick2xpix(int tick){
  //return (tick - ticks_offset) * zoom / 128 - scroll;
  return tick * zoom / TICKS_PER_BEAT - scroll - ticks_offset*zoom/(32);
}

int Timeline::xpix2tick(int xpix){
  //return (xpix+scroll+ticks_offset*32/zoom)*128 / zoom;
  return (xpix+scroll) * TICKS_PER_BEAT / zoom + ticks_offset*120/30;
}

int Timeline::quantize(int tick){
  return tick/TICKS_PER_BEAT * TICKS_PER_BEAT;
}

