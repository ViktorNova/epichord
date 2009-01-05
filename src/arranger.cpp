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
#include <vector>
#include <fltk/Group.h>
#include <fltk/Widget.h>
#include <fltk/events.h>

#include "ui.h"

#include "uihelper.h"

#include "util.h"

#include "backend.h"

extern UI* ui;

extern std::vector<track*> tracks;

extern struct conf config;

using namespace fltk;

#define SWAP(X,Y) tmp=X; X=Y; Y=tmp;

Arranger::Arranger(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {
  new_default_w = 128*4;
  delete_flag = 0;
  move_flag = 0;
  paste_flag = 0;
  main_sel = NULL;

  zoom = 30;
  zoom_n = 4;

  q_tick = 128*4;

  xp_last = 0;
  yp_last = 0;

  insert_flag = 0;
  box_flag = 0;
  rresize_flag = 0;
  lresize_flag = 0;

  last_handle == NULL;

  color_flag = 0;

  maxt = 0;

}

int Arranger::handle(int event){
  Command* c;

  int X = event_x();
  int Y = event_y();

  seqpat* s;

  switch(event){
    case fltk::FOCUS:
      return 1;
    case fltk::ENTER:
      return 1;
    case fltk::KEYUP:

      return 0;
    case fltk::MOUSEWHEEL:
      s = over_seqpat();
      if(s){
        s->autocomplete();
        if(event_dy()>0){
          s->prev_layer();
        }
        else if(event_dy()<0){
          s->next_layer();
        }
        s->restate();
        redraw();
      }
      return 1;
    case fltk::SHORTCUT:
      if(event_state() && event_key()=='c'){

        return 1;
      }
      if(event_state() && event_key()=='v'){

        return 1;
      }
      if(event_state() && event_key()=='z'){

        return 1;
      }
      if(event_key()==fltk::DeleteKey){
        apply_delete();
        delete_flag = 0;
        redraw();
        return 1;
      }
      if(zoom_out_key(event_key(),event_state())){
      //if(event_key()==fltk::LeftKey){
        if(zoom_n > 1){
          zoom_n--;
          zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->update(get_play_position());
          ui->song_timeline->redraw();
          relayout();
        }
        redraw();
        return 1;
      }
      if(zoom_in_key(event_key(),event_state())){
      //if(event_key()==fltk::RightKey){
        if(zoom_n < 8){
          zoom_n++;
          zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->update(get_play_position());
          ui->song_timeline->redraw();
          relayout();
        }
        redraw();
        return 1;
      }
      return 0;
    case fltk::PUSH:
      take_focus();
      if(event_button()==1){//left mouse
        seqpat* s = over_seqpat();
        if(s==NULL){
          if(color_flag){//do nothing
          }
          else if(event_state()&fltk::SHIFT){//begin box
            box_flag = 1;
            box_x1=X;
            box_x2=X;
            box_y1=Y;
            box_y2=Y;
            box_t1=xpix2tick(X);
            box_t2=box_t1;
            box_k1=Y/30;
            box_k2=box_k1;
          }
          else{//begin insert
            insert_flag = 1;
            insert_torig = xpix2tick(X)/q_tick*q_tick;
            insert_toffset = q_tick;
            insert_track = event_y() / 30;
          }
        }
        else{
          main_sel = s;
          if(color_flag){
            color_sel = s->p;
            color_orig_x = event_x();
            color_orig_y = event_y();
            color_orig_h = color_sel->h;
            color_orig_v = color_sel->v;
            color_h = color_orig_h;
            color_v = color_orig_v;
            return 1;
          }
          if(!s->selected && !(event_state()&SHIFT)){
            unselect_all();
          }
          s->selected = 1;
          if(fltk::event_clicks() > 0){//'double click'
            ui->piano_roll->load(s);
            ui->event_edit->load(s);
            ui->pattern_scroll->scrollTo(s->scrollx,s->scrolly);
            ui->pattern_timeline->update(get_play_position());
            ui->keyboard->cur_port = tracks[s->track]->port;
            ui->keyboard->cur_chan = tracks[s->track]->chan;
            ui->track_info->set_rec(s->track);
            set_rec_track(s->track);
            show_pattern_edit();
            return 1;
          }

          if(over_lhandle(s,X,Y)){//begin resize
            lresize_flag = 1;
            lresize_torig = s->tick;
            lresize_toffset = 0;
          }
          else if(over_rhandle(s,X,Y)){//begin resizemove
            rresize_flag = 1;
            rresize_torig = s->tick+s->dur;
            rresize_toffset = 0;
          }

          else{//begin move
            move_flag = 1;
            move_torig = s->tick;
            move_toffset = 0;
            move_korig = s->track;
            move_koffset = 0;
            move_x = X;
            move_y = Y;
            move_offset = xpix2tick(X)/q_tick*q_tick - s->tick;
          }
        }
      }
      else if(event_button()==2){//middle mouse
        seqpat* s = over_seqpat();
        if(color_flag && s){
          s->p->h = color_h;
          s->p->v = color_v;
          s->p->regen_colors();
          redraw();
          return 1;
        }
        if(main_sel){
          paste_flag = 1;
          paste_t = quantize(xpix2tick(event_x()));
          paste_track = event_y() / 30;
        }
      }
      else if(event_button()==3){//right mouse
        seqpat* s = over_seqpat();
        if(color_flag && s){
          seqpat* ptr = tracks[s->track]->head->next;
          while(ptr){
            ptr->p->h = color_h;
            ptr->p->v = color_v;
            ptr->p->regen_colors();
            ptr = ptr->next;
          }
          redraw();
          return 1;
        }
        if(s==NULL){
          unselect_all();
          delete_sel = NULL;
          main_sel = NULL;
          color_sel = NULL;
        }
        else{//begin delete
          delete_flag = 1;
          delete_sel = s;//this line needs to be removed
          if(!(s->selected)){
            unselect_all();
          }
          s->selected = 1;
        }
      }
      redraw();
      return 1;
    case fltk::DRAG:
      if(box_flag){
        box_x2 = X;
        box_y2 = Y;
        box_t2 = xpix2tick(X);
        box_k2 = Y/30;
      }
      if(color_flag && color_sel){
        color_sel->h = color_orig_h + (color_orig_x - event_x())/1.0;
        color_sel->v = color_orig_v + (color_orig_y - event_y())/100.0;
        color_sel->regen_colors();
        color_h = color_sel->h;
        color_v = color_sel->v;
        set_default_hsv_value(color_v);
      }
      if(insert_flag){
        insert_toffset = xpix2tick(X)/q_tick*q_tick + q_tick - insert_torig;
        if(insert_toffset <=0){
          insert_toffset -= q_tick;
        }
        insert_track = Y / 30;
      }
      else if(rresize_flag){
        rresize_toffset = xpix2tick(X)/128*128 - rresize_torig;
      }
      else if(lresize_flag){
        lresize_toffset = xpix2tick(X)/128*128 - lresize_torig;
      }
      else if(move_flag){
        move_toffset = quantize(xpix2tick(X)) - move_torig - move_offset;
        move_koffset = event_y() / 30 - move_korig;
      }
      else if(paste_flag){
        paste_t = quantize(xpix2tick(event_x()));
        paste_track = event_y() / 30;
      }
      redraw();
      return 1;
    case fltk::RELEASE:
      if(event_button()==1){
        if(box_flag){
          apply_box();
          box_flag = 0;
        }
        if(insert_flag){
          apply_insert();
          insert_flag = 0;
        }
        else if(move_flag){
          apply_move();
          move_flag = 0;
        }
        else if(rresize_flag){
          apply_rresize();
          rresize_flag = 0;
          if(last_handle){
            last_handle->lhandle = 0;
            last_handle->rhandle = 0;
          }
        }
        else if(lresize_flag){
          apply_lresize();
          lresize_flag = 0;
          if(last_handle){
            last_handle->lhandle = 0;
            last_handle->rhandle = 0;
          }
        }

        insert_flag=0;
        color_sel = NULL;
      }
      else if(event_button()==2){
        if(paste_flag){
          apply_paste();
        }
        paste_flag=0;
      }
      else if(event_button()==3){
        seqpat* over_s = over_seqpat();
        if(delete_flag && over_s){
          if(over_s->selected){
            apply_delete();
          }
        }
        delete_flag=0;
        //last_handle==NULL;
      }



      redraw();
      return 1;
    case fltk::MOVE:
      if(color_flag){break;}
      seqpat* s = over_seqpat();
      if(s){
        if(over_rhandle(s,X,Y)){s->rhandle = 1;}
        else{s->rhandle = 0;}
        if(over_lhandle(s,X,Y)){s->lhandle = 1;}
        else{s->lhandle = 0;}
        if(s != last_handle){
          if(last_handle){
            last_handle->rhandle = 0;
            last_handle->lhandle = 0;
          }
          last_handle = s;
        }
        redraw();
      }
      else if(last_handle){
        last_handle->rhandle = 0;
        last_handle->lhandle = 0;
        last_handle = NULL;
        redraw();
      }
      return 1;

  }
  return 0;
}

void Arranger::draw(){

  fltk::setfont(fltk::HELVETICA,8);

  fltk::setcolor(fltk::GRAY05);
  fltk::fillrect(0,0,w(),h());

  fltk::setcolor(fltk::GRAY20);
  int M = config.beats_per_measure;
  int I=0;
  for(int i=1; I<w(); i++){
    I = i*zoom*M/4;
    fltk::fillrect(I,0,1,h());
  }
  fltk::setcolor(fltk::GRAY50);
  int P = config.measures_per_phrase;
  if(P){
    I=0;
    for(int i=1; I<w(); i++){
      I = i*zoom*4*P*M/4/4;
      fltk::fillrect(I,0,1,h());
    }
  }


  if(insert_flag){
    fltk::setcolor(fltk::BLUE);
    int T1 = insert_torig;
    int T2 = T1 + insert_toffset;
    int tmp;
    if(T1>T2){SWAP(T1,T2);}
    int X = tick2xpix(T1)+1;
    int Y = insert_track*30;
    int W = tick2xpix(T2)-tick2xpix(T1) - 1;
    fltk::fillrect(X,Y,W,28);
  }

  if(move_flag){
    if(check_move_safety()){
      fltk::setcolor(fltk::MAGENTA);
    }
    else{
      fltk::setcolor(fltk::RED);
    }

    for(int i=0; i<tracks.size(); i++){
      seqpat* s = tracks[i]->head->next;
      while(s){
        if(s->selected){
          int X = tick2xpix(s->tick + move_toffset);
          int Y = (s->track + move_koffset)*30;
          int W = tick2xpix(s->dur);
          fltk::fillrect(X+1,Y+1,W-1,1);
          fltk::fillrect(X+1,Y+1,1,29-1);
          fltk::fillrect(X+1,Y+29-1,W-1,1);
          fltk::fillrect(X+W-1,Y+1,1,29-1);
        }
        s = s->next;
      }
    }
  }

  if(paste_flag){
    fltk::setcolor(fltk::GREEN);
    int X = tick2xpix(paste_t)+1;
    int Y = paste_track*zoom;
    int W = tick2xpix(main_sel->dur);
    fltk::fillrect(X,Y,W-1,1);
    fltk::fillrect(X,Y+28,W-1,1);
    fltk::fillrect(X,Y,1,28);
    fltk::fillrect(X+W-2,Y,1,28);
  }

  int tmp;
  if(box_flag){
    fltk::setcolor(fltk::GREEN);
    int X1,X2,Y1,Y2;
    X1 = box_x1;
    X2 = box_x2;
    Y1 = box_y1; 
    Y2 = box_y2;
    if(X1>X2){SWAP(X1,X2);}
    if(Y1>Y2){SWAP(Y1,Y2);}
    fltk::fillrect(X1,Y1,X2-X1,1);
    fltk::fillrect(X1,Y1,1,Y2-Y1);
    fltk::fillrect(X2,Y1,1,Y2-Y1);
    fltk::fillrect(X1,Y2,X2-X1,1);
  }

  //draw all seqpat
  seqpat* s;
  fltk::Color c;

  fltk::Color c1,c2,c3,cx;
  c1 = fltk::BLACK;

  for(int i=0; i<tracks.size(); i++){

    s = tracks[i]->head->next;
    while(s){

      pattern* p = s->p;

      get_outline_color(s,&c1,&c2,&c3,&cx);

      fltk::setcolor(c1);

      int R1 = lresize_flag&&s->selected ? lresize_toffset : 0;
      int R2 = rresize_flag&&s->selected ? rresize_toffset : 0;

      int T1 = s->tick+R1;
      int T2 = s->tick+s->dur+R2;

      if(T1 > T2){SWAP(T1,T2)};

      int X = tick2xpix(T1)+1;
      int Y = s->track * 30;
      int W = tick2xpix(T2)-tick2xpix(T1)-1;

      if(rresize_flag && s->selected && T1==T2){
        W = tick2xpix(128)-1;
      }
      if(lresize_flag && s->selected && T1==T2){
        W = tick2xpix(128)-1;
      }

      fillrect(X+1,Y+1,W-2,27);
      float a = 1.5f;


      fltk::setcolor(c2);
      fillrect(X+W-1,Y,1,29);
      fillrect(X,Y+28,W-1,1);

      fltk::setcolor(c3);
      fillrect(X,Y,1,28);
      fillrect(X,Y,W,1);

      fltk::push_clip(tick2xpix(T1),s->track*30,tick2xpix(T2-T1),30);

     if(s->rhandle && !rresize_flag){
        setcolor(cx);
        if(delete_flag){
          setcolor(fltk::color(128,0,0));
        }

        W = 5;
        X = tick2xpix(s->tick+s->dur) - W - 1;
        Y = s->track*30;
        addvertex(X+W,Y+28/2);
        addvertex(X,Y);
        addvertex(X,Y+28);
        fillpath();
      }

      if(s->lhandle && !lresize_flag){
        setcolor(cx);
        if(delete_flag){
          setcolor(fltk::color(128,0,0));
        }
        W = 5;
        X = tick2xpix(s->tick)+1;
        Y = s->track*30;
        addvertex(X,Y+28/2);
        addvertex(X+W,Y);
        addvertex(X+W,Y+28);
        fillpath();
      }

      fltk::setcolor(cx);


      mevent* e = s->p->events;
      while(e){
        if(e->tick >= s->dur){
          break;
        }
        if(e->type == MIDI_NOTE_ON){
          X = tick2xpix(e->tick) + tick2xpix(s->tick)+2;
          Y = s->track*30 + 27 - e->value1*27/127;
          W = tick2xpix(e->dur);
          if(W==0){W=1;}
          fillrect(X,Y,W,1);
        }
        e=e->next;
      }


      int total = s->layer_total();
      if(total > 1){
        fltk::setcolor(fltk::BLACK);
        int X = tick2xpix(s->tick);
        int Y = s->track * 30;
        int count = s->layer_index()+1;
        char buf[16];
        snprintf(buf,16,"%d / %d",count,total);
        fltk::drawtext(buf,X+2,Y+27);
      }


      fltk::pop_clip();

      s=s->next;
    }
  }
}

static int kludge = 4;//see the same kludge in pianoroll.cpp
void Arranger::layout(){
  if(kludge > 0){
    kludge--;
    return;
  }

/* this function has given some trouble so i will
document what it is supposed to do

it is called, ideally, when the scroller is dragged,
when zoom changes, the window is resized or when 
something changes in the arranger that means it needs 
to be resized.

this function is supposed to tell the timeline and
track info widgets to update their scroll state and
redraw to simulate being controlled by the scroller.

the arranger widget itself needs to resize itself
so that it covers

vertically, all track modules and scroll area, whichever is bigger
horizontally, all blocks (plus some) and scroll area, whichever is bigger

*/



  maxt = 0;
  for(int i=0; i<tracks.size(); i++){
    seqpat* s = tracks[i]->head->next;
    while(s){
      if(s->tick+s->dur > maxt){maxt=s->tick+s->dur;}
      s=s->next;
    }
  }




  int wp1 = ui->song_scroll->w();
  int wp2 = tick2xpix(maxt)+500;
  int hp1 = ui->song_scroll->h();
  int hp2 = tracks.size() * 30;
  int xp = ui->song_scroll->xposition();
  int yp = ui->song_scroll->yposition();


  ui->song_timeline->scroll = xp;
  ui->track_info->scroll = yp;

  int hp = hp1>hp2 ? hp1 : hp2;
  if(h() < hp){
    h(hp);
  }

  ui->track_info->scroll = yp;
  ui->track_info->redraw();

  int wp = wp1>wp2 ? wp1 : wp2;
  if(w() < wp){
    w(wp);
  }

//printf("relayout arranger %d %d\n",w(),h());

  ui->song_timeline->scroll = xp;
  ui->song_timeline->redraw();

//  yp_last = yp;
//  xp_last = xp;

}



seqpat* Arranger::over_seqpat(){
  int track = event_y() / 30;
  if(track >= tracks.size()){
    return NULL;
  }
  int tick = xpix2tick(event_x());
  seqpat* s = tfind<seqpat>(tracks[track]->head,tick);
  if(s){
    if(tick < s->tick+s->dur){
      return s;
    }
  }
  return NULL;
}


//true if over right handle of s
int Arranger::over_rhandle(seqpat* s, int X, int Y){
  int X1 = tick2xpix(s->tick);
  int X2 = X1 + tick2xpix(s->dur);
  int Y1 = s->track * 30 + 1;
  int Y2 = Y1 + 29;

  if(tick2xpix(s->dur) < 10){
    return 0;
  }

  return (Y > Y1 && Y < Y2 && X < X2 && X > X2 - 5);
}

//true if over left handle of s
int Arranger::over_lhandle(seqpat* s, int X, int Y){
  int X1 = tick2xpix(s->tick);
  int X2 = X1 + tick2xpix(s->dur);
  int Y1 = s->track * 30 + 1;
  int Y2 = Y1 + 29;

  if(tick2xpix(s->dur) < 10){
    return 0;
  }

  return (Y > Y1 && Y < Y2 && X < X1 + 5 + 1 && X > X1+1);
}

// 4=beats per measure, 128=ticks per beat, 30=width of measure in pixels
int Arranger::tick2xpix(int tick){
  return tick *zoom /(128*4);
}

int Arranger::xpix2tick(int xpix){
  return xpix * (128*4) /zoom;
}

int Arranger::quantize(int tick){
  return tick/q_tick * q_tick;
}


void Arranger::update(int pos){
  if(!is_backend_playing()){
    return;
  }
  int wp = ui->song_scroll->w();
  int xp = ui->song_scroll->xposition();
  int yp = ui->song_scroll->yposition();
  int X1 = tick2xpix(pos);
  int X2 = X1 - xp;
  if(X1 > w()-40){
    return;
  }
  if(X2 < 0){
    ui->song_scroll->scrollTo(X1-50<0?0:X1-50,yp);
  }
  if(X2 > wp-30){
    ui->song_scroll->scrollTo(X1-50,yp);
  }
}


void Arranger::unselect_all(){
  seqpat* s;
  for(int i=0; i<tracks.size(); i++){
    s = tracks[i]->head->next;
    while(s){
      if(s->selected==1){
        s->selected = 0;
      }
      s = s->next;
    }
  }
}

void Arranger::get_outline_color(seqpat* s, fltk::Color* c1, fltk::Color* c2, fltk::Color* c3, fltk::Color* cx){

  pattern* p = s->p;
  *c1 = fltk::color(p->r1, p->g1, p->b1);
  *cx = fltk::color(p->rx, p->gx, p->bx);

  int T1,T2;
  int tmp;
  seqpat* over_s = over_seqpat();
  if(delete_flag && s->selected){
    *c1 = fltk::color(255,0,0);
    *c2 = fltk::color(255,0,0);
    *c3 = fltk::color(255,0,0);
    return;
  }

  if(box_flag){
    T1=box_t1;
    T2=box_t2;
    int K1 = box_k1;
    int K2 = box_k2;
    int K = s->track;
    if(T1>T2){SWAP(T1,T2);}
    if(K1<K2){SWAP(K1,K2);}
    if(s->tick+s->dur > T1 && s->tick < T2 && K >= K2 && K <= K1){
      *c1 = fltk::color(0,255,0);
      *c2 = fltk::color(71,120,59);
      *c3 = fltk::color(108,229,75);
      return;
    }
  }

  if(s->selected){
    *c1 = fltk::color(255,255,0);
    *c2 = fltk::color(140,137,46);
    *c3 = fltk::color(232,255,37);
    *cx = fltk::color(128,128,0);
    return;
  }


  *c2 = fltk::color(p->r2,p->g2,p->b2);
  *c3 = fltk::color(p->r3,p->g3,p->b3);

}


void Arranger::apply_insert(){

  if(!check_insert_safety()){
    return;
  }

  int tmp;
  int T1 = insert_torig;
  int T2 = T1 + insert_toffset;
  if(T1>T2){SWAP(T1,T2);}

  Command* c=new CreateSeqpatBlank(insert_track,T1,T2-T1);
  set_undo(c);
  undo_push(1);

  if(T2>maxt){relayout();}
}

void Arranger::apply_box(){
  seqpat* s;
  int tmp;
  int T1=box_t1;
  int T2=box_t2;
  int K1 = box_k1;
  int K2 = box_k2;
  if(T1>T2){SWAP(T1,T2);}
  if(K1>K2){SWAP(K1,K2);}
  if(K1 < 0){K1 = 0;}
  if(K2 > tracks.size()-1){K2 = tracks.size()-1;}
  for(int i=K1; i<=K2; i++){
    s = tracks[i]->head->next;
    while(s){
      if(s->tick+s->dur > T1 && s->tick < T2){
        s->selected = 1;
      }
      s = s->next;
    }
  }
}


void Arranger::apply_delete(){
  Command* c;
  seqpat* s;
  seqpat* next;
  int N=0;
  for(int i=0; i<tracks.size(); i++){
    s = tracks[i]->head->next;
    while(s){
      next = s->next;
      if(s->selected){
        tracks[s->track]->modified = 1;
        c=new DeleteSeqpat(s);
        set_undo(c);
        N++;
      }
      s = next;
    }
  }
  undo_push(N);

  unmodify_and_unstick_tracks();
}


void Arranger::apply_move(){
  if(move_toffset==0 && move_koffset==0){
    return;
  }

  if(!check_move_safety()){
    return;
  }

  Command* c;
  seqpat* s;
  seqpat* next;
  int N=0;
  for(int i=0; i<tracks.size(); i++){
    s = tracks[i]->head->next;
    while(s){
      next = s->next;
      if(s->selected && s->modified == 0){
        int K = s->track + move_koffset;
        int T = s->tick + move_toffset;
        tracks[s->track]->modified = 1;
        tracks[K]->modified = 1;
        s->modified = 1;
        c=new MoveSeqpat(s,K,T);
        set_undo(c);
        N++;

        if(T+s->dur > maxt){relayout();}
      }
      s = next;
    }
  }
  undo_push(N);

  unmodify_blocks();
  unmodify_and_unstick_tracks();
}

void Arranger::apply_paste(){
  //safety check


  Command* c;

  c = new CreateSeqpat(paste_track,paste_t,main_sel,0);
  set_undo(c);
  undo_push(1);

  
}

void Arranger::apply_rresize(){
  if(rresize_toffset==0){
    return;
  }

  if(!check_resize_safety()){
    return;
  }

  Command* c;
  seqpat* s;
  seqpat* next;
  int tmp;
  int N=0;
  for(int i=0; i<tracks.size(); i++){
    s = tracks[i]->head->next;
    while(s){
      next = s->next;
      if(s->selected && s->modified == 0){
        tracks[i]->modified = 1;
        s->modified = 1;
        int T1 = s->tick;
        int T2 = s->tick + s->dur + rresize_toffset;
        if(T1 > T2){
          SWAP(T1,T2);
          seqpat* stmp = s->prev;
          //c=new ReverseSeqpat(s);
          //set_undo(c);
          s = stmp->next;
          c=new ResizeSeqpat(s,T2-T1);
          set_undo(c);
          s = stmp->next;
          c=new MoveSeqpat(s,s->track,T1);
          set_undo(c);
          N+=2;

          s = stmp->next;
          if(s->tick+s->dur > maxt){relayout();}
        }
        else{
          if(T1==T2){
            T2 = T1+128; //magic
          }
          c=new ResizeSeqpat(s,T2-T1);
          set_undo(c);
          N++;

          if(T2 > maxt){relayout();}
        }

      }
      s = next;
    }
  }
  undo_push(N);

  unmodify_blocks();
  unmodify_and_unstick_tracks();
}

void Arranger::apply_lresize(){
  if(lresize_toffset==0){
    return;
  }

  if(!check_resize_safety()){
    return;
  }

  Command* c;
  seqpat* s;
  seqpat* next;
  int tmp;
  int N=0;
  for(int i=0; i<tracks.size(); i++){
    s = tracks[i]->head->next;
    while(s){
      next = s->next;
      if(s->selected && s->modified == 0){
        tracks[i]->modified = 1;
        s->modified = 1;
        int T1 = s->tick + lresize_toffset;
        int T2 = s->tick + s->dur;
        if(T1 > T2){
          SWAP(T1,T2);
          seqpat* stmp = s->prev;
          //c=new ReverseSeqpat(s);
          //set_undo(c);
          s = stmp->next;
          c=new ResizeSeqpat(s,T2-T1);
          set_undo(c);
          s = stmp->next;
          c=new MoveSeqpat(s,s->track,T1);
          set_undo(c);
          N+=2;

          s = stmp->next;
          if(s->tick+s->dur > maxt){relayout();}
        }
        else{
          if(T1==T2){
            T2 = T1+128; //magic
          }
          seqpat* stmp = s->prev;
          c=new MoveSeqpat(s,s->track,T1);
          set_undo(c);
          s = stmp->next;
          c=new ResizeSeqpat(s,T2-T1);
          set_undo(c);
          N+=2;

          s = stmp->next;
          if(s->tick+s->dur>maxt){relayout();}
        }

      }
      s = next;
    }
  }
  undo_push(N);

  unmodify_blocks();
  unmodify_and_unstick_tracks();
}



int collision_test(int t11, int t12, int t21, int t22){
  return !((t11 < t21 && t12 <= t21)  ||
           (t11 >= t22 && t12 > t22)) ? 1 : 0;
}

int Arranger::check_move_safety(){
  seqpat* s;
  seqpat* ptr;

  for(int i=0; i<tracks.size(); i++){
    s = tracks[i]->head->next;
    while(s){
      if(s->selected){
        if(i+move_koffset < 0 || i+move_koffset > tracks.size()-1 ||
           s->tick + move_toffset < 0){
          return 0;
        }
        ptr = tracks[i+move_koffset]->head->next;
        while(ptr){
          if(ptr == s){
            ptr=ptr->next; continue;
          }
          if(collision_test(s->tick+move_toffset,s->tick+s->dur+move_toffset,ptr->tick,ptr->tick+ptr->dur) ){
            if(!ptr->selected){
              return 0;
            }
          }
          ptr = ptr->next;
        }
      }
      s = s->next;
    }
  }

  return 1;
}

int Arranger::check_insert_safety(){
  seqpat* s;

  int T1 = insert_torig;
  int T2 = T1 + insert_toffset;
  int tmp;

  if(T1>T2){SWAP(T1,T2);}

  if(T1 < 0){
    return 0;
  }
  if(insert_track > tracks.size()-1){
    return 0;
  }
  if(tracks[insert_track]==NULL){
    return 0;
  }

  s = tracks[insert_track]->head->next;

  while(s){
    if(collision_test(T1,T2,s->tick,s->tick+s->dur)){
      return 0;
    }
    s = s->next;
  }

  return 1;
}

int Arranger::check_resize_safety(){
  seqpat* s;
  seqpat* ptr;

  int T1,T2;
  int S1,S2;
  int tmp;

  for(int i=0; i<tracks.size(); i++){
    s = tracks[i]->head->next;
    while(s){
      if(!s->selected){
        s = s->next; continue;
      }

      if(rresize_flag){
        T1 = s->tick;
        T2 = s->tick + s->dur + rresize_toffset;
      }
      else if(lresize_flag){
        T1 = s->tick + lresize_toffset;
        T2 = s->tick + s->dur;
      }
      if(T1>T2){SWAP(T1,T2);}

      if(T1 < 0){
        return 0;
      }
      ptr = tracks[s->track]->head->next;
      while(ptr){
        if(ptr == s){
          ptr=ptr->next; continue;
        }

        S1 = ptr->tick;
        S2 = ptr->tick + ptr->dur;
        if(ptr->selected){
          if(rresize_flag){
            S2 += rresize_toffset;
          }
          else if(lresize_flag){
            S1 += lresize_toffset;
          }
        }

        if(collision_test(T1,T2,S1,S2)){
          return 0;
        }
        ptr = ptr->next;
      }

      s = s->next;
    }

  }
  return 1;
}


int Arranger::check_paste_safety(){
  return 1;
}



