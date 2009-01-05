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
#include <stdlib.h>
#include <vector>

#include <fltk/Group.h>
#include <fltk/Widget.h>
#include <fltk/events.h>

#include "ui.h"

#include "eventedit.h"

#include "uihelper.h"


#define MAG_MAX 16383

#define SWAP(X,Y) tmp=X; X=Y; Y=tmp;

extern struct conf config;

extern UI* ui;
extern std::vector<track*> tracks;

extern char controller_names[128][64];

using namespace fltk;

EventEdit::EventEdit(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {

  zoom = 15;

  event_type = MIDI_NOTE_ON;
  controller_type = 0;


  label_flag = 0;
  select_flag = 0;
  insert_flag = 0;
  line_flag = 0;
  paste_flag = 0;
  delete_flag = 0;
  box_flag = 0;

  for(int i=0; i<134; i++){
    has[i] = 0;
  }
}

int EventEdit::handle(int event){
  int X,Y;
  switch(event){
    case FOCUS:
      return 1;
    case MOUSEWHEEL:
      if(event_dy() < 0){
        event_type_next();
      }
      else if(event_dy() > 0){
        event_type_prev();
      }
      redraw();
      return 1;
      break;
    case PUSH:
      X=event_x();
      Y=event_y();
      if(event_button()==1){
        if(event_state()&fltk::CTRL){//insert
          insert_flag = 1;
          insert_x = X;
          insert_y = Y;
          insert_t = quantize(xpix2tick(X));
          insert_M = ypix2mag(Y);
        }
        else if(event_state()&fltk::SHIFT){//box select
          box_flag=1;
          box_x1=X;
          box_x2=X;
          box_y1=Y;
          box_y2=Y;
          box_t1=xpix2tick(X);
          box_t2=box_t1;
          box_m1=ypix2mag(Y);
          box_m2=box_m1;
        }
        else{//line
          line_flag=1;
          line_x1=X;
          line_x2=X;
          line_y1=Y;
          line_y2=Y;
          line_t1=xpix2tick(X);
          line_M1=ypix2mag(Y);
          line_t2=line_t1;
          line_M2=line_M1;
        }
      }
      else if(event_button()==2){//paste
        paste_flag = 1;
        paste_x = X;
        paste_t = xpix2tick(X);
      }
      else if(event_button()==3){//delete
        delete_flag = 1;
        delete_x1 = X;
        delete_x2 = X;
        delete_t1 = xpix2tick(X);
        delete_t2 = delete_t1;
      }
      redraw();
      return 1;
      break;
    case DRAG:
      X=event_x();
      Y=event_y();
      if(line_flag){
        line_x2 = X;
        line_y2 = Y;
        line_t2 = xpix2tick(X);
        line_M2 = ypix2mag(Y);
      }
      if(box_flag){
        box_x2 = X;
        box_y2 = Y;
        box_t2 = xpix2tick(X);
        box_m2 = ypix2mag(Y);
      }
      if(insert_flag){
        insert_x = X;
        insert_y = Y;
        insert_t = quantize(xpix2tick(X));
        insert_M = ypix2mag(Y);
      }
      if(delete_flag){
        delete_x2 = X;
        delete_t2 = xpix2tick(X);
      }
      if(paste_flag){
        paste_x = X;
        paste_t = xpix2tick(X);
      }
      redraw();
      break;
    case RELEASE:
      if(event_button()==1){//insert, box, line
        if(line_flag){
          apply_line();
          line_flag=0;
        }
        if(box_flag){
          apply_box();
          ui->piano_roll->redraw();
          box_flag=0;
        }
        if(insert_flag){
          apply_insert();
          insert_flag=0;
        }
      }
      else if(event_button()==2){//complete paste
        apply_paste();
        paste_flag=0;
      }
      else if(event_button()==3){//delete
        apply_delete();
        delete_flag = 0;
        clear_selection();
      }
      redraw();
      break;
  }
  return 0;
}

void EventEdit::draw(){
  fltk::setfont(fltk::HELVETICA,12);
  fltk::setcolor(fltk::GRAY05);
  fltk::fillrect(0,0,w(),h());

  fltk::push_clip(0,0,w(),h());

  if(delete_flag){
    fltk::setcolor(fltk::color(64,0,0));
    int X1,X2;
    if(delete_x1>delete_x2){
      X1=delete_x2;
      X2=delete_x1;
    }
    else{
      X1=delete_x1;
      X2=delete_x2;
    }
    fltk::fillrect(X1,0,X2-X1,h());
  }

  fltk::setcolor(fltk::GRAY20);
  fltk::drawtext(event_type_name(), 2, h()-5);

  fltk::setcolor(fltk::GRAY20);
  fltk::fillrect(0,h()-3,w(),1);
  for(int i=zoom - scroll; i<w(); i+=zoom){
    if(i>=0){
      fltk::drawline(i,0,i,h()-1);
    }
  }

  fltk::setcolor(fltk::GRAY50);
  for(int i=zoom*4-scroll; i<w(); i+=zoom*4){
    if(i>=0){
      fltk::drawline(i,0,i,h()-1);
    }
  }

  fltk::setcolor(fltk::WHITE);
  int M = config.beats_per_measure;
  int I = 0;
  for(int i=1; I<w(); i++){
    I = i*zoom*4*M - scroll;
    if(I>=0){
      fltk::fillrect(I,0,1,h());
    }
  }

  fltk::setcolor(fltk::color(128,0,0));
  int rightend = tick2xpix(cur_seqpat->dur)-scroll;
  if(rightend >=0 && rightend < w()){
    fltk::fillrect(rightend,0,1,h());
  }

  mevent* e = cur_seqpat->p->events->next;

  while(e){
    if(e->type==event_type){
      if(e->type==MIDI_CONTROLLER_CHANGE){
        if(e->value1 == controller_type){
          M = val2mag(e->value2);
        }
        else{
          e=e->next;
          continue;
        }
      }
      else{
        switch(e->type){
          case -1:
            e = e->next;
            continue;
          case MIDI_PROGRAM_CHANGE:
          case MIDI_CHANNEL_PRESSURE:
            M = val2mag(e->value1);
            break;
          default:
            M = val2mag(e->value2);
            break;
        }
      }
      int T1 = line_t1;
      int T2 = line_t2;
      int M1 = line_M1;
      int M2 = line_M2;
      if(T1>T2){
        int tmp = T2;
        T2 = T1;
        T1 = tmp;
        tmp = M2;
        M2 = M1;
        M1 = tmp;
      }
      if(line_flag && e->tick > T1 && e->tick < T2){
        if(!select_flag || e->selected){
          float m = (float)(M2-M1)/(T2-T1);
          float b = M1 - T1*m;
          M = (int)(m*e->tick + b);
          if(M<0){M=0;}
          if(M>MAG_MAX){M=MAG_MAX;}
        }
      }
      int X = tick2xpix(e->tick) - scroll;
      int Y = mag2ypix(M);
      int H = h()-Y;

      fltk::Color c1,c2,c3;
      c1 = fltk::BLACK;
      c2 = fltk::BLACK;
      c3 = fltk::BLACK;
      get_event_color(e,&c1,&c2,&c3);

      fltk::setcolor(c1);
      fltk::fillrect(X,Y+1,1,H);
      fltk::fillrect(X+1,Y,1,1);
      fltk::setcolor(c2);
      fltk::fillrect(X+1,Y+1,1,H);
      fltk::setcolor(c3);
      fltk::fillrect(X,Y,1,1);
      if(label_flag){
        fltk::setcolor(c1);
        char buf[16];
        if(e->type == MIDI_PITCH_WHEEL){
          snprintf(buf,16,"%d",M);
        }
        else{
          snprintf(buf,16,"%d",mag2val(M));
        }
        fltk::drawtext(buf,X-fltk::getwidth(buf),Y+12<h()-3?Y+12:h()-3);
      }
    }
    e = e->next;
  }

  if(line_flag){
    fltk::setcolor(fltk::BLUE);
    fltk::drawline(line_x1,line_y1,line_x2,line_y2);
  }

  if(box_flag){
    fltk::setcolor(fltk::GREEN);
    int X1,X2,Y1,Y2;
    if(box_x1>box_x2){
      X1=box_x2;
      X2=box_x1;
    }
    else{
      X1=box_x1;
      X2=box_x2;
    }
    if(box_y1>box_y2){
      Y1=box_y2;
      Y2=box_y1;
    }
    else{
      Y1=box_y1;
      Y2=box_y2;
    }
    fltk::fillrect(X1,Y1,X2-X1,1);
    fltk::fillrect(X1,Y1,1,Y2-Y1);
    fltk::fillrect(X2,Y1,1,Y2-Y1);
    fltk::fillrect(X1,Y2,X2-X1,1);
  }

  if(insert_flag){
    fltk::setcolor(fltk::CYAN);
    int X = tick2xpix(insert_t)-scroll;
    int Y = insert_y;
    if(Y<0){Y=0;}
    if(Y>h()-3){Y=h()-3;}
    int M = insert_M;
    if(M<0){M=0;}
    if(M>MAG_MAX){M=MAG_MAX;}
    fltk::fillrect(X,Y,2,h()-Y);
    if(label_flag){
      char buf[16];
      if(event_type == MIDI_PITCH_WHEEL){
        snprintf(buf,16,"%d",M);
      }
      else{
        snprintf(buf,16,"%d",mag2val(M));
      }
      fltk::drawtext(buf,X-fltk::getwidth(buf),Y+12<h()-3?Y+12:h()-3);
    }
  }


  fltk::pop_clip();

}


void EventEdit::load(seqpat* s){
  cur_seqpat = s;
  cur_track = tracks[s->track];
  recount_has();
}

int EventEdit::tick2xpix(int tick){
  return tick*zoom*4 / 128;
}

const char* EventEdit::event_type_name(){
  switch(event_type){
    case MIDI_NOTE_OFF:
      return "note off velocity";
    case MIDI_NOTE_ON:
      return "note on velocity";
    case MIDI_AFTERTOUCH:
      return "polyphonic key pressure (aftertouch)";
    case MIDI_CONTROLLER_CHANGE:
      return controller_names[controller_type];
    case MIDI_PROGRAM_CHANGE:
      return "program change";
    case MIDI_CHANNEL_PRESSURE:
      return "channel pressure";
    case MIDI_PITCH_WHEEL:
      return "pitch wheel";
    default:
      return "booya";
  }
}

void EventEdit::event_type_next(){
  switch(event_type){
    case 0x80:
      event_type = 0xA0;
      break;
    case 0x90:
      event_type = 0x80;
      break;
    case 0xA0:
      event_type = 0xC0;
      break;
    case 0xB0:
      if(controller_type == 127){
        event_type = 0x90;
      }
      else{
        controller_type++;
      }
      break;
    case 0xE0:
      event_type = 0xB0;
      controller_type = 0;
      break;
    default:
      event_type += 0x10;
  }
}

void EventEdit::event_type_prev(){
  switch(event_type){
    case 0x80:
      event_type = 0x90;
      break;
    case 0x90:
      event_type = 0xB0;
      controller_type = 127;
      break;
    case 0xC0:
      event_type = 0xA0;
      break;
    case 0xB0:
      if(controller_type == 0){
        event_type = 0xE0;
      }
      else{
        controller_type--;
      }
      break;
    case 0xA0:
      event_type = 0x80;
      break;
    default:
      event_type -= 0x10;
  }
}

void EventEdit::set_event_type(int type, int controller){
  event_type = type;
  controller_type = controller;
}


int EventEdit::ypix2mag(int ypix){
  int H = h()-3;
  int R = ypix*MAG_MAX/H;
  return MAG_MAX-R;
}

int EventEdit::mag2ypix(int mag){
  int H = mag*(h()-3)/MAG_MAX;
  return h()-H-3;
}

int EventEdit::mag2val(int mag){
  return mag*127/MAG_MAX;
}

int EventEdit::val2mag(int val){
  return val*MAG_MAX/127;
}

void EventEdit::apply_line(){
  mevent* e = cur_seqpat->p->events;
  Command* c;
  int N = 0;
  int T1, T2;
  int M1 = line_M1;
  int M2 = line_M2;
  if(line_t1>line_t2){
    T1=line_t2;
    T2=line_t1;
  }
  else{
    T1=line_t1;
    T2=line_t2;
  }
  while(e->tick < T1){
    e = e->next;
    if(!e){
      return;
    }
  }
  while(e){
    if(e->tick > T2){
      break;
    }
    if(match_event_type(e) && (e->selected || !select_flag)){
      float m = (float)(M2-M1)/(T2-T1);
      float b = M1 - m*T1;
      int M = (int)(m*e->tick + b);
      int V1, V2;
      if(M<0){M=0;}
      if(M>MAG_MAX){M=MAG_MAX;}
      switch(e->type){
        case MIDI_NOTE_OFF:
        case MIDI_NOTE_ON:
        case MIDI_AFTERTOUCH:
        case MIDI_CONTROLLER_CHANGE:
          V1 = e->value1;
          V2 = mag2val(M);
          break;
        case MIDI_PROGRAM_CHANGE:
        case MIDI_CHANNEL_PRESSURE:
          V1 = mag2val(M);
          break;
        case MIDI_PITCH_WHEEL:
          V1 = M&0x7f;
          V2 = (M&0x3f80) >> 7;
          break;
      }
      c = new ChangeEvent(e,V1,V2);
      set_undo(c);
      N++;
    }
    e = e->next;
  }
  undo_push(N);
}

void EventEdit::apply_box(){
  select_flag=1;
  mevent* e = cur_seqpat->p->events->next;
  int T1=box_t1;
  int T2=box_t2;
  int M1 = box_m1;
  int M2 = box_m2;
  int tmp;
  while(e){
    if(e->tick > T2){break;}
    int M = get_event_mag(e);
      if(T1>T2){SWAP(T1,T2);}
      if(M1<M2){SWAP(M1,M2);}
      if(e->tick > T1 && e->tick < T2 && M > M2){
        e->selected = 1;
      }
    e=e->next;
  }
}

void EventEdit::apply_insert(){
  int V1,V2;
  Command* c;
  get_event_value(&V1,&V2);
  c = new CreateEvent(cur_seqpat->p,event_type,insert_t,V1,V2);
  set_undo(c);
  undo_push(1);

  switch(event_type){
    case MIDI_NOTE_ON: has[0]=1; break;
    case MIDI_NOTE_OFF: has[1]=1; break;
    case MIDI_AFTERTOUCH: has[2]=1; break;
    case MIDI_PROGRAM_CHANGE: has[3]=1; break;
    case MIDI_CHANNEL_PRESSURE: has[4]=1; break;
    case MIDI_PITCH_WHEEL: has[5]=1; break;
    default: has[controller_type+6]=1; break;
  }

  if(event_type==MIDI_NOTE_ON){
    ui->piano_roll->redraw();
  }
}

void EventEdit::delete_events(int (EventEdit::*pred)(mevent* e)){
  mevent* e = cur_seqpat->p->events->next;
  mevent* next;
  Command* c;
  int N=0;
  while(e){
    if(((this)->*(pred))(e)){
      next = e->next;
      c = new DeleteEvent(e);
      set_undo(c);
      N++;
      e = next;
    }
    else{
      e = e->next;
    }
  }
  undo_push(N);
}


int EventEdit::delete_type_in_range_pred(mevent* e){
  if(e->tick > delete_t1 && e->tick < delete_t2 && match_event_type(e))
    return 1;
  else
    return 0;
}

void EventEdit::apply_delete(){
  delete_events(&EventEdit::delete_type_in_range_pred);
  ui->piano_roll->redraw();
}



void EventEdit::apply_paste(){
printf("apply paste\n");
}

int EventEdit::match_event_type(mevent* e){
  if(e->type == event_type){
    if(e->type == MIDI_CONTROLLER_CHANGE){
      if(e->value1 == controller_type){
        return 1;
      }
    }
    else{
      return 1;
    }
  }
  return 0;
}

void EventEdit::get_event_color(mevent* e, fltk::Color* c1, fltk::Color* c2, fltk::Color* c3){

  int T1,T2;
  int tmp;
  if(delete_flag){
    T1=delete_t1;
    T2=delete_t2;
    if(T1>T2){SWAP(T1,T2);}
    if(e->tick > T1 && e->tick < T2){
      *c1 = fltk::color(229,79,75);
      *c2 = fltk::color(120,60,58);
      *c3 = fltk::color(225,131,109);
      return;
    }
  }

  if(box_flag){
    T1=box_t1;
    T2=box_t2;
    int M1 = box_m1;
    int M2 = box_m2;
    int M = get_event_mag(e);
    if(T1>T2){SWAP(T1,T2);}
    if(M1<M2){SWAP(M1,M2);}
    if(e->tick > T1 && e->tick < T2 && M > M2){
      *c1 = fltk::color(108,229,75);
      *c2 = fltk::color(71,120,59);
      *c3 = fltk::color(108,229,75);
      return;
    }
  }

  if(line_flag){
    T1=line_t1;
    T2=line_t2;
    if(T1>T2){SWAP(T1,T2);}
    if(e->tick > T1 && e->tick < T2){
      if(!select_flag || e->selected){
        *c1 = fltk::color(75,119,229);
        *c2 = fltk::color(58,76,120);
        *c3 = fltk::color(109,123,225);
        return;
      }
    }
  }

  if(e->selected){
    *c1 = fltk::color(255,248,47);
    *c2 = fltk::color(140,137,46);
    *c3 = fltk::color(232,255,37);
    return;
  }

  *c1 = fltk::color(169,75,229);
  *c2 = fltk::color(95,58,119);
  *c3 = fltk::color(198,109,225);
}

void EventEdit::get_event_value(int* v1, int* v2){
  int M = insert_M;
  if(M<0){M=0;}
  if(M>MAG_MAX){M=MAG_MAX;}
  switch(event_type){
    case MIDI_NOTE_OFF:
    case MIDI_NOTE_ON:
    case MIDI_AFTERTOUCH:
      *v1 = 60;
      *v2 = mag2val(M);
      break;
    case MIDI_CONTROLLER_CHANGE:
      *v1 = controller_type;
      *v2 = mag2val(M);
      break;
    case MIDI_PROGRAM_CHANGE:
    case MIDI_CHANNEL_PRESSURE:
      *v1 = mag2val(M);
      break;
    case MIDI_PITCH_WHEEL:
      *v1 = M&0x7f;
      *v2 = (M&0x3f80) >> 7;
      break;
   }
}

int EventEdit::xpix2tick(int xpix){
  ui->piano_roll->xpix2tick(xpix+scroll);
}

int EventEdit::get_event_mag(mevent* e){
  switch(e->type){
    case MIDI_NOTE_OFF:
    case MIDI_NOTE_ON:
    case MIDI_AFTERTOUCH:
    case MIDI_CONTROLLER_CHANGE:
      return val2mag(e->value2);
    case MIDI_PROGRAM_CHANGE:
    case MIDI_CHANNEL_PRESSURE:
      return val2mag(e->value1);
    case MIDI_PITCH_WHEEL:
      return e->value1 | (e->value2<<7);
  }
  return 0;
}


int EventEdit::delete_type_all_pred(mevent* e){
  if(match_event_type(e))
    return 1;
  else
    return 0;
}

int EventEdit::delete_all_non_note_pred(mevent* e){
  if(e->type != MIDI_NOTE_ON && e->type != MIDI_NOTE_OFF)
    return 1;
  else
    return 0;
}

int EventEdit::delete_all_pred(mevent* e){
  return 1;
}

void EventEdit::clear_events(){
  delete_events(&EventEdit::delete_type_all_pred);
  switch(event_type){
    case MIDI_NOTE_ON: has[0]=0; break;
    case MIDI_NOTE_OFF: has[1]=0; break;
    case MIDI_AFTERTOUCH: has[2]=0; break;
    case MIDI_PROGRAM_CHANGE: has[3]=0; break;
    case MIDI_CHANNEL_PRESSURE: has[4]=0; break;
    case MIDI_PITCH_WHEEL: has[5]=0; break;
    default: has[controller_type+6]=0; break;
  }
  redraw();
  ui->piano_roll->redraw();
  ui->event_menu->redraw();
}

void EventEdit::clear_non_note_events(){
  delete_events(&EventEdit::delete_all_non_note_pred);
  redraw();
  ui->piano_roll->redraw();
  for(int i=2;i<134;i++){
    has[i]=0;
  }
  ui->event_menu->redraw();
}

void EventEdit::clear_all_events(){
  delete_events(&EventEdit::delete_all_pred);
  redraw();
  ui->piano_roll->redraw();
  for(int i=0;i<134;i++){
    has[i]=0;
  }
  ui->event_menu->redraw();
}

void EventEdit::clear_selected_events(){

}

void EventEdit::clear_selection(){
  select_flag = 0;
  mevent* e = cur_seqpat->p->events->next;
  while(e){
    e->selected=0;
    e = e->next;
  }
  redraw();
  ui->piano_roll->redraw();
}

int EventEdit::quantize(int tick){
  return ui->piano_roll->quantize(tick);
}

void EventEdit::recount_has(){
  for(int i=0; i<134; i++){has[i]=0;}
  mevent* e = cur_seqpat->p->events->next;
  while(e){
    switch(e->type){
      case MIDI_NOTE_ON: has[0]=1; break;
      case MIDI_NOTE_OFF: has[1]=1; break;
      case MIDI_AFTERTOUCH: has[2]=1; break;
      case MIDI_PROGRAM_CHANGE: has[3]=1; break;
      case MIDI_CHANNEL_PRESSURE: has[4]=1; break;
      case MIDI_PITCH_WHEEL: has[5]=1; break;
      default: has[e->value1+6]=1; break;
    }
    e = e->next;
  }
}
