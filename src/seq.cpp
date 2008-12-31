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

#include <stdlib.h>
#include <math.h>
#include <list>
#include <vector>

#include "seq.h"

#include <fltk/TextDisplay.h>
#include "util.h"

#include "backend.h"

std::vector<track*> tracks;
pattern* patterns;

std::list<Command*> undo_stack;
std::list<int> undo_number;
std::list<Command*>::iterator undo_ptr;

int solo_flag = 0;

int rec_track = 0;

static float default_hsv_value = 0.8;

int init_seq(){

  patterns = new pattern();

  track* t;
  for(int i=0; i<16; i++){
    t = new track();
    t->head->track = i;
    t->chan = i;
    tracks.push_back(t);
  }



}


void pattern_add(pattern* p){
  //printf("pattern_add: adding pattern %p\n",p);
  pattern* ptr = patterns;
  while(ptr->next){
    ptr = ptr->next;
  }
  ptr->next = p;
}


void pattern_remove(pattern* p){
  //printf("pattern_remove: %p removing\n",p);
  pattern* ptr = patterns;
  pattern* prev = patterns;
  while(ptr){
    if(ptr==p){
      if(ptr!=patterns){
        prev->next = ptr->next;
      }
      return;
    }
    prev=ptr;
    ptr=ptr->next;
  }
  printf("pattern_remove: pattern %p not found. not good.\n",p);
}


void pattern_clear(){
  while(patterns->next){
    delete patterns->next;
  }
}




int play_seq(int cur_tick, void (*dispatch_event)(mevent*, int port, int tick)){

  seqpat* s;
  pattern* p;
  mevent* e;
  int base;

  /* start at skip s, and skip e
     if e is in the past, dispatch event and set new skip
     if next e is past pattern dur, switch patseq
     if next e is null, switch patseq
     switch patseq sets skip s
     do the above until e is in the future and not past end
     then go to next track and start over
  */

  //printf("playing. cur_tick = %d\n",cur_tick);
  s = tracks[0]->skip;
  for(int i=0; i<tracks.size(); i++){
    //if(tracks[i]->alive == 0){continue;}
    s = tracks[i]->skip;
    if(!s){continue;}
    p = s->p;
    e = s->skip;

    if(!e){ goto switchpatseq; }//no more events in this seqpat

again:

    if(e->tick+s->tick <= cur_tick && e->tick <= s->dur){//should have happened

      if(e->tick != s->dur || e->type == MIDI_NOTE_OFF)
      if(tracks[i]->mute == 0)
      if((get_solo() && tracks[i]->solo != 0) || !get_solo())
          dispatch_event(e, i, s->tick);

      e = e->next;
      s->skip = e;
      if(!e){//no more events in this seqpat
        goto switchpatseq;
      }
    }
    else if(e->tick > s->dur){//went past the end
      goto switchpatseq;
    }
    else{//no more events on this track right now
      continue;
    }
goto again;//try to play next event
switchpatseq:
      s = s->next;

      tracks[i]->skip = s;
      if(!s){continue;}//no more seqpats
      p = s->p;
      if(!p){continue;}//this shouldnt happen anymore
      e = s->skip;
      if(!e){continue;}//...means this pattern has played already...

      goto again;//play some or all of the next seqpat
  }
}

int set_seq_pos(int new_tick){
  //reset all skip
  //reset cur_seqpat
  seqpat* s;
  pattern* p;
  mevent* e;

  for(int i=0; i<tracks.size(); i++){

    s = tracks[i]->head;
    tracks[i]->skip = s;

    while(s){
      s->skip = s->p->events;
      s = s->next;
    }

    s = tracks[i]->head;
    while(s){
      if(s->tick+s->dur <= new_tick){
        tracks[i]->skip = s;
      }
      else{
        tracks[i]->skip = s;
        break;
      }
      s = s->next;
    }

    if(!s){
      tracks[i]->skip = NULL;
      continue;
    }

    if(!s->p){continue;}
    e = s->p->events;

    while(1){
      if(e == NULL){
        s->skip = e;
        break;
      }
      else if(e->tick >= new_tick - s->tick){
        s->skip = e;
        break;
      }
      e = e->next;
    }
  }
}

void set_rec_track(int t){
  rec_track = t;
}

int get_rec_track(){
  return rec_track;
}

int set_default_hsv_value(float v){
  default_hsv_value = v;
}

void set_undo(Command* c){
  if(undo_ptr != undo_stack.end()){
    //then we need to clean house
    //delete everything at and after undo_ptr (for real);
  }

  undo_stack.push_back(c);
  undo_ptr = undo_stack.end();
  c->redo();
}

void undo_push(int n){
  undo_number.push_back(n);
}

void do_undo(){

  undo_ptr--;
  (*undo_ptr)->undo();
}

void do_redo(){

  (*undo_ptr)->redo();
  undo_ptr++;
}





CreateSeqpat::CreateSeqpat(int track, int tick, seqpat* zs, int copy){
  s = new seqpat(zs);
  if(copy){
    s->p = new pattern(zs->p);
    s->skip = s->p->events;
  }
  s->track = track;
  s->tick = tick;

  s->prev = tfind<seqpat>(tracks[track]->head,tick);
  s->next = s->prev->next;
}

void CreateSeqpat::redo(){
  tinsert<seqpat>(s->prev, s);
}

void CreateSeqpat::undo(){
  tremove<seqpat>(s);
}

CreateSeqpatBlank::CreateSeqpatBlank(int track, int tick, int len){
  s = new seqpat(track, tick, len, new pattern());
  unsigned char r,g,b;

  pattern* p = s->p;
  p->ref_c = 1;
  p->h = ((track%16) / 16.0) * 360;
  p->v = default_hsv_value;
  p->regen_colors();

  s->scrolly = 300;
  s->scrollx = 0;

  s->prev = tfind<seqpat>(tracks[track]->head,tick);
  s->next = s->prev->next;
  s->skip = s->p->events;
}


void DeleteSeqpat::redo(){
  //check for restate
  int pos = get_play_position();
  if(pos >= s->tick && pos <= s->tick+s->dur){
    tracks[s->track]->skip = s->next;
  }

  tremove<seqpat>(s);
}

void DeleteSeqpat::undo(){
  tinsert<seqpat>(s->prev, s);
  //check for restate
}

void ResizeSeqpat::redo(){
  tremove<seqpat>(s1);
  tinsert<seqpat>(s1->prev,s2);

  //check for restate
}

void ResizeSeqpat::undo(){
  tremove<seqpat>(s2);
  tinsert<seqpat>(s1->prev,s1);
}

MoveSeqpat::MoveSeqpat(seqpat* zs, int track, int tick){
  s = zs;
  track1 = s->track;
  track2 = track;
  tick1 = s->tick;
  tick2 = tick;
  targ1 = s->prev;
  targ2 = tfind<seqpat>(tracks[track2]->head,tick);
  if(targ2 == zs){//dont put it after itself
    targ2 = zs->prev;
  }
}

void MoveSeqpat::redo(){

  tremove<seqpat>(s);


  int play_pos = get_play_position();
  if(play_pos >= s->tick && play_pos <= s->tick+s->dur){//no entry into s by cb
    tracks[track1]->skip = s->next;
  }

  track* t = tracks[track2];

  //here possibly wait a few ms for the audio to leave s if it was in

  //perform nitty gritty
  s->track = track2;

  s->tick = tick2;

  s->skip = tfind<mevent>(s->p->events, play_pos - s->tick)->next;
  tinsert<seqpat>(targ2,s);
}

void MoveSeqpat::undo(){

  tremove<seqpat>(s);

  int play_pos = get_play_position();
  if(play_pos >= s->tick && play_pos <= s->tick+s->dur){
    tracks[track2]->skip = s->next;
  }
  s->track = track1;
  s->tick = tick1;


  s->skip = tfind<mevent>(s->p->events, play_pos - s->tick)->next;

  tinsert<seqpat>(targ1,s);
}

void SplitSeqpat::redo(){
  s->p = p2;
}

void SplitSeqpat::undo(){
  s->p = p1;
}




void CreateNote::redo(){
  mevent* targ = tfind<mevent>(p->events, e1->tick);
  tinsert<mevent>(targ, e1);
  targ = tfind<mevent>(p->events, e2->tick);
  tinsert<mevent>(targ, e2);
}

void CreateNote::undo(){
  tremove<mevent>(e1);
  tremove<mevent>(e2);
}

void CreateNoteOn::redo(){
  mevent* targ = tfind<mevent>(p->events, e1->tick);
  tinsert<mevent>(targ, e1);
  //targ = tfind<mevent>(p->events, e2->tick);
  //tinsert<mevent>(targ, e2);
}

void CreateNoteOn::undo(){
  tremove<mevent>(e1);
  //tremove<mevent>(e2);
}


CreateNoteOff::CreateNoteOff(pattern* zp, int note, int vel, int tick){
  p = zp;
  e1 = NULL;
  mevent* ptr = tfind<mevent>(zp->events,tick);
  while(ptr){
    if(ptr->type == MIDI_NOTE_ON && ptr->value1 == note){
      dur1 = ptr->dur;
      dur2 = tick - ptr->tick;
      e1 = ptr;
      break;
    }
    if(ptr->type == MIDI_NOTE_OFF && ptr->value1 == note){
      break;
    }
    ptr = ptr->prev;
  }
  e2 = new mevent(MIDI_NOTE_OFF, tick, note);
  e2->value2 = vel;
}

void CreateNoteOff::redo(){
  //mevent* targ = tfind<mevent>(p->events, e1->tick);
  //tinsert<mevent>(targ, e1);
  mevent* targ;
  targ = tfind<mevent>(p->events, e2->tick);
  tinsert<mevent>(targ, e2);
  if(e1){
    e1->dur = dur2;
  }
}

void CreateNoteOff::undo(){
  //tremove<mevent>(e1);
  if(e1){
    e1->dur = dur1;
  }
  tremove<mevent>(e2);
}

void DeleteNote::redo(){
  tremove<mevent>(e1);
  if(e2){
    tremove<mevent>(e2);
  }
}

void DeleteNote::undo(){
  tinsert<mevent>(e1->prev,e1);
  if(e2){
    tinsert<mevent>(e2->prev,e2);
  }
}



void MoveNote::arrive(int t){
  mevent* targ = tfind<mevent>(p->events, t);
  tinsert<mevent>(targ, e1);
  if(e2){
    targ = tfind<mevent>(p->events, t+e1->dur);
    tinsert<mevent>(targ, e2);
  }
}

void MoveNote::redo(){
  tremove<mevent>(e1);
  e1->tick = t2;
  e1->value1 = note2;
  if(e2){
    tremove<mevent>(e2);
    e2->tick = t2+e1->dur;
    e2->value1 = note2;
  }
  arrive(t2);
}

void MoveNote::undo(){
  tremove<mevent>(e1);
  e1->tick = t1;
  e1->value1 = note1;
  if(e2){
    tremove<mevent>(e2);
    e2->tick = t1+e1->dur;
    e2->value1 = note1;
  }
  arrive(t1);
}




ResizeNote::ResizeNote(pattern* zp, mevent* ze, int dur){
  p = zp;
  l1 = ze;
  l2 = new mevent(ze);
  l2->dur = dur;

  r2 = NULL;
  r1 = find_off(ze);
  if(r1){
    r2 = new mevent(r1);
    r2->prev = tfind<mevent>(p->events, ze->tick + dur);
    if(r2->prev == r1 || r2->prev == l1){
      r2->prev = l2;
    }
    r2->tick = ze->tick + dur;
  }
}

void ResizeNote::redo(){
  tremove<mevent>(l1);
  if(r1){
    tremove<mevent>(r1);
  }

  tinsert<mevent>(l2->prev, l2);
  if(r2){
    tinsert<mevent>(r2->prev, r2);
  }
}

void ResizeNote::undo(){
  if(r2){
    tremove<mevent>(r2);
  }
  tremove<mevent>(l2);

  tinsert<mevent>(l1->prev, l1);
  if(r1){
    tinsert<mevent>(r1->prev, r1);
  }
}




void CreateEvent::redo(){
  tinsert<mevent>(e->prev, e);
}

void CreateEvent::undo(){
  tremove<mevent>(e);
}

void DeleteEvent::redo(){
  tremove<mevent>(e);
}

void DeleteEvent::undo(){
  tinsert<mevent>(e->prev, e);
}

void ChangeEvent::redo(){
  tswap<mevent>(e1,e2);
}

void ChangeEvent::undo(){
  tswap<mevent>(e2,e1);
}





mevent* find_off(mevent* e){
  mevent* ptr = e;
  while(ptr){
    if(ptr->type == MIDI_NOTE_OFF && ptr->value1 == e->value1){
      return ptr;
    }
    ptr = ptr->next;
  }
  return NULL;
}




pattern::pattern(){
  events = new mevent();//dummy
  events->tick = 0;
  next = NULL;
  ref_c = 0;
  h=0;
  s=1;
  v=0.8;
  regen_colors();

  if(patterns){
    pattern_add(this);
  }
}

pattern::~pattern(){
  mevent* e = events;
  mevent* next;
  while(e){
    next = e->next;
    delete e;
    e = next;
  }

  if(this != patterns){
    pattern_remove(this);
  }
}

pattern::pattern(pattern* p){
    mevent* ptr = p->events;
    mevent* ptr2;
    events = new mevent(ptr);
    ptr2 = events;
    ptr=ptr->next;
    while(ptr){
      ptr2->next = new mevent(ptr);
      ptr2->next->prev = ptr2;
      ptr=ptr->next;
      ptr2=ptr2->next; 
    }
    ptr2->next = NULL;

    next = NULL;
    ref_c = 0;
    h = p->h;
    s = p->s;
    v = p->v;
    regen_colors();

    pattern_add(this);
}

void pattern::regen_colors(){

  while(h>360){h-=360;}
  while(h<0){h+=360;}
  if(s < 0){s = 0;}
  if(s > 1){s = 1;}
  if(v < 0.2){v = 0.2;}
  if(v > 0.8){v = 0.8;}

  hsv_to_rgb(h,s,v,&r1,&g1,&b1);
  hsv_to_rgb(h,s,v/2,&r2,&g2,&b2);
  hsv_to_rgb(h,s,v+0.2 > 1 ? 1 : v+0.2,&r3,&g3,&b3);
  if(v > 0.5){
    hsv_to_rgb(h,s,v-0.3,&rx,&gx,&bx);
  }
  else{
    hsv_to_rgb(h,s,0.7,&rx,&gx,&bx);
  }

}


//used when the sequence is changed in such a way 
//that the sequencer state needs to be updated
void track::restate(){
  int pos = get_play_position();
  seqpat* s = head->next;
  int snullflag = 1;
  int pnullflag = 1;
  int pointfound = 0;
  while(s){
//printf("trying block: pos %d, tick %d, t2 %d\n",pos,s->tick,s->tick+s->dur);
    if(pointfound){//we are past the point, reset up coming blocks
      //printf("future block\n");
      s->skip = s->p->events->next;
    }

    else if(s->tick+s->dur < pos){//we are past this block
      //printf("past block\n");
      s->skip = NULL;
    }

    else if(s->tick+s->dur > pos && s->tick <= pos){//we are inside this block, point found
      //printf("inside block\n");
      pointfound = 1;
      snullflag = 0;
      skip = s;
      mevent* e = s->p->events->next;
      while(e){
        if(e->tick+s->tick >= pos){
          s->skip = e;
          pointfound = 1;
          pnullflag = 0;
          break;
        }
        e = e->next;
      }
      if(pnullflag){
        s->skip = NULL;
      }
    }

    else{//we are not in a block, point found
      //printf("pointfound outside block\n");
      skip = s;
      snullflag = 0;
      pointfound = 1;
    }

    s = s->next;
  }

  if(snullflag){skip = NULL;}
}


void seqpat::restate(){
  mevent* e = p->events->next;
  int pos = get_play_position();
  while(e){
    if(e->tick+tick >= pos){
      skip = e;
      return;
    }
    e = e->next;
  }
  skip = NULL;
}


//clear the pattern
void seqpat::apply_erase(){
  if(layers){
    layers->ref_c--;
    if(layers->ref_c == 0){
      delete layers;
    }
  }

  pattern* ptmp = new pattern();
  ptmp->ref_c = 1;
  ptmp->h = p->h;
  ptmp->s = p->s;
  ptmp->v = p->v;
  ptmp->regen_colors();

  if(p){
    if(--(p->ref_c) == 0){
      delete p;
    }
  }

  p = ptmp;
}

//create new pattern and make it current
void seqpat::apply_layer(){
  if(layers){
    p = layers->push_new();
  }
  else{
    layers = new layerstack();
    layers->ref_c = 1;
    layers->push_new(p);
    p = layers->push_new();
  }
}

void seqpat::next_layer(){
  if(layers){
    p = layers->next();
  }
}

void seqpat::prev_layer(){
  if(layers){
    p = layers->prev();
  }
}

int seqpat::layer_index(){
  if(layers){
    return layers->index;
  }
  else{
    return 1;
  }
}

int seqpat::layer_total(){
  if(layers){
    return layers->total;
  }
  else{
    return 1;
  }
}

void seqpat::record_check(int mode){
  if(record_flag==0){
    if(mode==1 || mode==2){
      if(mode == 1){apply_erase();}
      else if(mode == 2){/*apply_layer();*/}
      tracks[track]->restate();
      //midi_track_off(track);
    }
    record_flag = 1;
  }
}





pattern* layerstack::push_new(){
  if(total==memsize){
    //reallocate
  }

  index++;
  total++;
  array[index] = new pattern();
  return array[index];
}

void layerstack::push_new(pattern* p){
  if(total==memsize){
    //reallocate
  }

  index++;
  total++;
  array[index] = p;
  p->ref_c++;
}

pattern* layerstack::next(){
  if(index==total-1){
    index=0;
  }
  else{
    index++;
  }
  return array[index];
}

pattern* layerstack::prev(){
  if(index==0){
    index=total-1;
  }
  else{
    index--;
  }
  return array[index];
}

layerstack::layerstack(pattern* p){
  index = 0;
  total = 1;
  array = new pattern*[16];
  memsize = 16;
  array[0] = p;
  ptr = array[0];
  ref_c = 0;
}

layerstack::~layerstack(){
  for(int i=0; i<total; i++){
    array[i]->ref_c--;
    if(array[i]->ref_c == 0){
      delete array[i];
    }
  }
}



void reset_record_flags(){
  for(int i=0; i<tracks.size(); i++){
    seqpat* s = tracks[i]->head->next;
    while(s){
      if(s->record_flag == 1){
        s->record_flag = 0;
      }
      s = s->next;
    }
  }
}
