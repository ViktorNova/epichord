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

#ifndef backend_h
#define backend_h

#include <inttypes.h>

#define TICKS_PER_BEAT 128

int init_backend(int* argc, char*** argv);
int shutdown_backend();
int start_backend();
int pause_backend();


void dispatch_event(mevent* e, int track, int tick_base);


int reset_backend(int tick);
void toggle_backend_recording();

int is_backend_playing();
int is_backend_recording();

void set_loop_start(int tick);
void set_loop_end(int tick);
int get_loop_start();
int get_loop_end();
void toggle_loop();

void set_solo(int s);
int get_solo();

void backend_set_passthru(int v);

void set_bpm(int new_bpm);

int get_play_position();

void send_midi(char* raw, uint16_t n, uint8_t p);
void send_midi_local(char* raw, uint16_t n);
int recv_midi(int* chan, int* tick, int* type, int* val1, int* val2);
const char* getsysexbuf();
void all_notes_off();

void program_change(int track, int prog);
void midi_bank_controller(int track, int bank);
void midi_volume_controller(int track, int vol);
void midi_pan_controller(int track, int pan);
void midi_expression_controller(int track, int expr);

void midi_note_off(int note, int chan, int port);
void midi_channel_off(int chan, int port);
void midi_track_off(int track);



enum {SESSION_NOMORE,SESSION_LOAD,SESSION_SAVE,SESSION_QUIT,SESSION_UNHANDLED};
int backend_session_process();
char* get_session_string();

#endif

