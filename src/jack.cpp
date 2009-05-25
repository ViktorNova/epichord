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
#include <string>

#include <pthread.h>
#include <jack/jack.h>
#include <jack/types.h>
#include <jack/ringbuffer.h>
#include <jack/midiport.h>
#include <jack/transport.h>

#define HAVE_LASH

#ifdef HAVE_LASH
#include <lash/lash.h>
lash_client_t* lash_client;
#endif

#include "seq.h"
#include "backend.h"

//#include "uihelper.h"

//#include "ui.h"

#define PORT_COUNT 8

//#define HAVE_LASH

extern std::vector<track*> tracks;

jack_client_t* client;

jack_port_t* outport[PORT_COUNT];
jack_port_t* inport;
void* pbo[PORT_COUNT];
void* pbi;

jack_ringbuffer_t* outbuf;
jack_ringbuffer_t* inbuf;
jack_ringbuffer_t* selfbuf;

static int playing = 0;
static int looping = 0;
static int recording=0;
static int loop_start = 0;
static int loop_end = TICKS_PER_BEAT*4;
static int passthru = 1;
static int rec_port = 0;
static int trackinit = 1;

static int init_chans = 1;

static uint64_t cur_frame = 0;
static uint64_t last_frame = 0;
static int frame_jump = 0;
static int cur_tick;
static int last_tick;
static int bpm = 120;
static int new_bpm = 120;
static int tpb = TICKS_PER_BEAT;
static int sample_rate = 0;
static int frame_count = 0;

#define t2f(X) ((uint64_t)X*60*sample_rate/(bpm*tpb))
#define f2t(X) ((uint64_t)X*bpm*tpb/(sample_rate*60))






/* callback for seq.c and play.c */
void dispatch_event(mevent* e, int track, int tick_base){

  jack_midi_data_t* md;

  int p = tracks[track]->port;
  int c = tracks[track]->chan;

  //uint64_t base = t2f(tick_base);
  //uint64_t frame = t2f(e->tick) + base - last_frame + frame_jump;
  uint64_t eframe = t2f((uint64_t)(e->tick+tick_base));
  uint64_t frame = eframe - last_frame + frame_jump;

  if(e->type == -1){//dummy events
    return;
  }

  if(looping && e->tick+tick_base == loop_end && e->type != MIDI_NOTE_OFF){
    //dispatch only note off events on a loop_end
    return;
  }

//printf("%d %d %d %llu %llu %llu\n",e->tick,tick_base,e->tick+tick_base, eframe, last_frame, frame);

  if(eframe < last_frame){
    printf("dispatch: %llu %llu negative frame index. BOOM segfault.\n", t2f(e->tick+tick_base), last_frame);
    return;
  }

  if(frame == frame_count){
    //printf("dispatch: scheduling bug. frame index == frame count.\n");
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
      printf("dispatch: can't reserve midi event.\n");
      return;
    }
    printf("dispatch: send midi using scheduling kludge.\n");
  }
  for(int i=0; i<n; i++){
    md[i] = buf[i];
  }

}



/* jack callbacks */



static int H = 1;
static int process(jack_nframes_t nframes, void* arg){

  jack_midi_data_t* md1;
  jack_midi_data_t* md2;

  jack_midi_event_t me;

  frame_count = nframes;

  for(int i=0; i<PORT_COUNT; i++){
    pbo[i] = jack_port_get_buffer(outport[i],nframes);
    jack_midi_clear_buffer(pbo[i]);
  }
  pbi = jack_port_get_buffer(inport,nframes);


  //handle incoming midi events
  uint32_t rec_tick;

  for(int i=0; i<jack_midi_get_event_count(pbi); i++){
    jack_midi_event_get(&me,pbi,i);
    md1 = me.buffer;
    //printf("%d got midi event, type 0x%x, value 0x%x 0x%x\n",i,md1[0],md1[1],md1[2]);
    if(passthru){
      md2 = jack_midi_event_reserve(pbo[rec_port],0,me.size);
      if(md2 == NULL){
        printf("passthru: can't reserve midi event\n");
      }
      else{
        memcpy(md2,md1,me.size);
      }
    }
    rec_tick = last_tick + (me.time*bpm*tpb)/(sample_rate*60);

    //if(playing && recording){
      uint16_t size = me.size;
      jack_ringbuffer_write(inbuf,(char*)&rec_tick,4);
      jack_ringbuffer_write(inbuf,(char*)&size,2);
      jack_ringbuffer_write(inbuf,(char*)md1,me.size);
   // }
  }
  
  //init chans
  if(init_chans && trackinit && playing){
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
      if(new_bpm != 0){
        bpm = new_bpm;
      }
      else{
        printf("process: someone set the bpm to zero!\n");
        bpm = 120;
      }
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
      play_seq(cur_tick);

      reset_backend(ls);
      //printf("cur tick %d\n",cur_tick);
      frame_jump = jump;
      last_frame = t2f(ls);
      cur_frame = last_frame + left_over;
      cur_tick = f2t(cur_frame);

      play_seq(cur_tick);
    }
    else if(lf && cur_tick > le){
      reset_backend(ls);
      last_frame = t2f(ls);
      cur_frame = last_frame + nframes;
      cur_tick = f2t(cur_frame);
      //printf("%llu %llu %d %d\n",last_frame,cur_frame,last_tick,cur_tick);
      play_seq(cur_tick);
    }
    else{
      //printf("%llu %llu %d %d\n",last_frame,cur_frame,last_tick,cur_tick);
      play_seq(cur_tick);
    }
  }


  


  return 0;
}





/* backend api wrapper */

int init_backend(int* argc, char*** argv){

  client = jack_client_open("Epichord",(jack_options_t)0,NULL);
  if(client == NULL){
    printf("init_backend: failure to open jack client\n");
    return -1;
  }

  jack_set_process_callback(client, process, NULL);

  char buf[64];
  for(int i=0; i<PORT_COUNT; i++){
    sprintf(buf,"midi out %d",i);
    outport[i]=jack_port_register(client,buf,JACK_DEFAULT_MIDI_TYPE,JackPortIsOutput,0);
  }

  inport=jack_port_register(client, "midi in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

  outbuf = jack_ringbuffer_create(1024);
  inbuf = jack_ringbuffer_create(1024);
  selfbuf = jack_ringbuffer_create(1024);

  sample_rate = jack_get_sample_rate(client);

  jack_activate(client);

#ifdef HAVE_LASH

  lash_client = lash_init(lash_extract_args(argc, argv), "Epichord",
                          LASH_Config_File, LASH_PROTOCOL( 2, 0 ));
  if(!lash_client){
    printf("init_backend: lash failed to initialize\n");
  }
  else{
    lash_jack_client_name(lash_client, "Epichord");
  }

#endif

  return 0;
}




int shutdown_backend(){
  pause_backend();
  all_notes_off();
  sleep(1);
  jack_deactivate(client);
  //sleep(1);
  return 0;
}


int start_backend(){
  playing = 1;
  jack_transport_start(client);
}

int pause_backend(){
  playing = 0;
  jack_transport_stop(client);
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

  jack_transport_locate(client, t2f(tick));
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
//FIXME check return value
  jack_ringbuffer_write(outbuf,buf,3+n);
}

void send_midi_local(char* raw, uint16_t n){
  uint32_t tick = cur_tick;
  uint16_t size = n;
//FIXME chec return value
  jack_ringbuffer_write(inbuf,(char*)&tick,4);
  jack_ringbuffer_write(inbuf,(char*)&size,2);
  jack_ringbuffer_write(inbuf,raw,n);
}



std::string sysexbuf;
//get next incoming midi event for gui thread
int recv_midi(int* chan, int* tick, int* type, int* val1, int* val2){
  uint16_t n;
  uint32_t t;
  unsigned char* buf;
  if(jack_ringbuffer_read(inbuf,(char*)&t,4) == 0){
    return 0;
  }
  jack_ringbuffer_read(inbuf,(char*)&n,2);
  buf = (unsigned char*)malloc(n);
  jack_ringbuffer_read(inbuf,(char*)buf,n);

  *tick = t;
  *chan = buf[0]&0x0f;
  *type = buf[0]&0xf0;
  *val1 = buf[1];
  *val2 = buf[2];

  char hbuf[8];

  if (buf[0]==0xf0){
    sysexbuf = "";
    for(int i=2; i<n-1; i++){
      snprintf(hbuf,8,"%02x ",buf[i]);
      sysexbuf.append(hbuf);
    }
  }

  free(buf);
  return 1;
}

const char* getsysexbuf(){
  return sysexbuf.c_str();
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


void backend_set_trackinit(int v){
  trackinit = v;
}
void backend_set_passthru(int v){
  passthru = v;
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
  return recording&&playing;
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

void set_rec_port(int n){
  rec_port = n;
}


char* session_string;
int backend_session_process(){
  int ret = SESSION_NOMORE;
#ifdef HAVE_LASH
  lash_event_t *e;

  char *name;

  e = lash_get_event(lash_client);
  if(!e){
    return SESSION_NOMORE;
  }

  asprintf(&session_string,"%s",lash_event_get_string(e));
  const int t = lash_event_get_type (e);

  switch(t)
  {
    case LASH_Save_File:
      printf("session_process: LASH save\n");
      lash_send_event(lash_client, lash_event_new_with_type(LASH_Save_File));
      ret = SESSION_SAVE;
      break;
    case LASH_Restore_File:
      printf("session_process: LASH load\n");
      lash_send_event(lash_client, lash_event_new_with_type(LASH_Restore_File));
      ret = SESSION_LOAD;
      break;
    case LASH_Quit:
      printf("session_process: LASH quit\n");
      ret = SESSION_QUIT;
      break;
    default:
      printf("session_process: unhandled LASH event (%d)\n", t);
      switch(t){
        case LASH_Client_Name: printf("LASH_Client_Name\n"); break;
        case LASH_Jack_Client_Name: printf("LASH_Jack_Client_Name\n"); break;
        case LASH_Alsa_Client_ID: printf("LASH_Alsa_Client_ID\n"); break;
        case LASH_Save_File: printf("LASH_Save_File\n"); break;
        case LASH_Save_Data_Set: printf("LASH_Save_Data_Set\n"); break;
        case LASH_Restore_Data_Set: printf("LASH_Restore_Data_Set\n"); break;
        case LASH_Save: printf("LASH_Save\n"); break;
      }
      ret = SESSION_UNHANDLED;
      break;
  }
  //lash_event_destroy(e);
#endif
  return ret;
}

char* get_session_string(){
  return session_string;
}
