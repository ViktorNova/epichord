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

    int maxt;

    int scrollbuffer;

    int insert_flag;
    int insert_torig;
    int insert_toffset;
    int insert_track;
    int new_default_w;

    int box_flag;
    int box_x1;
    int box_y1;
    int box_x2;
    int box_y2;
    int box_t1;
    int box_t2;
    int box_k1;
    int box_k2;

    int move_flag;
    int move_korig;
    int move_torig;
    int move_koffset;
    int move_toffset;
    int move_x;
    int move_y;
    int move_w;
    int move_offset;

    int rresize_flag;
    int rresize_torig;
    int rresize_toffset;

    int lresize_flag;
    int lresize_torig;
    int lresize_toffset;

    int resize_arrow;
    int resize_redraw;
    int resize_x;
    int resize_y;
    fltk::Color resize_arrow_color;
    seqpat* resize_s;
    int resize_handle_width;

    int paste_flag;
    int paste_tcenter0;
    int paste_kcenter0;
    int paste_tcenter1;
    int paste_kcenter1;


    seqpat* main_sel;
    seqpat* delete_sel;
    int sel_timer;

    seqpat* over_seqpat();
    int over_rhandle(seqpat* s);
    int over_lhandle(seqpat* s);

    int tick2xpix(int tick);
    int xpix2tick(int xpix);
    int quantize(int xpix);

    int delete_flag;

    float color_h;
    float color_v;
    pattern* color_sel;
    int color_orig_x;
    int color_orig_y;
    float color_orig_h;
    float color_orig_v;

    void get_outline_color(seqpat* s, fltk::Color* c1, fltk::Color* c2, fltk::Color* c3, fltk::Color* cx);


    seqpat* split_s;
    int split_t;

    seqpat* join_s;

    void unselect_all();

    void apply_box();
    void apply_delete();
    void apply_move();
    void apply_paste();
    void apply_rresize();
    void apply_lresize();
    void apply_insert();

    void apply_unclone();
    void apply_split();
    void apply_join();

    int check_move_safety();
    int check_insert_safety();
    int check_resize_safety();
    int check_paste_safety();

    int check_join_safety();

    void recalc_paste_center();


  public:

    int fakeh;
    int scrollx;
    int scrolly;

    int zoom;
    int zoom_n;

    int color_flag;
    int unclone_flag;
    int join_flag;
    int split_flag;

    int q_tick;

    Arranger(int x, int y, int w, int h, const char* label);
    int handle(int event);
    void draw();

    void update(int pos);

    void layout();

    void scrollTo(int X, int Y);

};

#endif
