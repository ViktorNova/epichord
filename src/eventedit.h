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

#include <vector>

class EventEdit : public fltk::Widget {

    int event_type;
    int controller_type;

    int tick2xpix(int tick);

    char* event_type_name();
    void event_type_next();
    void event_type_prev();
    void set_event_type(int type, int controller);

    int q_tick;

    std::vector<mevent*> selection;

    int line_flag;
    int line_t1;
    int line_M1;
    int line_t2;
    int line_M2;
    int line_x1;
    int line_x2;
    int line_y1;
    int line_y2;

    int box_flag;
    int box_x1;
    int box_y1;
    int box_x2;
    int box_y2;
    int box_t1;
    int box_t2;

    int insert_flag;
    int insert_x;
    int insert_y;
    int insert_t;
    int insert_M;

    int paste_flag;
    int paste_x;
    int paste_t;

    int delete_flag;
    int delete_t1;
    int delete_t2;
    int delete_x1;
    int delete_x2;

    int xpix2tick(int xpix);
    int ypix2mag(int ypix);
    int mag2ypix(int mag);
    int mag2val(int mag);
    int val2mag(int val);

    void apply_line();
    void apply_box();
    void apply_insert();
    void apply_delete();
    void apply_paste();
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

    void clear_events();
    void clear_all_events();

};

#endif
