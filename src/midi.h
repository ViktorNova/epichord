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

/* midi junk

notes on timing
the sequencer will use some number of ticks per beat which
corresponds to its minimum allowed note length.

a beat is divided into four parts at the regular zoom.
in 4/4 time this corresponds to 1/16 notes.
lets make the shortest note 1/128 note.
this means there are 128 'ticks' per beat.

to convert from time in us into ticks do

ticks = time * BPM * TPB / (1000000 * 60);


*/

#ifndef midi_h
#define midi_h

#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80

//encodes data in e as a midi event placed in buf
int midi_encode(mevent* e, int chan, char* buf, size_t* n);

//decodes midi data and creates a new mevent
int midi_decode(char* buf, mevent* e);

#endif
