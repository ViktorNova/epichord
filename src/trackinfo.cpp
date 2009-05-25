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

#include <vector>
#include "ui.h"



extern UI* ui;

extern std::vector<track*> tracks;

TrackInfo::TrackInfo(int x, int y, int w, int h, const char* label) : fltk::Group(x, y, w, h, label)
{
  //begin();

  //TrackModule* mod;
  //for(int i=0; i<16; i++){
    //mod = new TrackModule(0,30*i,255,30,i);
    //mod->box(fltk::UP_BOX);
    //mod->index = i;
    //mod->set_channel(i);
    //add(mod);
 // }

  //end();

  scroll = 0;

  settings = 0;

}

int TrackInfo::handle(int event){
  switch(event){
    case fltk::SHORTCUT:
      return 0;
  }
  return Group::handle(event);
}

void TrackInfo::draw(){
  fltk::push_clip(0,0,w(),h());
  for(int i=0; i<children(); i++){
    child(i)->y(i*30 - scroll);
  }
  Group::draw();
  fltk::pop_clip();
}

void TrackInfo::layout(){
  for(int i=0; i<children(); i++){
    child(i)->h(30);
  }
}


void TrackInfo::update(){
  //for each child cast to TrackModule and call update
  for(int i=0; i<children(); i++){
    ((TrackModule*)child(i))->update();
  }
}

void TrackInfo::unset_solo(){
  for(int i=0; i<children(); i++){
    ((TrackModule*)child(i))->unset_solo();
  }
}

void TrackInfo::set_rec(int t){
  for(int i=0; i<children(); i++){
    if(t==i){
      ((TrackModule*)child(i))->set_rec();
    }
    else{
      ((TrackModule*)child(i))->unset_rec();
    }
  }
}



void TrackInfo::clear_tracks(){
  clear();
}

void TrackInfo::add_track(){
  int i = tracks.size()-1;
  TrackModule* mod = new TrackModule(0,30*i,325,30,i);

  mod->box(fltk::UP_BOX);
  mod->index = i;
  mod->set_channel(i%16);

  add(mod);

}

void TrackInfo::del_track(int n){

}
