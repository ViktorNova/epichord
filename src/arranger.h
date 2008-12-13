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

#ifndef arranger_h
#define arranger_h

class Arranger : public fltk::Widget {

    int xp_last;
    int yp_last;

    int new_orig_t;
    int new_left_t;
    int new_right_t;
    int new_drag;
    int new_track;
    int new_default_w;

    int move_offset;
    int move_t;
    int move_track;
    int move_flag;
    int move_start;

    int paste_t;
    int paste_track;
    int paste_flag;

    seqpat* main_sel;
    seqpat* delete_sel;
    int sel_timer;

    seqpat* over_seqpat();
    int over_handle(seqpat* s);

    int tick2xpix(int tick);
    int xpix2tick(int xpix);
    int quantize(int xpix);

    int delete_flag;

  public:

    int zoom;
    int zoom_n;

    int q_tick;

    Arranger(int x, int y, int w, int h, const char* label);
    int handle(int event);
    void draw();

    void layout();

};

#endif
