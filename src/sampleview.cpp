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



#include "ui.h"



extern UI* ui;


SampleView::SampleView(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {
  
}

int SampleView::handle(int event){
  switch(event){
  }
  return 0;
}

void SampleView::draw(){
  fltk::setcolor(fltk::GRAY40);

  fltk::fillrect(0,0,w(),h());
}

