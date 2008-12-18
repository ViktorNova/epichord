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

  track* t;
  for(int i=0; i<16; i++){
    t = new track();
    t->head->track = i;
    t->chan = i;
    tracks.push_back(t);
  }

  patterns = new pattern();

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

    pattern* tmp = patterns;
    while(tmp->next){
      tmp=tmp->next;
    }
    tmp->next = this;

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

  //float X = rand()*1.0 / RAND_MAX * 360;
  pattern* p = s->p;
  p->h = ((track%16) / 16.0) * 360;
  p->v = default_hsv_value;
  p->regen_colors();
  /*float X = ;
  hsv_to_rgb(X,1,0.8,&r,&g,&b);
  s->color[0][0] = r;
  s->color[0][1] = g;
  s->color[0][2] = b;
  hsv_to_rgb(X,1,0.4,&r,&g,&b);
  s->color[1][0] = r;
  s->color[1][1] = g;
  s->color[1][2] = b;
  hsv_to_rgb(X,1,1,&r,&g,&b);
  s->color[2][0] = r;
  s->color[2][1] = g;
  s->color[2][2] = b;*/
  

  s->scrolly = 300;
  s->scrollx = 0;

  p = patterns;
  s->p->ref_c = 1;
  while(p->next){
    p = p->next;
  }
  p->next = s->p;

  s->prev = tfind<seqpat>(tracks[track]->head,tick);
  s->next = s->prev->next;
  s->skip = s->p->events;
}


void DeleteSeqpat::redo(){
  tremove<seqpat>(s);
}

void DeleteSeqpat::undo(){
  tinsert<seqpat>(s->prev, s);
}

void ResizeSeqpat::redo(){
  tremove<seqpat>(s1);
  tinsert<seqpat>(s1->prev,s2);
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

  //do this and now the audio cannot enter s
  tracks[track1]->skip = s->next;

  track* t = tracks[track2];

  //here possibly wait a few ms for the audio to leave s if it was in

  //perform nitty gritty
  s->track = track2;

  s->tick = tick2;
  int play_pos = get_play_position();
  s->skip = tfind<mevent>(s->p->events, play_pos - s->tick)->next;
  tinsert<seqpat>(targ2,s);

  if(s->tick+s->dur >= play_pos){
    if(tracks[track2]->skip){
      if(s->tick < tracks[track2]->skip->tick){
        tracks[track2]->skip = s;
      }
    }
    else{
      tracks[track2]->skip = s;
    }
  }

  //send note off for all playing notes / all notes on this channel
}

void MoveSeqpat::undo(){

  tremove<seqpat>(s);

  tracks[track2]->skip = s->next;
  s->track = track1;
  s->tick = tick1;

 int play_pos = get_play_position();
  s->skip = tfind<mevent>(s->p->events, play_pos - s->tick)->next;

  tinsert<seqpat>(targ1,s);
  if(s->tick+s->dur >= play_pos){
    if(tracks[track1]->skip){
      if(s->tick < tracks[track1]->skip->tick){
        tracks[track1]->skip = s;
      }
    }
    else{
      tracks[track1]->skip = s;
    }
  }
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



void ResizeNote::redo(){
  tremove<mevent>(l1);
  tinsert<mevent>(l2->prev, l2);
  tremove<mevent>(r1);
  tinsert<mevent>(r2->prev, r2);
}

void ResizeNote::undo(){
  tremove<mevent>(l2);
  tinsert<mevent>(l1->prev, l1);
  tremove<mevent>(r2);
  tinsert<mevent>(r1->prev, r1);
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





/*create a seqpat linked to an existing pattern*/
int create_seqpat(int track, int tick, int len, pattern* p){
  //set_undo(new CreateSeqpat(track, tick, len, p));
}

/*create a seqpat and link it to a new blank pattern*/
int create_seqpat_new(int track, int tick, int dur){
  //set_undo(new CreateSeqpatBlank(track, tick, dur));
}

/*delete a seqpat, if its the last link to a pattern, delete pattern*/
int delete_seqpat(seqpat* s){
  //set_undo(new DeleteSeqpat(s));
}

/*change size of a seqpat, if there are other links, split.
  doesnt affect the linked pattern */
int resize_seqpat(seqpat* s, int dur){
  //set_undo(new ResizeSeqpat(s,dur));
}

/* change the track and start tick of a seqpat */
int move_seqpat(seqpat* s, int track, int tick){
 // set_undo(new MoveSeqpat(s,track,tick));
}

/* resizes by moving the left limit of a seqpat */
int resizemove_seqpat(seqpat* s, int tick, int dur){
  //set_undo(new ResizeSeqpat(s,dur));
  //set_undo(new MoveSeqpat(s,s->track,tick));
}

/* make a copy of its pattern */
int split_seqpat(seqpat* s){
  //set_undo(new SplitSeqpat(s));
}




/* insert new event */
int create_event(pattern* p, int type, int tick, int value){
 // set_undo(new CreateEvent(p,type,tick,value));
}

/* delete an event */
int delete_event(mevent* e){
  //set_undo(new DeleteEvent(e));
}

/* modify the value of an event */
int change_event(mevent* e, int value1, int value2){
  //set_undo(new ChangeEvent(e,value1,value2));
}



/* note modifier functions. note is midi note number, tick is ticks
   past the beginning of pattern, dur is length in ticks */

int create_note(pattern* p, int note, int tick, int dur){
  //set_undo(new CreateNote(p,note,tick,dur));
}

int delete_note(pattern* p, mevent* e){
  //set_undo(new DeleteNote(p, e));
}

int move_note(pattern* p, mevent* e, int note, int tick){
  //set_undo(new MoveNote(p, e, tick, ));
}

int resize_note(pattern* p, mevent* e, int dur){
  //set_undo(new ResizeNote(p, e, dur));
}

int resizemove_note(pattern* p, mevent* e, int tick, int dur){
  //set_undo(new ResizeNote(p, e, dur));
  //set_undo(new MoveNote(p, e, tick));
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
