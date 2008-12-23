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

PianoRoll::PianoRoll(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {
  wkeyh = 12;
  bkeyh = 7;
  cur_seqpat = NULL;
  main_sel = NULL;

  zoom = 15;
  zoom_n = 3;

  q_tick = 32;

  xp_last = 0;
  yp_last = 0;
}

int PianoRoll::handle(int event){
  Command* c;
  pattern* p;

  switch(event){
    case fltk::FOCUS:
      return 1;
    case fltk::SHORTCUT:
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
      if(event_button()==1){//left mouse
        if(over_note()==NULL){//new note init
          new_drag = 1;
          new_left_t = quantize(xpix2tick(event_x()));
          new_orig_t = new_left_t;
          new_note = ypix2note(event_y(),1);
          new_right_t = new_left_t + q_tick;

          last_note = new_note;
          if(config.playinsert){
            ui->keyboard->play_note(last_note,0);
          }
        }
        else{
          //if shift, add to selection
          main_sel = over_note();
          if(over_handle(main_sel)){//begin resize or resize move
          }
          else{//begin move
            move_flag = 1;
            move_t = quantize(main_sel->tick);
            move_offset = quantize(xpix2tick(event_x())) - move_t;
            //move_track = event_y() / 30;
            move_note = ypix2note(event_y(),1);

            last_note = move_note;
            if(config.playmove){
              ui->keyboard->play_note(last_note,0);
            }
          }
          redraw();
        }
      }
      else if(event_button()==2){//middle mouse
        //button initiates paste
      }
      else if(event_button()==3){//right mouse
        if(over_note()==NULL){//begin box

        }
        else{//set up for deletion
          delete_flag = 1;
          main_sel = over_note();
        }
      }
      redraw();
      return 1;
    case fltk::DRAG:
      if(new_drag){
        new_right_t = quantize(xpix2tick(event_x())+q_tick);
        if(new_right_t <= new_orig_t){
          new_left_t = new_right_t - q_tick;
          new_right_t = new_orig_t;
        }
        else{
          new_left_t = new_orig_t;
        }
        new_note = ypix2note(event_y(),1);
        if(new_note != last_note){
          if(config.playinsert){//play on insert
            ui->keyboard->release_note(last_note,0);
            ui->keyboard->play_note(new_note,0);
          }
          last_note = new_note;
        }
        redraw();
        return 1;
      }
      else if(move_flag){
        move_t = quantize(xpix2tick(event_x())) - move_offset;
        move_note = ypix2note(event_y(),1);
        if(move_note != last_note){
          if(config.playmove){//play on move
            ui->keyboard->release_note(last_note,0);
            ui->keyboard->play_note(move_note,0);
          }
          last_note = move_note;
        }
        redraw();
        return 1;
      }
    case fltk::RELEASE:
      if(event_button()==1){
        if(new_drag && new_note < 128 && new_note >= 0){
          p = cur_seqpat->p;
          c=new CreateNote(p,new_note,127,new_left_t,new_right_t-new_left_t);
          set_undo(c);
          undo_push(1);
          ui->keyboard->release_note(new_note,0);
          ui->keyboard->redraw();
          ui->event_edit->has[0]=1;
          ui->event_edit->has[1]=1;
          ui->event_edit->redraw();
          ui->event_menu->redraw();
        }
        else if(move_flag && move_note < 128 && move_note >= 0){
          int play_pos = get_play_position();
          mevent* e = main_sel;
          track* tr = tracks[cur_seqpat->track];
          if(play_pos > e->tick && play_pos < e->tick + e->dur){
            midi_note_off(e->value1,tr->chan,tr->port);
          }
          int old_note = main_sel->value1;
          c=new MoveNote(cur_seqpat->p,main_sel,move_t,move_note);
          set_undo(c);
          undo_push(1);

          int cur_chan = tracks[cur_seqpat->track]->chan;
          int cur_port = tracks[cur_seqpat->track]->port;
          midi_note_off(old_note,cur_chan,cur_port);

          ui->keyboard->release_note(move_note,0);
          ui->keyboard->redraw();
          ui->event_edit->redraw();
        }
        new_drag=0;
        move_flag=0;
      }
      if(event_button()==3){
        if(delete_flag && over_note() == main_sel){
          //here we need more branches for deleting the entire selection
          c=new DeleteNote(cur_seqpat->p, main_sel);
          set_undo(c);
          undo_push(1);

          ui->event_edit->redraw();
        }
        delete_flag = 0;
      }
      redraw();

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

  if(new_drag){
    fltk::setcolor(fltk::BLUE);
    int X = tick2xpix(new_left_t)+1;
    int Y = note2ypix(new_note);
    int W = tick2xpix(new_right_t) - X;
    fltk::fillrect(X,Y,W,11);
  }

  if(move_flag){
    fltk::setcolor(fltk::color(255,0,0));
    int X = tick2xpix(move_t)+1;
    int Y = note2ypix(move_note);
    int W = tick2xpix(main_sel->dur);
    fltk::fillrect(X,Y,W-1,1);
    fltk::fillrect(X,Y+11,W-1,1);
    fltk::fillrect(X,Y,1,11);
    fltk::fillrect(X+W-2,Y,1,11);
  }

  //draw all notes
  mevent* e = cur_seqpat->p->events->next;

  int c1,c2,c3;

  while(e){
    if(e->type == MIDI_NOTE_ON){
      //fltk::fillrect(tick2xpix(e->tick),note2ypix(e->value),e->dur,11);
      int X = tick2xpix(e->tick) + 1;
      int Y = note2ypix(e->value1);
      int W = tick2xpix(e->tick+e->dur) - X;
      if(e == main_sel){
        c1 = 230;
        c2 = 230;
        c3 = 0;
      }
      else{
        c1 = 165;
        c2 = 75;
        c3 = 229;
      }
      fltk::setcolor(fltk::color(c1,c2,c3));
      fltk::fillrect(X+1,Y+1,W-1,10);

      fltk::setcolor(fltk::color(c1*3/4,c2*3/4,c3*3/4));
      fltk::fillrect(X,Y+11,W,1);
      fltk::fillrect(X+W-1,Y+1,1,11);

      fltk::setcolor(fltk::color(c1*3/4+64,c2*3/4+64,c3*3/4+64));
      fltk::fillrect(X,Y,W,1);
      fltk::fillrect(X,Y,1,11);
    }
    e=e->next;
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

int PianoRoll::over_handle(mevent* e){
  return 0;
}



void PianoRoll::update(int pos){
  if(!is_backend_playing()){
    return;
  }
  int wp = ui->pattern_scroll->w();
  int xp = ui->pattern_scroll->xposition();
  int yp = ui->pattern_scroll->yposition();
  int X1 = tick2xpix(pos);
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

