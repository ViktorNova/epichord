#ifndef uihelper_h
#define uihelper_h

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

struct conf{
  int beats_per_measure;
  int measures_per_phrase;
  int measures_until_record;
  int alwayscopy;
  int autotrackname;
  int passthru;
  int playinsert;
  int recordonchan;
  int playmove;
  int follow;
  int quantizedur;
  int recordmode;
  int robmode;
};

void config_init();
void start_monitor();

void press_stop();
void press_panic();
void press_play();

void set_quant(int q);

void scope_print(char* text);


void set_beats_per_measure(int n);
void set_measures_per_phrase(int n);
void set_mur(int n);

void set_alwayscopy(int v);
void set_autotrackname(int v);
void set_passthru(int v);
void set_playinsert(int v);
void set_recordonchan(int v);
void set_playmove(int v);
void set_follow(int v);
void set_quantizedur(int v);
void set_recordmode(int n);
void set_robmode(int n);


#endif
