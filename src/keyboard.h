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

#ifndef keyboard_h
#define keyboard_h


class Keyboard : public fltk::Widget {

    int cur_note;
    int sustain;

    char ons[128];

    int octave;

  public:
    int cur_port;
    int cur_chan;
    int scroll;

    Keyboard(int x, int y, int w, int h, const char* label);

    void play_note(int note);
    void release_note(int note);
    void cut_notes();
    void set_sustain(int state);

    void kb_play_note(int note);
    void kb_release_note(int note);
    void octave_up();
    void octave_down();

    int handle(int event);
    void draw();

};


int keyboard_handler(int e, fltk::Window* w);

#endif
