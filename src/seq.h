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

#ifndef seq_h
#define seq_h

#define MIDI_NOTE_OFF 0x80
#define MIDI_NOTE_ON 0x90
#define MIDI_AFTERTOUCH 0xA0
#define MIDI_CONTROLLER_CHANGE 0xB0
#define MIDI_PROGRAM_CHANGE 0xC0
#define MIDI_CHANNEL_PRESSURE 0xD0
#define MIDI_PITCH_WHEEL 0xE0

#include <stdlib.h>
#include <stdio.h>
struct mevent {

  public:
  int tick;

  int type;
  int value1;
  int value2;
  int dur; //for note on events

  //struct mevent* off; //for note on events
  //int off_index; //used when reloading

  struct mevent* prev;
  struct mevent* next;

  int selected;
  int modified;

  mevent(){
    type = -1;
    //off = NULL;
    prev = NULL;
    next = NULL;
    dur = 32;
    selected = 0;
    modified = 0;
  }

  mevent(int ztype, int ztick, int zv1){
    //off=NULL;
    prev=NULL;
    next=NULL;
    dur = 32;
    type=ztype;
    tick=ztick;
    value1=zv1;
    value2=0x7f;
    selected = 0;
    modified = 0;
  }

  mevent(mevent* e){
    type = e->type;
    tick = e->tick;
    value1 = e->value1;
    value2 = e->value2;
    dur = e->dur;
    //off = e->off;
    prev = e->prev;
    next = e->next;
    selected = e->selected;
    modified = e->modified;
  }

};

struct pattern {
  public:
  struct mevent* events;
  struct pattern* next;
  int ref_c;

  unsigned char r1,g1,b1;//main color
  unsigned char r2,g2,b2;//bottom color
  unsigned char r3,g3,b3;//top color
  unsigned char rx,gx,bx;//xray color

  float h, s, v; //color

  void regen_colors();

  pattern();
  pattern(pattern* p);
  ~pattern();
  void append(mevent* ze);
  void insert(mevent* ze, int tick);
  void fixdur();
};



struct layerstack {
  pattern** array;

  int index;
  int total;
  int memsize;

  int ref_c;

  pattern* push_new();
  void push(pattern* p);
  pattern* pop();
  void remove(int n);
  void insert(pattern* p, int n);
  pattern* next();
  pattern* prev();
  void reallocate();

  layerstack(){};
  layerstack(pattern* p);
  ~layerstack();
};


struct seqpat {
  int track;
  int tick;
  int dur;
  struct pattern* p;
  struct mevent* skip;
  struct seqpat* prev;
  struct seqpat* next;

  layerstack* layers;

 // unsigned char color[3][3];
  int selected;
  int modified;
  int record_flag;//0=on record, erase/save. 1=dont

  int scrollx, scrolly;

  void restate();
  void apply_erase();
  void apply_layer();
  void next_layer();
  void prev_layer();
  int layer_index();
  int layer_total();
  void record_check(int mode);
  void autocomplete();


  seqpat(){
    p = NULL;
    skip = NULL;
    prev = NULL;
    next = NULL;
    dur=0;
    selected=0;
    modified=0;
    record_flag=1;
    layers = NULL;
    scrollx = 0;
    scrolly = 300;
  }

  seqpat(int ztrack, int ztick, int zdur, pattern* zp){
    p = zp;
    p->ref_c++;
    track = ztrack;
    dur = zdur;
    tick = ztick;
    skip = NULL;
    prev = NULL;
    next = NULL;
    selected = 0;
    modified = 0;
    record_flag = 1;
    layers = NULL;
    scrollx = 0;
    scrolly = 300;
  }

  seqpat(seqpat* zs){
    p = zs->p;
    p->ref_c++;
    track = zs->track;
    dur = zs->dur;
    tick = zs->tick;
    if(p){p->ref_c++;}

    skip = p->events;
    prev = zs->prev;
    next = zs->next;

    scrollx = zs->scrollx;
    scrolly = zs->scrolly;

    selected = zs->selected;
    modified = zs->modified;

    record_flag = 1;
    layers = NULL;
  }

  seqpat(seqpat* zs, pattern* zp){
    p = zp;
    p->ref_c++;
    track = zs->track;
    dur = zs->dur;
    tick = zs->tick;
    if(p){p->ref_c++;}

    skip = p->events;
    prev = zs->prev;
    next = zs->next;

    scrollx = zs->scrollx;
    scrolly = zs->scrolly;

    selected = zs->selected;
    modified = zs->modified;

    record_flag=1;
    layers = NULL;
  }

  ~seqpat(){
     if(p){
       p->ref_c--;
       if(p->ref_c == 0){
         delete p;
       }
     }
     if(layers){
       layers->ref_c--;
       if(layers->ref_c == 0){
         delete layers;
       }
     }
   }
};







struct track {
  int port;
  int chan;
  int prog;
  int bank;
  int mute;
  int solo;
  int vol;
  int pan;
  char* name;
  int alive;
  seqpat* head;
  seqpat* skip;

  int modified;

  void restate();

  track(){
    port = 0;
    chan = 0;
    prog = 0;
    bank = 0;
    mute = 0;
    solo = 0;
    vol = 127;
    pan = 64;
    name = (char*)malloc(8);
    name[0] = '\0';
    alive = 1;
    head = new seqpat(0,0,0,new pattern());
    head->tick = 0;
    skip = head;
    modified = 0;
  }

  ~track(){
    free(name);
    seqpat* s = head;
    seqpat* next;
    while(s){
      next = s->next;
      delete s;
      s = next;
    }
  }
};





template <class T>
void tswap(T* old, T* nu){
  nu->next = old->next;
  nu->prev = old->prev;

  if(old->prev){
    old->prev->next = nu; //'atomic'
  }
  if(old->next){
    old->next->prev = nu; //prev ptrs are only read by gui thread
  }
}

template <class T>
void tremove(T* old){
  if(old->prev){
    old->prev->next = old->next; //'atomic'
  }
  if(old->next){
    old->next->prev = old->prev; //prev ptrs are only read by gui thread
  }
}

template <class T>
void tinsert(T* targ, T* nu){
  nu->prev = targ;
  nu->next = targ->next;

  targ->next = nu; //'atomic'
  if(nu->next){
    nu->next->prev = nu; //prev ptrs are only read by gui thread
  }
}

template <class T>
T* tfind(T* begin, int tick){
  T* ptr = begin;
  while(ptr->next){
    if(ptr->next->tick > tick){
      break;
    }
    ptr = ptr->next;
  }
  return ptr;
}

mevent* find_off(mevent* e);


class Command {

  public:

  virtual void undo() {}
  virtual void redo() {}

};

void set_undo(Command* c);
void push_undo(int n);
void do_undo();
void do_redo();

int clear_undos(int number);



class CreateSeqpat : public Command {
  protected:
    seqpat* s;

  public:

    CreateSeqpat(){}
    CreateSeqpat(int track, int tick, seqpat* zs, int copy);

    ~CreateSeqpat(){
      if(s->p->ref_c-- == 0){
        delete s->p;
      }
      delete s;
    }

    void redo();
    void undo();
};

class CreateSeqpatBlank : public CreateSeqpat {

  public:

    CreateSeqpatBlank(int track, int tick, int len);

};


class DeleteSeqpat : public Command {
    seqpat* s;

  public:

    DeleteSeqpat(seqpat* zs){
      s = zs;
    }

    void redo();
    void undo();
};

class ResizeSeqpat : public Command {
    seqpat* s1;
    seqpat* s2;

  public:

    ResizeSeqpat(seqpat* zs, int dur){
      s1 = zs;
      s2 = new seqpat(zs);
      s2->dur = dur;
    }
    ~ResizeSeqpat(){
      delete s2;
    }

    void redo();
    void undo();
};

class MoveSeqpat : public Command {
    seqpat* s;
    seqpat *targ1, *targ2;
    int track1, track2;
    int tick1, tick2;

  public:

    MoveSeqpat(seqpat* zs, int track, int tick);
    ~MoveSeqpat(){}

    void redo();
    void undo();
};


class SplitSeqpat : public Command {
    seqpat* s;
    seqpat* s1;
    seqpat* s2;

  public:

    SplitSeqpat(seqpat* zs, int tick);

    ~SplitSeqpat(){
      //delete s1 and s2
    }

    void redo();
    void undo();
};



class JoinSeqpat : public Command {
    seqpat* s1;
    seqpat* s2;
    seqpat* s;

  public:

    JoinSeqpat(seqpat* zs1, seqpat* zs2);

    ~JoinSeqpat(){
      //delete s
    }

    void redo();
    void undo();
};

class ClearSeqpat : public Command {

    seqpat* s;
    pattern* p1;
    pattern* p2;

  public:

    ClearSeqpat(seqpat* zs){
      s = zs;
      p1 = s->p;
      p2 = new pattern();
      p2->ref_c = 1;
    }

    ~ClearSeqpat(){
      //delete p2
    }

    void redo();
    void undo();
};

class LayerSeqpat : public Command {

    seqpat* s;
    pattern* p;

  public:

    LayerSeqpat(seqpat* zs){
      s = zs;
      p = new pattern();
      p->ref_c = 1;
    }

    ~LayerSeqpat(){
      //delete p2
    }

    void redo();
    void undo();
};




class CreateNote : public Command {
    pattern* p;
    mevent* e1;
    mevent* e2;

  public:

    CreateNote(pattern* zp, int note, int vel, int tick, int dur){
      p = zp;
      e1 = new mevent(MIDI_NOTE_ON, tick, note);
      e1->dur = dur;
      e1->value2 = vel;
      e2 = new mevent(MIDI_NOTE_OFF, tick+dur, note);
      e2->value2 = 0;
      //e->off = new mevent(MIDI_NOTE_OFF, tick+dur, note);
    }
    ~CreateNote(){
      //delete e->off;
      delete e1;
      delete e2;
    }

    void redo();
    void undo();
};

class CreateNoteOn : public Command {
    pattern* p;
    mevent* e1;

  public:

    CreateNoteOn(pattern* zp, int note, int vel, int tick, int dur){
      p = zp;
      e1 = new mevent(MIDI_NOTE_ON, tick, note);
      e1->value2 = vel;
      e1->dur = dur;
    }
    ~CreateNoteOn(){
      delete e1;
    }

    void redo();
    void undo();
};

class CreateNoteOff : public Command {
    pattern* p;
    mevent* e1;
    mevent* e2;

    int dur1;
    int dur2;

  public:

    CreateNoteOff(pattern* zp, int note, int vel, int tick);
    ~CreateNoteOff(){
      delete e2;
    }

    void redo();
    void undo();
};

class DeleteNote : public Command {
    pattern* p;
    mevent* e1;
    mevent* e2;

  public:

    DeleteNote(pattern* zp, mevent* ze){
      p = zp; 
      e1 = ze;
      e2 = find_off(e1);
    }

    void redo();
    void undo();
};

class MoveNote : public Command {
    pattern* p;
    mevent* e1;
    mevent* e2;
    int t1;
    int t2;
    int note1;
    int note2;

    void arrive(int t);

  public:

    MoveNote(pattern* zp, mevent* ze, int zt, int znote){
      p = zp; 
      e1 = ze;
      e2 = find_off(e1);
      note1 = ze->value1;
      note2 = znote;
      t1 = ze->tick; 
      t2 = zt;
    }

    void redo();
    void undo();
};


class ResizeNote : public Command {
  pattern* p;
  mevent* l1;
  int d1;
  int d2;
  //mevent* l2;
  mevent* r1;
  mevent* r2;

  public:

  ResizeNote(pattern* zp, mevent* ze, int dur);
  ~ResizeNote(){
    if(r2){
      delete r2;
    }
  }

  void redo();
  void undo();

};



class CreateEvent : public Command {
    pattern* p;
    mevent* e;

  public:

    CreateEvent(pattern* zp, int type, int tick, int value1, int value2){
      p = zp;
      e = new mevent(type,tick,value1);
      e->value2 = value2;
      e->prev = tfind<mevent>(zp->events,tick);
      e->next = e->prev->next;
    }
    ~CreateEvent(){
      delete e;
    }

    void redo();
    void undo();
};

class DeleteEvent : public Command {
    mevent* e;

  public:

    DeleteEvent(mevent* ze){
      e = ze;
    }

    void redo();
    void undo();
};

class ChangeEvent : public Command {
    mevent* e1;
    mevent* e2;

  public:

    ChangeEvent(mevent* ze, int zv1, int zv2){
      e1 = ze;
      e2 = new mevent(ze);
      e2->value1 = zv1;
      e2->value2 = zv2;
    }

    void redo();
    void undo();
};



int play_seq(int cur_tick);
int set_seq_pos(int new_tick);

void set_rec_track(int t);
int get_rec_track();

int set_default_hsv_value(float v);

void set_undo(Command* c);
void undo_push(int n);
void do_undo();
void do_redo();
void undo_reset();


void reset_record_flags();


//encodes data in e as a midi event placed in buf
int midi_encode(mevent* e, int chan, unsigned char* buf, size_t* n);

//decodes midi data and creates a new mevent
int midi_decode(char* buf, mevent* e);

#endif
