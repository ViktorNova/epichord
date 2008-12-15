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



extern UI* ui;

extern std::vector<track*> tracks;

using namespace fltk;

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
}

int Arranger::handle(int event){
  Command* c;

  /* if recording, and we restrict manual changes, then we can
     easily set up an single undo for the entire recording

     if we allow manual changes during recording, then undo becomes
     granular.

     provide the user with the choice.

  */

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
      if(event_key() == '-'){
        //printf("arranger zoom out\n");
        if(zoom_n > 1){
          zoom_n--;
          zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->redraw();
        }
        redraw();
        return 1;
      }
      if(event_key() == '='){
        //printf("arranger zoom in\n");
        if(zoom_n < 8){
          zoom_n++;
          zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->zoom = 30*(1<<zoom_n)/16;
          ui->song_timeline->redraw();
        }
        redraw();
        return 1;
      }
      return 0;
    case fltk::PUSH:
      take_focus();
      if(event_button()==1){//left mouse
        if(over_seqpat()==NULL){//begin pattern creation
          new_drag = 1;
          new_left_t = quantize(xpix2tick(event_x()));
          new_orig_t = new_left_t;
          new_track = event_y() / 30;
          new_right_t = new_left_t + q_tick;
        }
        else{
          //if shift, add to selection
          main_sel = over_seqpat();
          if(fltk::event_clicks() > 0){//'double click'
            ui->piano_roll->load(main_sel);
            ui->pattern_scroll->scrollTo(main_sel->scrollx,main_sel->scrolly);
            ui->keyboard->cur_port = tracks[main_sel->track]->port;
            ui->keyboard->cur_chan = tracks[main_sel->track]->chan;
            ui->track_info->set_rec(main_sel->track);
            ui->song_edit->hide();
            ui->song_edit->deactivate();
            ui->pattern_edit->take_focus();
            ui->pattern_edit->show();
            return 1;
          }
          if(over_handle(main_sel)){//begin resize or resize move
          }
          else{//begin move
            move_start = 1;
            move_t = main_sel->tick;
            move_offset = quantize(xpix2tick(event_x())) - move_t;
            move_track = event_y() / 30;
          }
        }
      }
      else if(event_button()==2){//middle mouse
        paste_flag = 1;
        paste_t = quantize(xpix2tick(event_x()));
        paste_track = event_y() / 30;
      }
      else if(event_button()==3){//right mouse
        if(over_seqpat()==NULL){//begin box
          delete_sel = NULL;
          main_sel = NULL;
        }
        else{//set up for deletion
          delete_flag = 1;
          delete_sel = over_seqpat();
        }
      }
      redraw();
      return 1;
    case fltk::DRAG:
      if(move_start){
        move_flag = 1;
      }
      if(new_drag){
        new_right_t = quantize(xpix2tick(event_x())+q_tick);
        if(new_right_t <= new_orig_t){
          new_left_t = new_right_t - q_tick;
          new_right_t = new_orig_t;
        }
        else{
          new_left_t = new_orig_t;
        }
        new_track = event_y() / 30;

        redraw();
        return 1;
      }
      else if(move_flag){
        //printf("moving something\n");
        move_t = quantize(xpix2tick(event_x())) - move_offset;
        move_track = event_y() / 30;
        redraw();
        return 1;
      }
      else if(paste_flag){
        paste_t = quantize(xpix2tick(event_x()));
        paste_track = event_y() / 30;
        redraw();
        return 1;
      }
      break;
    case fltk::RELEASE:
      if(event_button()==1){
        if(new_drag && new_track < tracks.size()){
          if(tracks[new_track]->alive){ //if track is active
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
        new_drag=0;
      }
      else if(event_button()==2){
        if(paste_flag && paste_track < tracks.size()){
          c = new CreateSeqpat(paste_track,paste_t,main_sel);
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
  for(int i=zoom; i<w(); i+=zoom){
    fltk::drawline(i,0,i,h());
  }
  fltk::setcolor(fltk::GRAY50);
  for(int i=zoom*4; i<w(); i+=zoom*4){
    fltk::drawline(i,0,i,h());
  }

  if(new_drag){
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

  //draw all seqpat
  seqpat* s;
  fltk::Color c;

  int c11,c12,c13;
  int c21,c22,c23;
  int c31,c32,c33;

  for(int i=0; i<tracks.size(); i++){

    s = tracks[i]->head->next;
    while(s){

      c11 = s->r1;
      c12 = s->g1;
      c13 = s->b1;
      if(s!=main_sel){
        c21 = s->r2;
        c22 = s->g2;
        c23 = s->b2;
        c31 = s->r3;
        c32 = s->g3;
        c33 = s->b3;
      }
      else{
        c21 = 128;
        c22 = 255;
        c23 = 128;
        c31 = 128;
        c32 = 255;
        c33 = 128;
      }


      fltk::setcolor(fltk::color(c11,c12,c13));
      int X = tick2xpix(s->tick)+1;
      int Y = s->track * 30;
      int W = tick2xpix(s->tick+s->dur) - X;

      fillrect(X+1,Y+1,W-2,27);
      float a = 1.5f;


      fltk::setcolor(fltk::color(c21,c22,c23));
      fillrect(X+W-1,Y,1,29);
      fillrect(X,Y+28,W-1,1);


      fltk::setcolor(fltk::color(c31,c32,c33));
      fillrect(X,Y,1,28);
      fillrect(X,Y,W,1);

      fltk::push_clip(tick2xpix(s->tick),s->track*30,tick2xpix(s->dur),30);

      fltk::setcolor(fltk::color(s->rx,s->gx,s->bx));

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

