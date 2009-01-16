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
#include <math.h>
#include <vector>

#include <fltk/TextDisplay.h>
#include <fltk/TextBuffer.h>

#include "seq.h"
#include "backend.h"



#include "util.h"




extern std::vector<track*> tracks;

void load_text(fltk::TextDisplay* o, const char* filename){
  fltk::TextBuffer* T = new fltk::TextBuffer();
  //printf("loading file %s\n",filename);
  T->loadfile(filename);
  o->buffer(T);
}


void hsv_to_rgb(float h, float s, float v, unsigned char* r, unsigned char* g, unsigned char* b){

  float lh = h - 360*floor(h/360);

  float R,G,B;

  float V = v;
  float S = s;

  if(lh < 60){
    R = V;
    G = lh*V*S/60 + V*(1-S);
    B = V*(1-S);
  }
  else if(lh < 120){
    R = -(lh-60)*V*S/60 + V;
    G = V;
    B = V*(1-S);
  }
  else if(lh < 180){
    R = V*(1-S);
    G = V;
    B = (lh-120)*V*S/60 + V*(1-S);
  }
  else if(lh < 240){
    R = V*(1-S);
    G = -(lh-180)*V*S/60 + V;
    B = V;
  }
  else if(lh < 300){
    R = (lh-240)*V*S/60 + V*(1-S);
    G = V*(1-S);
    B = V;
  }
  else{
    R = V;
    G = V*(1-S);
    B = -(lh-300)*V*S/60 + V;
  } 

  *r = (unsigned char)(255*R);
  *g = (unsigned char)(255*G);
  *b = (unsigned char)(255*B);

}


int ypix2note(int ypix, int black){
  int udy = 900 - ypix + 1;
  int white = udy/12;
  int note = 2*white - white/7 - (white+4)/7;

  if(black){
    if(udy%12<4 && white%7!=0 && (white+4)%7!=0){note--;}
    else if(udy%12>8 && (white+1)%7!=0 && (white+5)%7!=0){note++;}
  }

  return note;
}

int note2ypix(int note, int* black){
  int key = (note + (note+7)/12 + note/12)/2;

  if((note + (note+7)/12 + note/12)%2 == 0){//white key
    *black=0;
    return 900 - key*12 - 12;
  }
  else{//black key
    *black=1;
    return 900 - key*12 - 12 - 3;
  }

}

void unmodify_blocks(){
  seqpat* s;
  for(int i=0; i<tracks.size(); i++){
    s = tracks[i]->head->next;
    while(s){
      s->modified = 0;
      s = s->next;
    }
  }
}

int unmodify_and_unstick_tracks(){
  for(int i=0; i<tracks.size(); i++){
    if(tracks[i]->modified){
      midi_track_off(i);
      tracks[i]->restate();
      tracks[i]->modified = 0;
    }
  }
}


float randf(float l, float r){
  float L,R;
  if(l > r){
    L=r;
    R=l;
  }
  else{
    L=l;
    R=r;
  }
  float M = (rand()*1.0)/RAND_MAX;
  float ans = M*(R-L) + L;
  return ans;
}

