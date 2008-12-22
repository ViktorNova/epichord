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

#ifndef eventedit_h
#define eventedit_h

class EventEdit : public fltk::Widget {

    int event_type;
    int controller_type;

    int tick2xpix(int tick);

    char* event_type_name();
    void event_type_next();
    void event_type_prev();
    void set_event_type(int type, int controller);

    int q_tick;

    int line_flag;
    int line_x;
    int line_y;
    int line_orig_x;
    int line_orig_y;

    int ypix2mag(int ypix);
    int mag2ypix(int mag);
    int mag2val(int mag);
    int val2mag(int val);

    void apply_line(int t1, int t2, int v1, int v2);
    int match_event_type(mevent* e);

  public:

    int label_flag;

    seqpat* cur_seqpat;
    track* cur_track;

    int zoom;

    int scroll;

    EventEdit(int x, int y, int w, int h, const char* label);
    int handle(int event);
    void draw();

    void load(seqpat* s);
    void set_qtick(int q){q_tick=q;}

};

#endif
