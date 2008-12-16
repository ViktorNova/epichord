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

#include <stdio.h>
#include <string.h>

#include <vector>

#include <pthread.h>
#include <jack/jack.h>
#include <jack/types.h>
#include <jack/ringbuffer.h>
#include <jack/midiport.h>
#include <jack/transport.h>

#include "seq.h"
#include "backend.h"


#include "ui.h"

#define PORT_COUNT 8

extern std::vector<track*> tracks;

jack_client_t* client;

jack_port_t* outport[PORT_COUNT];
jack_port_t* inport;
void* pbo[PORT_COUNT];
void* pbi;

jack_ringbuffer_t* outbuf;
jack_ringbuffer_t* inbuf;

static int playing = 0;
static int looping = 0;
static int recording=0;
static int loop_start = 0;
static int loop_end = 512;
static int pass_through = 1;
static int rec_port = 0;

static int init_chans = 1;

static uint64_t cur_frame = 0;
static uint64_t last_frame = 0;
static int frame_jump = 0;
static int cur_tick;
static int last_tick;
static int bpm = 120;
static int new_bpm = 120;
static int tpb = 128;
static int sample_rate = 0;
static int frame_count = 0;

#define t2f(X) ((uint64_t)X*60*sample_rate/(bpm*tpb))
#define f2t(X) ((uint64_t)X*bpm*tpb/(sample_rate*60))

//void (*update_display)() = NULL;

/* callback for seq.c and play.c */
/* THIS BARELY WORKS */
void dispatch_event(mevent* e, int track, int tick_base){

  jack_midi_data_t* md;

  int p = tracks[track]->port;
  int c = tracks[track]->chan;

  uint64_t base = t2f(tick_base);
  uint64_t frame = t2f(e->tick) + base - last_frame + frame_jump;

  if(e->type == -1){
    return;
  }

  if(looping && e->tick+tick_base == loop_end && e->type != MIDI_NOTE_OFF){
    //dispatch only note off events on a loop_end
    return;
  }

//printf("0x%x %d %llu %llu %llu %d %llu %d\n", e->type, e->tick, t2f(e->tick), base, last_frame, frame_jump, frame, bpm);

if(t2f(e->tick)+base < last_frame){
  printf("playing a note in the past?? lucky it didnt segfault!\n");
  return;
}

  if(frame == frame_count){
    frame--;
  }

  unsigned char buf[3];
  size_t n;
  if(midi_encode(e,c,buf,&n) < 0){
    return;
  }

  md = jack_midi_event_reserve(pbo[p],frame,n);
  if(md == NULL){
    md = jack_midi_event_reserve(pbo[p],frame_count-1,n);
    if(md == NULL){
      printf("dispatch: can't reserve midi event\n");
      return;
    }
  }
  for(int i=0; i<n; i++){
    md[i] = buf[i];
  }

}



/* jack callbacks */

static int H = 1;
static int process(jack_nframes_t nframes, void* arg){

  frame_count = nframes;

  

  //handle incoming midi events
  for(int i=0; i<PORT_COUNT; i++){
    pbo[i] = jack_port_get_buffer(outport[i],nframes);
    jack_midi_clear_buffer(pbo[i]);
  }
  pbi = jack_port_get_buffer(inport,nframes);

  jack_midi_data_t* md1;
  jack_midi_data_t* md2;

  jack_midi_event_t me;

  uint32_t rec_tick;

  for(int i=0; i<jack_midi_get_event_count(pbi); i++){
    jack_midi_event_get(&me,pbi,i);
    md1 = me.buffer;
    //printf("%d got midi event, type 0x%x, value 0x%x 0x%x\n",i,md1[0],md1[1],md1[2]);
    if(pass_through){
      md2 = jack_midi_event_reserve(pbo[rec_port],0,me.size);
      if(md2 == NULL){
        printf("passthru: can't reserve midi event\n");
      }
      else{
        memcpy(md2,md1,me.size);
      }
    }
    rec_tick = last_tick + (me.time*bpm*tpb)/(sample_rate*60);

    if(playing && recording){
      uint16_t size = me.size;
      jack_ringbuffer_write(inbuf,(char*)&rec_tick,4);
      jack_ringbuffer_write(inbuf,(char*)&size,2);
      jack_ringbuffer_write(inbuf,(char*)md1,me.size);
    }
  }

  //init chans
  if(init_chans && playing){
    init_chans=0;
    char buf[3];
    for(int j=0; j<tracks.size(); j++){
      int ch = tracks[j]->chan;
      int pt = tracks[j]->port;

      //bank select
      buf[0] = 0xB0 | ch;
      buf[1] = 0;
      buf[2] = tracks[j]->bank;
      send_midi(buf,3,pt);

      //program change
      buf[0] = 0xC0 | ch;
      buf[1] = tracks[j]->prog;
      send_midi(buf,2,pt);

      //channel volume
      buf[0] = 0xB0 | ch;
      buf[1] = 7;
      buf[2] = tracks[j]->vol;
      send_midi(buf,3,pt);

      //channel pan
      buf[0] = 0xB0 | ch;
      buf[1] = 10;
      buf[2] = tracks[j]->pan;
      send_midi(buf,3,pt);

    }
  }

  //handle outgoing immediate midi events
  uint16_t n;
  char p;
  char buf[1024];
  while(jack_ringbuffer_read_space(outbuf) > 0){
    jack_ringbuffer_read(outbuf,&p,1);
    jack_ringbuffer_read(outbuf,(char*)&n,2);
    jack_ringbuffer_read(outbuf,buf,n);
    md1 = jack_midi_event_reserve(pbo[p],0,n);
    if(md1 == NULL){
      printf("gui: can't reserve midi event\n");
      break;
    }
    for(int i=0; i<n; i++){
      md1[i]=buf[i];
    }
  }


  //play
  if(playing){
    if(bpm != new_bpm){
      bpm = new_bpm;
      cur_frame = t2f(cur_tick);
      last_frame = t2f(last_tick);
    }

    last_frame = cur_frame;
    cur_frame += nframes;
    last_tick = cur_tick;
    cur_tick = f2t(cur_frame);
    frame_jump = 0;
    int le = loop_end;
    int ls = loop_start;
    int lf = looping;


    /* this only handles the case where the nframes covers the
       loop boundary once. a more robust way would split it into
       before covering n loop boundaries, a loop for n looping regions,
       and a final section covering part of the loop region. this
       would only need to be used for insanely high bpm */
    if(lf && last_tick < le && cur_tick > le ){
      //printf("split l%d m%d r%d\n",last_tick,loop_end,cur_tick);
      int split_frame = t2f(le) - last_frame + 1;
      int left_over = nframes - split_frame;
      int jump = split_frame;
      cur_frame = last_frame + jump;
      cur_tick = f2t(cur_frame);
      //printf("cur tick %d\n",cur_tick);
      play_seq(cur_tick, dispatch_event);

      reset_backend(ls);
      //printf("cur tick %d\n",cur_tick);
      frame_jump = jump;
      last_frame = t2f(ls);
      cur_frame = last_frame + left_over;
      cur_tick = f2t(cur_frame);

      play_seq(cur_tick, dispatch_event);
    }
    else if(lf && cur_tick > le){
      reset_backend(ls);
      last_frame = t2f(ls);
      cur_frame = last_frame + nframes;
      cur_tick = f2t(cur_frame);
      //printf("%llu %llu %d %d\n",last_frame,cur_frame,last_tick,cur_tick);
      play_seq(cur_tick, dispatch_event);
    }
    else{
      //printf("%llu %llu %d %d\n",last_frame,cur_frame,last_tick,cur_tick);
      play_seq(cur_tick, dispatch_event);
    }
  }

  return 0;
}





/* backend api wrapper */

int init_backend(){

  client = jack_client_open("Epichord",(jack_options_t)0,NULL);
  if(client == NULL){
    printf("failure to open jack client\n");
    return -1;
  }

  jack_set_process_callback(client, process, 0);

  char buf[64];
  for(int i=0; i<PORT_COUNT; i++){
    sprintf(buf,"midi out %d",i);
    outport[i]=jack_port_register(client,buf,JACK_DEFAULT_MIDI_TYPE,JackPortIsOutput,0);
  }

  inport=jack_port_register(client, "midi in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

  outbuf = jack_ringbuffer_create(1024);
  inbuf = jack_ringbuffer_create(1024);

  sample_rate = jack_get_sample_rate(client);

  jack_activate(client);

  return 0;
}




int shutdown_backend(){
  pause_backend();
  all_notes_off();
  sleep(1);
  jack_deactivate(client);
  return 0;
}


int start_backend(){
  playing = 1;
}

int pause_backend(){
  playing = 0;
}

int reset_backend(int tick){
  if(tick==0){
    init_chans = 1;
  }
  cur_frame = t2f(tick);
  last_frame = cur_frame;
  last_tick = tick;
  cur_tick = tick;
  set_seq_pos(tick);
}


void toggle_backend_recording(){
  if(recording){
    recording = 0;
  }
  else{
    recording = 1;
  }
}


//send a midi event from the gui thread
void send_midi(char* raw, uint16_t n, uint8_t port){
  if(n>1024){
    printf("send_midi: cant send, immediate message too big\n");
    return;
  }
  char buf[2048];
  buf[0] = port;
  memcpy(buf+1,&n,2);
  memcpy(buf+3,raw,n);
  jack_ringbuffer_write(outbuf,buf,3+n);
}

//get next incoming midi event for gui thread
int recv_midi(int* chan, int* tick, int* type, int* val1, int* val2){
  uint16_t n;
  uint32_t t;
  char* buf;
  if(jack_ringbuffer_read(inbuf,(char*)&t,4) == 0){
    return 0;
  }
  jack_ringbuffer_read(inbuf,(char*)&n,2);
  buf = (char*)malloc(n);
  jack_ringbuffer_read(inbuf,buf,n);

  *tick = t;
  *chan = buf[0]&0x0f;
  *type = buf[0]&0xf0;
  *val1 = buf[1]; 
  *val2 = buf[2];

  free(buf);
  return 1;
}

void all_notes_off(){
  for(int i=0; i<tracks.size(); i++){
    midi_track_off(i);
  }
}

void program_change(int track, int prog){
  int port = tracks[track]->port;
  int chan = tracks[track]->chan;

  char buf[2];
  buf[0] = 0xC0 | chan;
  buf[1] = prog;
  send_midi(buf, 2, port);
}

void midi_bank_controller(int track, int bank){
  int port = tracks[track]->port;
  int chan = tracks[track]->chan;

  char buf[3];
  buf[0] = 0xB0 | chan;
  buf[1] = 0;
  buf[2] = bank;
  send_midi(buf, 3, port);
}

void midi_volume_controller(int track, int vol){
  int port = tracks[track]->port;
  int chan = tracks[track]->chan;

  char buf[3];
  buf[0] = 0xB0 | chan;
  buf[1] = 7;
  buf[2] = vol;
  send_midi(buf, 3, port);
}

void midi_pan_controller(int track, int pan){
  int port = tracks[track]->port;
  int chan = tracks[track]->chan;

  char buf[3];
  buf[0] = 0xB0 | chan;
  buf[1] = 10;
  buf[2] = pan;
  send_midi(buf, 3, port);
}

void midi_expression_controller(int track, int expr){
  int port = tracks[track]->port;
  int chan = tracks[track]->chan;

  char buf[3];
  buf[0] = 0xB0 | chan;
  buf[1] = 11;
  buf[2] = expr;
  send_midi(buf, 3, port);
}


void midi_note_off(int note, int chan, int port){
  char buf[3];
  buf[0] = 0x80 | chan;
  buf[1] = note;
  buf[2] = 0x00;
  send_midi(buf,3,port);
}

void midi_track_off(int track){
  int chan = tracks[track]->chan;
  int port = tracks[track]->port;

  midi_channel_off(chan,port);
}

void midi_channel_off(int chan, int port){
  char buf[3] = {0xB0,123,0};
  buf[0] = 0xB0 | chan;
  send_midi(buf,3,port);

  buf[0] = 0xB0 | chan;
  buf[1] = 120;
  send_midi(buf,3,port);
}

void set_loop_start(int tick){
  loop_start = tick;
}

void set_loop_end(int tick){
  loop_end = tick;
}

int get_loop_start(){
  return loop_start;
}

int get_loop_end(){
  return loop_end;
}

int get_play_position(){
  return cur_tick;
}

int solo;
void set_solo(int s){
  solo = s;
}

int get_solo(){
  return solo;
}

int is_backend_playing(){
  return playing;
}

int is_backend_recording(){
  return recording;
}

void toggle_loop(){
  if(looping){
    looping = 0;
  }
  else{
    looping = 1;
  }
}


void set_bpm(int n){
  new_bpm = n;
}
