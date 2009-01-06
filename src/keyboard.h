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

#include <fstream>

class Keyboard : public fltk::Widget {

    int cur_note;
    int sustain;

    char ons[128];
    char helds[128];

    int octave;

    int highlight;

  public:
    int cur_port;
    int cur_chan;
    int scroll;

    Keyboard(int x, int y, int w, int h, const char* label);

    void play_note(int note, int rec);
    void release_note(int note, int rec);
    void cut_notes(int rec);
    void set_sustain(int state);

    void kb_play_note(int note);
    void kb_release_note(int note);
    void octave_up();
    void octave_down();

    int handle(int event);
    void draw();

    void highlight_note(int note);
    void highlight_clear();
};


int keyboard_handler(int e, fltk::Window* w);


struct combo {
  unsigned int key;
  unsigned int mod;
  combo(unsigned zkey, unsigned zmod){key=zkey; mod=zmod;}
  combo(){key=' '; mod=0;}
  int operator==(combo c){
    return (c.key==key && c.mod==mod) ? 1 : 0;
  }
};

class KeyGrabber : public fltk::Widget {

    char str[32];
    int in_flag;

  public:

    unsigned key;
    unsigned mod;
    KeyGrabber(int x, int y, int w, int h, const char* label);

    int handle(int event);
    void draw();

    int set_key(int key, int mod);
    int set_key(combo c);
    int cmp(combo c);

    void save(std::fstream& f);
    void load(std::fstream& f);
};

void set_keymap(int which, int index, int key, int mod);
char* get_keystring(int key, int mod);

void load_keymap(std::fstream& f);
void save_keymap(std::fstream& f);
void load_default_keymap();

int zoom_out_key(unsigned,unsigned);
int zoom_in_key(unsigned,unsigned);

#endif
