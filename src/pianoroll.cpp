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

#include <vector>
#include <fltk/Group.h>
#include <fltk/Widget.h>
#include <fltk/events.h>

#include <stdio.h>
#include <unistd.h>
#include "ui.h"

#include "util.h"

#include "backend.h"

#include "uihelper.h"

extern UI* ui;
extern std::vector<track*> tracks;

extern struct conf config;

using namespace fltk;

#define SWAP(X,Y) tmp=X; X=Y; Y=tmp;

PianoRoll::PianoRoll(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {
  wkeyh = 12;
  bkeyh = 7;
  cur_seqpat = NULL;

  zoom = 15;
  zoom_n = 3;

  q_tick = 32;

  xp_last = 0;
  yp_last = 0;

  box_flag = 0;

  move_toffset = 0;


  resize_arrow = 0;
  resize_e = NULL;
  resize_handle_width = 4;
}

int PianoRoll::handle(int event){
  Command* c;
  pattern* p;
  mevent* e;

  int X = event_x();
  int Y = event_y();

  switch(event){
    case fltk::ENTER:
      return 1;
    case fltk::FOCUS:
      return 1;
    case fltk::SHORTCUT:
      if(event_key()==fltk::DeleteKey){
        apply_delete();
        delete_flag = 0;
        redraw();
        resize_arrow = 0;
        ui->event_edit->redraw();
        return 1;
      }
      if(event_state(CTRL) && event_key()=='c'){
        //printf("roll copy\n");
        return 1;
      }
      if(zoom_out_key(event_key(),event_state())){
        if(zoom_n > 1){
          zoom_n--;
          set_zoom(30*(1<<zoom_n)/16);
          ui->pattern_timeline->zoom = zoom;
          ui->pattern_timeline->update(get_play_position());
          ui->pattern_timeline->redraw();
          ui->event_edit->zoom = zoom;
          ui->event_edit->redraw();
        }
        redraw();
        return 1;
      }
      if(zoom_in_key(event_key(),event_state())){
        if(zoom_n < 8){
          zoom_n++;
          set_zoom(30*(1<<zoom_n)/16);
          ui->pattern_timeline->zoom = zoom;
          ui->pattern_timeline->update(get_play_position());
          ui->pattern_timeline->redraw();
          ui->event_edit->zoom = zoom;
          ui->event_edit->redraw();
        }
        redraw();
        return 1;
      }
      return 0;
    case fltk::PUSH:
      take_focus();
      e = over_note();
      if(event_button()==1){//left mouse
        if(e==NULL){//new note init
          if(event_state()&fltk::SHIFT){//begin box
            box_flag = 1;
            box_x1=X;
            box_x2=X;
            box_y1=Y;
            box_y2=Y;
            box_t1=xpix2tick(X);
            box_t2=box_t1;
            box_n1=ypix2note(Y,1);
            box_n2=box_n1;
          }
          else{//begin insert
            insert_flag = 1;
            insert_torig = quantize(xpix2tick(event_x()));
            insert_toffset = q_tick;
            //new_orig_t = new_left_t;
            insert_note = ypix2note(event_y(),1);

            last_note = insert_note;
            if(config.playinsert){
              ui->keyboard->play_note(last_note,0);
            }
          }

        }
        else{

          if(!(e->selected) && !(event_state()&fltk::SHIFT)){
            unselect_all();
          }
          e->selected = 1;
          resize_arrow_color = fltk::color(128,128,0);

          if(over_rhandle(e,X,Y)){//resize
            rresize_flag = 1;
            rresize_torig = e->tick+e->dur;
            rresize_toffset = 0;
          }
          else if(over_lhandle(e,X,Y)){//resize move
            lresize_flag = 1;
            lresize_torig = e->tick;
            lresize_toffset = 0;
          }
          else{//begin move
            move_flag = 1;

            move_torig = e->tick;
            move_qoffset = e->tick - quantize(e->tick);

            move_toffset = -move_qoffset;

            //move_offset = quantize(xpix2tick(X)) - move_torig - move_qoffset;
            //move_toffset = 0;
            move_offset = X - tick2xpix(e->tick);
            move_norig = ypix2note(event_y(),1);
            move_noffset = 0;

            last_note = move_norig;
            if(config.playmove){
              ui->keyboard->play_note(last_note,0);
            }
          }
        }
      }
      else if(event_button()==2){//middle mouse
        //button initiates paste
      }
      else if(event_button()==3){//right mouse
        if(e==NULL){
          unselect_all();

          ui->event_edit->redraw();
        }
        else{//set up for deletion
          e->selected = 1;
          delete_flag = 1;
          resize_arrow_color = fltk::color(120,60,58);
        }

      }
      redraw();
      return 1;
    case fltk::DRAG:

      if(box_flag){
        box_x2 = X;
        box_y2 = Y;
        box_t2 = xpix2tick(X);
        box_n2 = ypix2note(Y,1);
      }
      else if(insert_flag){
        insert_toffset = quantize(xpix2tick(X)+q_tick) - insert_torig;
        if(insert_toffset<=0){
          insert_toffset -= q_tick;
        }
        insert_note = ypix2note(Y,1);
        if(insert_note != last_note){
          if(config.playinsert){//play on insert
            ui->keyboard->release_note(last_note,0);
            ui->keyboard->play_note(insert_note,0);
          }
          last_note = insert_note;
        }
      }
      else if(move_flag){
        move_toffset = quantize(xpix2tick(X - move_offset)) - move_torig;
        move_noffset = ypix2note(Y,1) - move_norig;
        int N = move_norig+move_noffset;
        if(N != last_note){
          if(config.playmove){//play on move
            ui->keyboard->release_note(last_note,0);
            ui->keyboard->play_note(N,0);
          }
          last_note = N;
        }
      }
      else if(rresize_flag){
        rresize_toffset = quantize(xpix2tick(X)) + q_tick - rresize_torig;
      }
      else if(lresize_flag){
        lresize_toffset = quantize(xpix2tick(X)) - lresize_torig;
      }
      redraw();
      return 1;
    case fltk::RELEASE:
      e = over_note();
      if(event_button()==1){
        if(box_flag){
          apply_box();
          ui->event_edit->redraw();
          box_flag=0;
        }
        else if(rresize_flag){
          apply_rresize();
          rresize_flag = 0;
          resize_arrow = 0;
          ui->event_edit->redraw();
        }
        else if(lresize_flag){
          apply_lresize();
          lresize_flag = 0;
          resize_arrow = 0;
          ui->event_edit->redraw();
        }
        else if(insert_flag){
          apply_insert();

          insert_flag = 0;

          ui->keyboard->release_note(insert_note,0);
          ui->keyboard->redraw();
          ui->event_edit->has[0]=1;
          ui->event_edit->has[1]=1;
          ui->event_edit->redraw();
          ui->event_menu->redraw();
        }
        else if(move_flag){
          apply_move();
          move_flag = 0;

          midi_track_off(cur_seqpat->track);
          ui->keyboard->release_note(last_note,0);
          ui->keyboard->release_note(move_norig+move_noffset,0);
          ui->keyboard->redraw();
          ui->event_edit->redraw();
        }
        insert_flag=0;
        move_flag=0;
      }
      if(event_button()==3){
        mevent* over_n = over_note();
        if(delete_flag && over_n){
          if(over_n->selected){
            apply_delete();
            midi_track_off(cur_seqpat->track);
            ui->event_edit->redraw();
          }
        }
        delete_flag=0;
        resize_arrow = 0;
      }
      redraw();

      return 1;

    case fltk::MOVE:
      e = over_note();
      if(e){
        if(over_rhandle(e,X,Y)){
          if(resize_e != e || resize_arrow != 1){
            if(e->selected){resize_arrow_color = fltk::color(128,128,0);}
            else{resize_arrow_color = fltk::color(95,58,119);}
            resize_e = e;
            resize_arrow = 1;
            resize_x = tick2xpix(e->tick + e->dur) - resize_handle_width;
            resize_y = note2ypix(e->value1);
            redraw();
          }
        }
        else if(over_lhandle(e,X,Y)){
          if(resize_e != e || resize_arrow != 1){
            if(e->selected){resize_arrow_color = fltk::color(128,128,0);}
            else{resize_arrow_color = fltk::color(95,58,119);}
            resize_e = e;
            resize_arrow = -1;
            resize_x = tick2xpix(e->tick)+1;
            resize_y = note2ypix(e->value1);
            redraw();
          }
        }
        else{
          if(resize_e != e || resize_arrow != 0){
            resize_e = e;
            resize_arrow = 0;
            redraw();
          }
        }
      }
      else{
        if(resize_arrow != 0){
          resize_arrow = 0;
          redraw();
        }
      }

      return 1;
  }
  return 0;
}

void PianoRoll::draw(){

  fltk::setcolor(fltk::GRAY05);
  fltk::fillrect(0,0,w(),h());

  fltk::setcolor(fltk::GRAY20);
  for(int i=12; i<h(); i+=12){
    fltk::drawline(0,i,w(),i);
  }
  for(int i=zoom; i<w(); i+=zoom){
    fltk::drawline(i,0,i,h());
  }

  fltk::setcolor(fltk::GRAY30);
  for(int i=12*5; i<h(); i+=12*7){
    fltk::drawline(0,i,w(),i);
  }

  fltk::setcolor(fltk::GRAY50);
  for(int i=zoom*4; i<w(); i+=zoom*4){
    fltk::drawline(i,0,i,h());
  }

  fltk::setcolor(fltk::WHITE);
  int M = config.beats_per_measure;
  for(int i=zoom*4*M; i<w(); i+=zoom*4*M){
    fltk::fillrect(i,0,1,h());
  }

  fltk::setcolor(fltk::color(128,0,0));
  int rightend = tick2xpix(cur_seqpat->dur);
  fltk::fillrect(rightend,0,1,h());

  fltk::setcolor(fltk::color(128,128,0));
  fltk::drawline(0,12*40,w(),12*40);

  int tmp;
  if(insert_flag){
    fltk::setcolor(fltk::BLUE);
    int T1 = insert_torig;
    int T2 = T1 + insert_toffset;
    if(T1>T2){SWAP(T1,T2);}
    int X = tick2xpix(T1)+1;
    int Y = note2ypix(insert_note);
    int W = tick2xpix(T2) - X;
    fltk::fillrect(X,Y,W,11);
  }

  if(move_flag){
    fltk::setcolor(fltk::MAGENTA);
    mevent* ptr = cur_seqpat->p->events->next;
    while(ptr){
      if(ptr->type == MIDI_NOTE_ON && ptr->selected){
        int X = tick2xpix(ptr->tick+move_toffset)+1;
        int Y = note2ypix(ptr->value1+move_noffset);
        int W = tick2xpix(ptr->dur);
        fltk::fillrect(X,Y,W-1,1);
        fltk::fillrect(X,Y+11,W-1,1);
        fltk::fillrect(X,Y,1,11);
        fltk::fillrect(X+W-2,Y,1,11);
      }
      ptr=ptr->next;
    }
  }



  //draw all notes
  mevent* e = cur_seqpat->p->events->next;

  fltk::Color c1,c2,c3;

  while(e){
    if(e->type == MIDI_NOTE_ON){
      //fltk::fillrect(tick2xpix(e->tick),note2ypix(e->value),e->dur,11);

      int R1 = rresize_flag&&e->selected ? rresize_toffset : 0;
      int R2 = lresize_flag&&e->selected ? lresize_toffset : 0;

      int T1 = e->tick + R2;
      int T2 = e->tick+e->dur + R1;

      if(T1 >= T2-q_tick && e->selected){
        if(rresize_flag){
          T1 = e->tick;
          T2 = T1 + q_tick;
        }
        else if(lresize_flag){
          T2 = e->tick + e->dur;
          T1 = T2 - q_tick;
        }
      }

      int X = tick2xpix(T1) + 1;
      int Y = note2ypix(e->value1);
      int W = tick2xpix(T2) - X;
      get_event_color(e,&c1,&c2,&c3);

      fltk::setcolor(c1);
      fltk::fillrect(X+1,Y+1,W-1,10);

      fltk::setcolor(c2);
      fltk::fillrect(X,Y+11,W,1);
      fltk::fillrect(X+W-1,Y+1,1,11);

      fltk::setcolor(c3);
      fltk::fillrect(X,Y,W,1);
      fltk::fillrect(X,Y,1,11);
    }
    e=e->next;
  }


  if(!rresize_flag && !lresize_flag){
    if(resize_arrow > 0){
      setcolor(resize_arrow_color);

      int W = resize_handle_width;
      int H = 12;
      int X = resize_x;
      int Y = resize_y;

      addvertex(X,Y);
      addvertex(X,Y+H);
      addvertex(X+W,Y+H/2);
      fillpath();
    }
    else if(resize_arrow < 0){
      setcolor(resize_arrow_color);

      int W = resize_handle_width;
      int H = 12;
      int X = resize_x;
      int Y = resize_y;

      addvertex(X+W,Y);
      addvertex(X+W,Y+H);
      addvertex(X,Y+H/2);
      fillpath();
    }
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

}


static int kludge = 4; //very powerful magic
void PianoRoll::layout(){

  /* the kludge is used so the fltk::ScrollGroup can update
     widgets not contained within it. Better solution, the
     scrollgroup could do its callback if it scrolls.
     Subclassing fltk::ScrollGroup to add this behavior failed. */
  if(kludge != 0){
    kludge--;
    return;
  }

  ui->pattern_timeline->zoom = zoom;
  ui->event_edit->zoom = zoom;

  if(cur_seqpat){
    int W = tick2xpix(cur_seqpat->dur);
    resize(W+300,h());
  }

  int wp = ui->pattern_scroll->w();
  if(wp > w()){
    w(wp+120);
  }

  int hp = ui->pattern_scroll->h();
  if(hp > h()){
    h(hp+120);
  }

  int xp = ui->pattern_scroll->xposition();
  int yp = ui->pattern_scroll->yposition();

  if(xp > w() - wp){
    xp = w() - wp;
    ui->pattern_scroll->scrollTo(xp,yp);
  }

  ui->pattern_timeline->scroll = xp;
  ui->event_edit->scroll = xp;
  ui->keyboard->scroll = yp;

  if(cur_seqpat){
    cur_seqpat->scrolly = yp;
    cur_seqpat->scrollx = xp;
  }

  if(xp_last != xp){
    ui->pattern_timeline->redraw();
    ui->event_edit->redraw();
  }
  if(yp_last != yp){
    ui->keyboard->redraw();
  }

  yp_last = yp;
  xp_last = xp;
}



void PianoRoll::load(seqpat* s){
  cur_seqpat = s;
  cur_track = tracks[s->track];
  int W = tick2xpix(s->dur);
  resize(W+300,h());

  ui->pattern_timeline->ticks_offset = s->tick;
}

int PianoRoll::note2ypix(int note){
  int udy = 6*(note + (note+7)/12 + note/12) + 12;
  return h() - udy + 1;
}

int PianoRoll::tick2xpix(int tick){
  return tick*zoom*4 / 128;
}

int PianoRoll::xpix2tick(int xpix){
  return xpix*128 / (zoom*4);
}

int PianoRoll::quantize(int tick){
  return tick/q_tick * q_tick;
}


void PianoRoll::set_zoom(int z){
  zoom = z;
  relayout();
  //int W = tick2xpix(cur_seqpat->dur);
  //resize(W+300,h());
}


mevent* PianoRoll::over_note(){
  mevent* e = cur_seqpat->p->events->next;

  int cy, lx, rx;
  while(e){
    if(e->type == MIDI_NOTE_ON){
      cy = note2ypix(e->value1);
      lx = tick2xpix(e->tick);
      rx = tick2xpix(e->tick+e->dur);
      if(event_x() > lx && event_x() < rx &&
         event_y() < cy+12 && event_y() > cy){
        return e;
      }
    }
    e = e->next;
  }

  return NULL;
}



void PianoRoll::update(int pos){
  if(!is_backend_playing() || !cur_seqpat){
    return;
  }
  int wp = ui->pattern_scroll->w();
  int xp = ui->pattern_scroll->xposition();
  int yp = ui->pattern_scroll->yposition();
  int X1 = tick2xpix(pos-cur_seqpat->tick);
  int X2 = X1 - xp;
  if(X1 > w()-40){
    return;
  }
  if(X2 < 0){
    ui->pattern_scroll->scrollTo(X1-50<0?0:X1-50,yp);
  }
  if(X2 > wp-30){
    ui->pattern_scroll->scrollTo(X1-50,yp);
  }
}


void PianoRoll::unselect_all(){
  mevent* e = cur_seqpat->p->events;
  while(e){
    if(e->type == MIDI_NOTE_ON && e->selected==1){
      e->selected = 0;
    }
    e = e->next;
  }
}



void PianoRoll::get_event_color(mevent* e, fltk::Color* c1, fltk::Color* c2, fltk::Color* c3){

  int T1,T2;
  int tmp;
  if(delete_flag){
    if(e->selected){
      *c1 = fltk::color(229,79,75);
      *c2 = fltk::color(120,60,58);
      *c3 = fltk::color(225,131,109);
      return;
    }
  }

  if(box_flag){
    T1=box_t1;
    T2=box_t2;
    int N1 = box_n1;
    int N2 = box_n2;
    int N = e->value1;
    if(T1>T2){SWAP(T1,T2);}
    if(N1<N2){SWAP(N1,N2);}
    if(e->tick+e->dur > T1 && e->tick < T2 && N >= N2 && N <= N1){
      *c1 = fltk::color(108,229,75);
      *c2 = fltk::color(71,120,59);
      *c3 = fltk::color(108,229,75);
      return;
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


void PianoRoll::apply_box(){
  mevent* e = cur_seqpat->p->events->next;
  int tmp;
  int T1=box_t1;
  int T2=box_t2;
  int N1 = box_n1;
  int N2 = box_n2;

  if(T1>T2){SWAP(T1,T2);}
  if(N1<N2){SWAP(N1,N2);}
  while(e){
    int N = e->value1;
    if(e->type == MIDI_NOTE_ON &&
       e->tick+e->dur > T1 && e->tick < T2 && 
       N >= N2 && N <= N1){
        e->selected = 1;
    }
    e = e->next;
  }
}

void PianoRoll::apply_insert(){
  if(insert_note > 127 || insert_note < 0){
    return;
  }

  int tmp;
  int T1 = insert_torig;
  int T2 = T1 + insert_toffset;
  if(T1>T2){SWAP(T1,T2);}

  if(T1 < 0){
    return;
  }

  pattern* p = cur_seqpat->p;
  Command* c=new CreateNote(p,insert_note,127,T1,T2-T1);
  set_undo(c);
  undo_push(1);

  cur_track->restate();
}

void PianoRoll::apply_delete(){
  Command* c;
  mevent* e;
  mevent* next;
  pattern* p = cur_seqpat->p;
  int N=0;

  e = cur_seqpat->p->events->next;
  while(e){
    next = e->next;
    if(e->selected && e->type == MIDI_NOTE_ON){
      c=new DeleteNote(p,e);
      set_undo(c);
      N++;
    }
    e = next;
  }
  undo_push(N);

  cur_track->restate();
}

void PianoRoll::apply_move(){
  if(move_toffset==0 && move_noffset==0){
    return;
  }

  pattern* p = cur_seqpat->p;
  mevent* e = p->events->next;
  while(e){
    int K = e->value1+move_noffset;
    int T = e->tick+move_toffset;
    if(e->type == MIDI_NOTE_ON && e->selected && (T<0 || K < 0 || K > 127)){
      return;
    }
    e = e->next;
  }


  Command* c;
  e = p->events->next;

  mevent* next;
  int M=0;
  for(int i=0; i<tracks.size(); i++){
    e = p->events->next;
    while(e){
      next = e->next;
      if(e->selected && e->modified == 0){
        int K = e->value1 + move_noffset;
        int T = e->tick + move_toffset;
        e->modified = 1;
        c=new MoveNote(p,e,T,K);
        set_undo(c);
        M++;
      }
      e = next;
    }
  }
  undo_push(M);

  e = p->events->next;
  while(e){
    if(e->modified){e->modified=0;}
    e = e->next;
  }

  cur_track->restate();
}

void PianoRoll::apply_paste(){

}



void PianoRoll::apply_rresize(){
  if(rresize_toffset==0){
    return;
  }

  Command* c;
  mevent* e;
  mevent* next;
  pattern* p = cur_seqpat->p;
  int tmp;
  int N=0;

  e = p->events->next;
  while(e){
    next = e->next;
    if(e->selected && e->modified == 0){
      e->modified = 1;
      int W = e->dur;
      int R = rresize_toffset;
      if(W+R < q_tick){
        R = q_tick-W;
      }
      c=new ResizeNote(p,e,W+R);
      set_undo(c);
      N++;
    }
    e = next;
  }

  cur_track->restate();
  undo_push(N);
}

void PianoRoll::apply_lresize(){

}


int PianoRoll::over_rhandle(mevent* e, int X, int Y){
  int X1 = tick2xpix(e->tick);
  int X2 = X1 + tick2xpix(e->dur);
  int Y1 = note2ypix(e->value1);
  int Y2 = Y1 + 12;

  if(X2-X1 < resize_handle_width*3){
    return 0;
  }

  return (Y > Y1 && Y < Y2 && X < X2 && X > X2 - resize_handle_width);
}

int PianoRoll::over_lhandle(mevent* e, int X, int Y){
  int X1 = tick2xpix(e->tick);
  int X2 = X1 + tick2xpix(e->dur);
  int Y1 = note2ypix(e->value1);
  int Y2 = Y1 + 12;

  if(X2-X1 < resize_handle_width*3){
    return 0;
  }

  return (Y > Y1 && Y < Y2 && X < X1+ resize_handle_width +1 && X > X1 + 1);
}
