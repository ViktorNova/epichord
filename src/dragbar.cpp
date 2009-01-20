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

#include <fltk/Widget.h>
#include <fltk/Group.h>
#include <fltk/draw.h>
#include <fltk/events.h>

#include "dragbar.h"


DragBar::DragBar(int x, int y, int w, int h, const char* label) : 
fltk::Widget(x, y, w, h, label) {
  
  cached_flag;

}

int DragBar::handle(int event){
  int X = fltk::event_x();
  switch(event){
    case fltk::PUSH:
      orig_x = X;
      return 1;
    case fltk::DRAG:
      val = last_val - (X - orig_x);
      if(val < 0){val=0;}
      if(val != last_val){
        redraw();
        do_callback();
      }
      return 1;
    case fltk::RELEASE:
      last_val = val;
  }
  return 0;
}

void DragBar::draw(){
  draw_box();

  fltk::push_clip(2,2,w()-4,h()-4);

  fltk::setcolor(fltk::GRAY60);
  fltk::fillrect(2,2,w()-4,h()-4);

  if(!cached_flag){//draw first time
    for(int i=-val; i<w(); i+=6){
      if(i+10 > 0){
        fltk::setcolor(fltk::GRAY80);
        fltk::fillrect(i+5,4,3,1);
        fltk::fillrect(i+5,4,1,2);

        fltk::setcolor(fltk::GRAY30);
        fltk::fillrect(i+5,6,3,1);
        fltk::fillrect(i+7,5,1,2);

        fltk::setcolor(fltk::GRAY80);
        fltk::fillrect(i+5,4+5,3,1);
        fltk::fillrect(i+5,4+5,1,2);

        fltk::setcolor(fltk::GRAY30);
        fltk::fillrect(i+5,6+5,3,1);
        fltk::fillrect(i+7,5+5,1,2);
      }
    }

    int GX=x();
    int GY=y();
    fltk::Widget* p = parent();
    while(p){
      if(p->parent() != NULL){//the window
        GX+=p->x();
        GY+=p->y();
      }
      p = p->parent();
    }
    readimage(gfxbuf,fltk::RGB,fltk::Rectangle(GX+2,GY+2,102,h()-4));
    cached_flag=1;
  }
  else{
    for(int i=-val; i<w(); i+=102){
      if(i+200 > 0){
        drawimage(gfxbuf,fltk::RGB,fltk::Rectangle(2+i,2,102,h()-4));
      }
    }
  }

  fltk::pop_clip();
}

