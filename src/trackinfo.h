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

#ifndef trackinfo_h
#define trackinfo_h

class TrackInfo : public fltk::Group {

  public:

    int scroll;

    TrackInfo(int x, int y, int w, int h, const char* label);
    int handle(int event);
    void draw();
    void layout();

    void toggle_controls();
    void unset_solo();
    void set_rec(int t);

    void update();
  
};

#endif
