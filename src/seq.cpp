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

std::list<Command*> undo_stack;
std::list<Command*>::iterator undo_ptr;
std::list<int> undo_number;
std::list<int>::iterator undo_nptr;


int solo_flag = 0;

int rec_track = 0;

static float default_hsv_value = 0.8;





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


SplitSeqpat::SplitSeqpat(seqpat* zs, int tick){
  s = zs;

  int w1 = tick - s->tick;
  int w2 = s->tick+s->dur - tick;

  pattern* p1 = new pattern();
  pattern* p2 = new pattern();

  p1->h = randf(0,360);
  p2->h = randf(0,360);
  p1->regen_colors();
  p2->regen_colors();

  //split the events into two groups
  mevent* e = s->p->events->next;
  while(e){
    mevent* f = new mevent(e);
    f->next = NULL;
    if(e->tick < tick-s->tick){//group 1
      p1->append(f);
    }
    else if(e->tick == tick-s->tick){//depends, maybe one, maybe other
      if(e->type == MIDI_NOTE_OFF){
        p1->append(f);
      }
      else{
        p2->append(f);
        f->tick -= w1;
      }
    }
    else{//group 2
      p2->append(f);
      f->tick -= w1;
    }
    e = e->next;
  }

  s1 = new seqpat(zs,p1);
  s1->dur = w1;
  s1->scrollx = 0;

  s2 = new seqpat(zs,p2);
  s2->tick = s1->tick + w1;
  s2->dur = w2;
  s2->scrollx = 0;


}

void SplitSeqpat::redo(){
  tremove<seqpat>(s);
  tinsert<seqpat>(s->prev,s1);
  tinsert<seqpat>(s1,s2);
}

void SplitSeqpat::undo(){
  tremove<seqpat>(s2);
  tremove<seqpat>(s1);
  tinsert<seqpat>(s->prev,s);
}



JoinSeqpat::JoinSeqpat(seqpat* zs1, seqpat* zs2){
  s1 = zs1;
  s2 = zs2;

  pattern* p = new pattern();
  p->h = randf(0,360);
  p->regen_colors();

  mevent* e = s1->p->events->next;
  while(e){
    mevent* f = new mevent(e);
    f->next = NULL;
    p->append(f);
    e=e->next;
  }

  e=s2->p->events->next;
  while(e){
    mevent* f = new mevent(e);
    f->next = NULL;
    f->tick += s1->dur;
    p->append(f);
    e=e->next;
  }

  s = new seqpat(s1,p);
  s->scrollx = 0;
  s->dur = s1->dur + s2->dur;
}

void JoinSeqpat::redo(){
  tremove<seqpat>(s2);
  tremove<seqpat>(s1);
  tinsert<seqpat>(s->prev,s);
}

void JoinSeqpat::undo(){
  tremove<seqpat>(s);
  tinsert<seqpat>(s->prev,s1);
  tinsert<seqpat>(s1,s2);
}


void ClearSeqpat::redo(){
  s->p = p2;
}

void ClearSeqpat::undo(){
  s->p = p1;
}

void LayerSeqpat::redo(){
  if(s->layers == NULL){
    s->layers = new layerstack(s->p);
  }
  s->layers->push(p);
  s->p = p;
}

void LayerSeqpat::undo(){
  s->p = s->layers->pop();
printf("ok...%d\n",s->layers->total);
  if(s->layers->total == 1){
printf("deleting layers\n");
    delete s->layers;
    s->layers = NULL;
  }
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

}

pattern::~pattern(){
  mevent* e = events;
  mevent* next;
  while(e){
    next = e->next;
    delete e;
    e = next;
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

void pattern::append(mevent* ze){
  mevent* e = events;
  while(e->next){
    e = e->next;
  }
  e->next = ze;
  ze->prev = e;
}

void pattern::insert(mevent* ze, int tick){
  mevent* ptr = events;
  while(ptr->next){
    if(ptr->next->tick > tick){
      ze->next = ptr->next;
      ze->next->prev = ze;
      ptr->next = ze;
      ze->prev = ptr;
      return;
    }
    ptr=ptr->next;
  }
  ptr->next = ze;
  ze->prev = ptr;
}


void pattern::fixdur(){
  mevent* e = events;
  mevent* f;
  while(e->next){
    if(e->type == MIDI_NOTE_ON){
      f = find_off(e);
      if(f){e->dur = f->tick - e->tick;}
    }
    e = e->next;
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

  Command* c;

  c = new ClearSeqpat(this);
  set_undo(c);
  undo_push(1);
}

//create new pattern and make it current
void seqpat::apply_layer(){
  Command* c = new LayerSeqpat(this);
  set_undo(c);
  undo_push(1);
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
    return 0;
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
      autocomplete();//end stuck notes
      if(mode == 1){apply_erase();}
      else if(mode == 2){apply_layer();}
      tracks[track]->restate();
    }
    record_flag = 1;
  }
}

//sends note off if it determines that
//we are in between two notes
void seqpat::autocomplete(){
  mevent* ptr;
  mevent* e = p->events->next;
  mevent* eoff;

  int pos = get_play_position()-tick;

  int chan = tracks[track]->chan;
  int port = tracks[track]->port;

  while(e){
    if(e->type == MIDI_NOTE_ON && e->tick < pos){
      eoff = find_off(e);
      if(eoff){
        if(eoff->tick > pos){
          midi_note_off(e->value1,chan,port);
        }
      }
    }
    e = e->next;
  }
}







pattern* layerstack::push_new(){
  if(total==memsize){
    reallocate();
  }
  total++;
  array[total-1] = new pattern();
  array[total-1]->h = array[0]->h;
  array[total-1]->s = array[0]->s;
  array[total-1]->v = array[0]->v;
  array[total-1]->regen_colors();
  index=total-1;
  return array[index];
}

void layerstack::push(pattern* p){
  if(total==memsize){
    reallocate();
  }

  total++;
  array[total-1] = p;
  index = total-1;
  p->ref_c++;
}

void layerstack::reallocate(){
  pattern** ptmp = new pattern*[memsize*2];
  for(int i=0; i<memsize; i++){
    ptmp[i] = array[i];
  }
  memsize *= 2;
  delete [] array;
  array = ptmp;
}

pattern* layerstack::pop(){
  if(index == 0){
    return NULL;
  }
  if(index == total-1){
    index--;
  }
  array[total-1]=NULL;
  total--;
  return array[index];
}

void layerstack::remove(int n){

}

void layerstack::insert(pattern* p, int n){

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
 // ptr = array[0];
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




void set_undo(Command* c){
  if(undo_ptr != undo_stack.end()){
    //printf("changing the past, need to erase the future\n");
    std::list<int>::iterator nptr = undo_number.end();
    nptr--;
    while(nptr != undo_nptr){
      int N = *nptr;
//printf("deleting %d commands\n",N);
      for(int i=0; i<N; i++){
        //delete this command
        undo_stack.pop_back();
      }
      nptr--;
      undo_number.pop_back();
    }
  }
//printf("pushing command\n");
  undo_stack.push_back(c);
  undo_ptr = undo_stack.end();
  c->redo();
}

void undo_push(int n){
  if(n==0){return;}
//printf("pushing number of commands %d\n",n);
  undo_number.push_back(n);
  undo_nptr++;
}

void do_undo(){
  if(undo_ptr==undo_stack.begin()){
    printf("no more to undo!\n");
    return;
  }
//printf("undoing\n");
  int N = *undo_nptr;
  undo_nptr--;
  for(int i=0; i<N; i++){
    undo_ptr--;
    (*undo_ptr)->undo();
  }
}

void do_redo(){
  if(undo_ptr==undo_stack.end()){
    printf("no more to redo!\n");
    return;
  }
//printf("redoing\n");
  undo_nptr++;
  int N = *undo_nptr;
  for(int i=0; i<N; i++){
    (*undo_ptr)->redo();
    undo_ptr++;
  }
}

void undo_reset(){
  //printf("undo reset\n");
  int N = undo_stack.size();
  for(int i=0; i<N; i++){
    std::list<Command*>::iterator c = undo_stack.end();
    c--;
    //delete (*c);
    undo_stack.pop_back();
  }

  N = undo_number.size();
  for(int i=0; i<N; i++){
    undo_number.pop_back();
  }

  undo_nptr = undo_number.begin();
  undo_ptr = undo_stack.begin();
}
