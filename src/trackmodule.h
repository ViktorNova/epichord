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

#ifndef trackmodule_h
#define trackmodule_h


#include <fltk/Input.h>
#include <fltk/Button.h>
#include <fltk/ValueInput.h>
#include <fltk/events.h>

class Gauge : public fltk::Widget {

  public:

  Gauge(int x, int y, int w, int h, const char* label=0);

  int value;
  int max;
  int step;
  int scale;
  int r,g,b;
  int R,G,B;

  int label_flag;
};

class VGauge : public Gauge {

  public:

  VGauge(int x, int y, int w, int h, const char* label=0);

  int handle(int e);
  void draw();

};

class HGauge : public Gauge {

  public:

  HGauge(int x, int y, int w, int h, const char* label=0);

  int handle(int e);
  void draw();

};

class Toggle : public fltk::Button {
  public:

  int key_flag;

  char c[4];
  int r;
  int g;
  int b;
  int R;
  int G;
  int B;

  int state;

  void set(int s);

  Toggle(int x, int y, int w, int h, const char* label=0);

  int handle(int e);
  void draw();
};


class TrackModule : public fltk::Group {

  int settings_shown;

  public:

  int index;
  VGauge volume;
  HGauge pan;
  Toggle solo;
  Toggle mute;
  fltk::Input name;
  VGauge chan;
  VGauge prog;
  VGauge port;
  VGauge bank;
  Toggle rec;

  void toggle();

  TrackModule(int x, int y, int w, int h, int i, const char* label=0);
  int handle(int event);

  void set_channel(int i);
  void unset_rec();
  void set_rec();
  void unset_solo();

  void update();

};

#endif
