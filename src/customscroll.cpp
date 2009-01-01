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
#include <fltk/ScrollGroup.h>
#include <fltk/Widget.h>

#include "customscroll.h"

#include "ui.h"



extern UI* ui;


CustomScroll::CustomScroll(int x, int y, int w, int h, const char* label) : fltk::ScrollGroup(x, y, w, h, label) {

}

int CustomScroll::handle(int event){
  switch(event){
    case fltk::MOUSEWHEEL:
      Widget* w = child(0);
      if(w){
        w->handle(event);
      }
      return 1;
      break;
  }

  return fltk::ScrollGroup::handle(event);
}


