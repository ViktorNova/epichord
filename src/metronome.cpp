#include <stdio.h>
#include <math.h>

#include <fltk/Widget.h>
#include <fltk/events.h>
#include <fltk/draw.h>

#include "metronome.h"

using namespace fltk;

Metronome::Metronome(int x, int y, int w, int h, const char* label = 0) : fltk::Widget(x, y, w, h, label) {
  N = 4;
  n = 2;

  last_beat = 0;
  plus = 0;

  r=255; g=0; b=0;

  update(0);
}

int Metronome::handle(int event){
  return 0;
}

void Metronome::draw(){
  draw_box();

  //setcolor(fltk::color(r,g,b));
  //fillrect(2,2,w()-4,h()-4);

  setcolor(fltk::WHITE);
  int X = last_beat%N*(w()-4)/N+2;
  int W = (w()-4)/N;
  int H = h()-4;
  fillrect(X,2,W,h()-4);

  //int W2 = W/2;
  //int H2 = H/2;
  //setcolor(fltk::color(r,g,b));
  //fillrect(X+W/2-W2/2,h()/2-H2/2,W2,H2);

  //setcolor(fltk::BLACK);
  //int C = (getascent()-getdescent())/2;
  //drawtext(label(),X+W/2-getwidth(label())/2,h()/2+C);

}

//div N is the greatest divisor of N less than or equal to sqrt(N)
int div(int N){
  int d = 0;
  for(int i=1; i<=sqrt(N); i++){
    if(N%i==0){
      d=i;
    }
  }
  if(d==1){
    //prime
  }
  return d;
}

void Metronome::update(int tick){
  int now_beat = tick/128;

  if(now_beat != last_beat){
    int step = last_beat - now_beat;
    last_beat = now_beat;

    char buf[16];
    snprintf(buf,16,"%d",now_beat%N + plus);
    copy_label(buf);

    if(now_beat%N == 0){
      r=255; g=0; b=0;
    }
    else if(now_beat%n == 0){
      r=128; g=0; b=0;
    }
    else{
      r=0; g=0; b=0;
    }

    redraw();
  }
}

void Metronome::set_N(int zN){
  N = zN;
  n = div(zN);
}

