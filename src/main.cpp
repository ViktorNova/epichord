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

#include <fltk/run.h>
#include <fltk/Group.h>
#include <fltk/Button.h>
#include <fltk/Input.h>
#include <fltk/ValueInput.h>
#include <fltk/TextDisplay.h>


#include "ui.h"
#include "backend.h"
#include "uihelper.h"
#include "saveload.h"

UI* ui;

int main(int argc, char* argv[]){

  ui = new UI();

  init_gui();

  ui->arranger->take_focus();

  load_config();

  if(init_backend(&argc, &argv) < 0){
    return 1;
  }

  if(argc > 1){
    if(loadsmf(argv[1])<0){
      if(load(argv[1])<0){
        reset_song();
      }
    }
  }
  else{
    reset_song();
  }

  



  int ret = fltk::run();

  shutdown_backend();

  delete ui;

  return ret;

}

