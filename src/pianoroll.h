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

#ifndef pianoroll_h
#define pianoroll_h


class PianoRoll : public fltk::Widget {

  int xp_last;
  int yp_last;

  int wkeyh;
  int bkeyh;

  int last_note;

  int new_left_t;
  int new_right_t;
  int new_orig_t;
  int new_drag;
  int new_note;

  int move_t;
  int move_offset;
  int move_note;
  int move_flag;

  mevent* main_sel;

  int q_tick;

  int note2ypix(int note);



  int delete_flag;

  int zoom_n;
  int zoom;

  public:

    int tick2xpix(int time);
    int xpix2tick(int xpix);
    int quantize(int tick);

    seqpat* cur_seqpat;
    track* cur_track;


    PianoRoll(int x, int y, int w, int h, const char* label);
    int handle(int event);
    void draw();
    void layout();

    void load(seqpat* s);

    mevent* over_note();
    int over_handle(mevent* e);

    void set_zoom(int z);
    void set_qtick(int q){q_tick=q;}

    void update(int pos);

};

#endif
