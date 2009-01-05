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



  int wkeyh;
  int bkeyh;

  int last_note;

  int insert_flag;
  int insert_torig;
  int insert_toffset;
  int insert_note;

  int move_flag;
  int move_torig;
  int move_toffset;
  int move_norig;
  int move_noffset;
  int move_offset;
  int move_qoffset;

  int box_flag;
  int box_x1;
  int box_y1;
  int box_x2;
  int box_y2;
  int box_t1;
  int box_t2;
  int box_n1;
  int box_n2;

  mevent* last_handle;
  int rresize_flag;
  int rresize_torig;
  int rresize_toffset;

  int lresize_flag;
  int lresize_torig;
  int lresize_toffset;

  mevent* resize_e;
  int resize_arrow;
  int resize_redraw;
  int resize_x;
  int resize_y;
  fltk::Color resize_arrow_color;
  int resize_handle_width;

  int q_tick;

  int note2ypix(int note);



  int delete_flag;

  int zoom_n;
  int zoom;

  void get_event_color(mevent* e, fltk::Color* c1, fltk::Color* c2, fltk::Color* c3);

  void unselect_all();

  void apply_box();
  void apply_delete();
  void apply_move();
  void apply_paste();
  void apply_rresize();
  void apply_lresize();
  void apply_insert();

  int over_lhandle(mevent* e);
  int over_rhandle(mevent* e);

  public:

    int tick2xpix(int time);
    int xpix2tick(int xpix);
    int quantize(int tick);

    seqpat* cur_seqpat;
    track* cur_track;


    PianoRoll(int x, int y, int w, int h, const char* label);
    int handle(int event);
    void draw();

    void load(seqpat* s);

    mevent* over_note();


    void set_zoom(int z);
    void set_qtick(int q){q_tick=q;}

    void update(int pos);

    void layout();

    int fakeh;
    int scrollx;
    int scrolly;

    int fakehmin;

    void scrollTo(int X, int Y);
};

#endif
