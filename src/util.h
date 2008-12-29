#ifndef util_h
#define util_h

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



void load_text(fltk::TextDisplay* o, const char* filename);
void hsv_to_rgb(float h, float s, float v, unsigned char* r, unsigned char* g, unsigned char* b);

int ypix2note(int ypix, int black);
int note2ypix(int note, int* black);


int seqpat_nonstick(seqpat* s);
int unmodify_and_unstick_tracks();
#endif
