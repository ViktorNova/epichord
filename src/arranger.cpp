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
  move_start = 0;
  paste_flag = 0;
  main_sel = NULL;

  zoom = 30;
  zoom_n = 4;

  q_tick = 128*4;

  xp_last = 0;
  yp_last = 0;
}

int Arranger::handle(int event){
  Command* c;

  int X = event_x();
  int Y = event_y();

  switch(event){
    case fltk::FOCUS:
      return 1;
    case fltk::KEYUP:
      //might want to consume release control c, etc
      return 0;
    case fltk::SHORTCUT:
      if(event_state(CTRL) && event_key()=='c'){
        //printf("arranger copy\n");
        return 1;
      }
      if(event_state(CTRL) && event_key()=='v'){
        //printf("arranger paste\n");
        return 1;
      }
      if(event_state(CTRL) && event_key()=='z'){
        //printf("arranger undo\n");
        return 1;
      }
      if(zoom_out_key(event_key(),event_state())){
        //printf("arranger zoom out\n");
        if(zoom_n > 1){
          zoom_n--;
          zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->update(get_play_position());
          ui->song_timeline->redraw();
        }
        redraw();
        return 1;
      }
      if(zoom_in_key(event_key(),event_state())){
        //printf("arranger zoom in\n");
        if(zoom_n < 8){
          zoom_n++;
          zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->update(get_play_position());
          ui->song_timeline->redraw();
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
          if(event_state()&fltk::SHIFT){//begin box
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
            new_left_t = quantize(xpix2tick(event_x()));
            new_orig_t = new_left_t;
            new_track = event_y() / 30;
            new_right_t = new_left_t + quantize(q_tick);
          }
        }
        else{
          if(!(event_state()&fltk::SHIFT)){
            unselect_all();
          }
          s->selected = 1;
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
          if(over_handle(main_sel)){//begin resize
          }
          else{//begin move
            move_start = 1;
            move_t = quantize(main_sel->tick);
            move_offset = quantize(xpix2tick(event_x())) - move_t;
            move_track = event_y() / 30;
          }
        }
      }
      else if(event_button()==2){//middle mouse
        if(main_sel){
          paste_flag = 1;
          paste_t = quantize(xpix2tick(event_x()));
          paste_track = event_y() / 30;
        }
        seqpat* s = over_seqpat();
        if(color_flag && s){
          s->p->h = color_h;
          s->p->v = color_v;
          s->p->regen_colors();
          redraw();
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
      if(move_start){
        move_flag = 1;
      }
      if(insert_flag){
        //new_right_t = quantize(xpix2tick(event_x())) + quantize(q_tick);
        new_right_t = quantize(xpix2tick(event_x()+tick2xpix(q_tick)));
        if(new_right_t <= new_orig_t){
          new_left_t = new_right_t - quantize(q_tick);
          new_right_t = new_orig_t;
        }
        else{
          new_left_t = new_orig_t;
        }
        new_track = Y / 30;
      }
      else if(move_flag){
        //printf("moving something\n");
        move_t = quantize(xpix2tick(event_x())) - move_offset;
        move_track = event_y() / 30;
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
        if(insert_flag && new_track < tracks.size()){
          if(tracks[new_track]->alive){
           c=new CreateSeqpatBlank(new_track,new_left_t,new_right_t-new_left_t);
            set_undo(c);
            undo_push(1);
          }
        }
        else if(move_flag && move_track < tracks.size()){
          c = new MoveSeqpat(main_sel,move_track,move_t);
          set_undo(c);
          undo_push(1);
        }
        move_start=0;
        move_flag=0;
        insert_flag=0;
        color_sel = NULL;
      }
      else if(event_button()==2){
        if(paste_flag && paste_track < tracks.size()){
          if(!config.alwayscopy){
            c = new CreateSeqpat(paste_track,paste_t,main_sel,0);
          }
          else{
            c = new CreateSeqpat(paste_track,paste_t,main_sel,1);
          }
          set_undo(c);
          undo_push(1);
        }
        paste_flag=0;
      }
      else if(event_button()==3){
        if(delete_flag && over_seqpat() == delete_sel){
          //here we need more branches for deleting the entire selection
          c=new DeleteSeqpat(delete_sel);
          set_undo(c);
          undo_push(1);
        }
        delete_flag = 0;
      }

      redraw();
      return 1;
  }
  return 0;
}

void Arranger::draw(){

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
    fltk::setcolor(fltk::RED);
    int X = tick2xpix(new_left_t)+1;
    int Y = new_track*30;
    int W = tick2xpix(new_right_t)-X;
    fltk::fillrect(X,Y,W,28);
  }

  if(move_flag){
    fltk::setcolor(fltk::RED);
    int X = tick2xpix(move_t)+1;
    int Y = move_track*30;
    int W = tick2xpix(main_sel->dur);
    fltk::fillrect(X,Y,W-1,1);
    fltk::fillrect(X,Y+28,W-1,1);
    fltk::fillrect(X,Y,1,28);
    fltk::fillrect(X+W-2,Y,1,28);
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

  for(int i=0; i<tracks.size(); i++){

    s = tracks[i]->head->next;
    while(s){

      pattern* p = s->p;

      get_outline_color(s,&c1,&c2,&c3,&cx);

      fltk::setcolor(c1);
      int X = tick2xpix(s->tick)+1;
      int Y = s->track * 30;
      int W = tick2xpix(s->tick+s->dur) - X;

      fillrect(X+1,Y+1,W-2,27);
      float a = 1.5f;


      fltk::setcolor(c2);
      fillrect(X+W-1,Y,1,29);
      fillrect(X,Y+28,W-1,1);

      fltk::setcolor(c3);
      fillrect(X,Y,1,28);
      fillrect(X,Y,W,1);

      fltk::push_clip(tick2xpix(s->tick),s->track*30,tick2xpix(s->dur),30);

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

  int wp = ui->song_scroll->w();
  if(wp > w()){
    w(wp+120);
  }

  int hp = ui->song_scroll->h();
  if(hp > h()){
    h(hp);
  }
  else{
    h(16*30);
  }

  int xp = ui->song_scroll->xposition();
  int yp = ui->song_scroll->yposition();
  ui->song_timeline->scroll = xp;
  ui->track_info->scroll = yp;

  if(xp_last != xp){
    ui->song_timeline->redraw();
  }
  if(yp_last != yp){
    ui->track_info->redraw();
  }

  yp_last = yp;
  xp_last = xp;

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

//0 not over, 1 over handle, 2 over middle
int Arranger::over_handle(seqpat* s){
  return 0;
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
  if(delete_flag){
    if(s->selected){
      *c2 = fltk::color(120,60,58);
      *c3 = fltk::color(225,131,109);
      return;
    }
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

void Arranger::apply_box(){
  seqpat* s;
  int tmp;
  int T1=box_t1;
  int T2=box_t2;
  int K1 = box_k1;
  int K2 = box_k2;
  if(T1>T2){SWAP(T1,T2);}
  if(K1>K2){SWAP(K1,K2);}
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
