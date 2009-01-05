// generated by Fast Light User Interface Designer (fluid) version 2.1000

#ifndef ui_h
#define ui_h
#include <fltk/Window.h>
#include <unistd.h>
#include <stdio.h>
#include "seq.h"
#include "trackmodule.h"
#include <fltk/Group.h>
#include "trackinfo.h"
#include <fltk/Button.h>
#include "arranger.h"
#include <fltk/Scrollbar.h>
#include "timeline.h"
#include <fltk/ScrollGroup.h>
#include "pianoroll.h"
#include "eventedit.h"
#include "eventmenu.h"
#include "trackselect.h"
#include "sampleview.h"
#include "keyboard.h"
#include <metronome.h>
#include "saveload.h"
#include <fltk/TabGroup.h>
#include <fltk/ValueInput.h>
#include <fltk/ValueOutput.h>
#include <fltk/ThumbWheel.h>
#include <fltk/CheckButton.h>
#include <fltk/Choice.h>
#include <fltk/Item.h>
#include <fltk/Input.h>
#include <fltk/TextEditor.h>
#include <fltk/TextDisplay.h>
#include <fltk/InvisibleBox.h>

class UI  {
public:
  UI();
  fltk::Window *main_window;
private:
  inline void cb_main_window_i(fltk::Window*, void*);
  static void cb_main_window(fltk::Window*, void*);
public:
      fltk::Group *song_edit;
          TrackInfo *track_info;
private:
          inline void cb_line_i(fltk::Button*, void*);
          static void cb_line(fltk::Button*, void*);
public:
          Timeline *song_timeline;
        fltk::Group *song_scrollgroup;
          Arranger *arranger;
          fltk::Scrollbar *song_vscroll;
          fltk::Scrollbar *song_hscroll;
      fltk::Group *pattern_edit;
            Timeline *pattern_timeline;
          fltk::ScrollGroup *pattern_scroll;
            PianoRoll *piano_roll;
            EventEdit *event_edit;
            EventMenu *event_menu;
            fltk::Button *event_menu_button;
private:
            inline void cb_event_menu_button_i(fltk::Button*, void*);
            static void cb_event_menu_button(fltk::Button*, void*);
            inline void cb_L_i(fltk::Button*, void*);
            static void cb_L(fltk::Button*, void*);
            inline void cb_C_i(fltk::Button*, void*);
            static void cb_C(fltk::Button*, void*);
            inline void cb_X_i(fltk::Button*, void*);
            static void cb_X(fltk::Button*, void*);
            inline void cb_Z_i(fltk::Button*, void*);
            static void cb_Z(fltk::Button*, void*);
public:
            TrackSelect *track_select;
            SampleView *sample_view;
            Keyboard *keyboard;
private:
          inline void cb__i(fltk::Button*, void*);
          static void cb_(fltk::Button*, void*);
public:
      fltk::Button *play_button;
private:
      inline void cb_play_button_i(fltk::Button*, void*);
      static void cb_play_button(fltk::Button*, void*);
public:
      fltk::Button *stop_button;
private:
      inline void cb_stop_button_i(fltk::Button*, void*);
      static void cb_stop_button(fltk::Button*, void*);
public:
      fltk::Button *record_button;
private:
      inline void cb_record_button_i(fltk::Button*, void*);
      static void cb_record_button(fltk::Button*, void*);
public:
      Metronome *metronome;
      fltk::Group *pattern_buttons;
        fltk::Button *qbutton4;
private:
        inline void cb_qbutton4_i(fltk::Button*, void*);
        static void cb_qbutton4(fltk::Button*, void*);
public:
        fltk::Button *qbutton8;
private:
        inline void cb_qbutton8_i(fltk::Button*, void*);
        static void cb_qbutton8(fltk::Button*, void*);
public:
        fltk::Button *qbutton16;
private:
        inline void cb_qbutton16_i(fltk::Button*, void*);
        static void cb_qbutton16(fltk::Button*, void*);
public:
        fltk::Button *qbutton32;
private:
        inline void cb_qbutton32_i(fltk::Button*, void*);
        static void cb_qbutton32(fltk::Button*, void*);
public:
        fltk::Button *qbutton64;
private:
        inline void cb_qbutton64_i(fltk::Button*, void*);
        static void cb_qbutton64(fltk::Button*, void*);
public:
        fltk::Button *qbutton128;
private:
        inline void cb_qbutton128_i(fltk::Button*, void*);
        static void cb_qbutton128(fltk::Button*, void*);
public:
        fltk::Button *qbutton0;
private:
        inline void cb_qbutton0_i(fltk::Button*, void*);
        static void cb_qbutton0(fltk::Button*, void*);
public:
        fltk::Button *quant1_button;
        fltk::Button *quant0_button;
        fltk::Button *tool_button;
private:
        inline void cb_tool_button_i(fltk::Button*, void*);
        static void cb_tool_button(fltk::Button*, void*);
public:
      fltk::Group *song_buttons;
        fltk::Button *color_toggle;
private:
        inline void cb_color_toggle_i(fltk::Button*, void*);
        static void cb_color_toggle(fltk::Button*, void*);
public:
        fltk::Button *unclone_button;
        fltk::Button *join_button;
        fltk::Button *split_button;
      fltk::Button *loop_toggle;
private:
      inline void cb_loop_toggle_i(fltk::Button*, void*);
      static void cb_loop_toggle(fltk::Button*, void*);
public:
      fltk::Button *config_button;
private:
      inline void cb_config_button_i(fltk::Button*, void*);
      static void cb_config_button(fltk::Button*, void*);
public:
      fltk::Button *scope_button;
private:
      inline void cb_scope_button_i(fltk::Button*, void*);
      static void cb_scope_button(fltk::Button*, void*);
public:
      fltk::Button *file_button;
private:
      inline void cb_file_button_i(fltk::Button*, void*);
      static void cb_file_button(fltk::Button*, void*);
public:
      fltk::Button *help_button;
private:
      inline void cb_help_button_i(fltk::Button*, void*);
      static void cb_help_button(fltk::Button*, void*);
public:
  fltk::Window *config_window;
        fltk::ValueInput *beats_per_measure;
private:
        inline void cb_beats_per_measure_i(fltk::ValueInput*, void*);
        static void cb_beats_per_measure(fltk::ValueInput*, void*);
public:
        fltk::ValueInput *measures_per_phrase;
private:
        inline void cb_measures_per_phrase_i(fltk::ValueInput*, void*);
        static void cb_measures_per_phrase(fltk::ValueInput*, void*);
public:
        fltk::ValueOutput *bpm_output;
        fltk::ThumbWheel *bpm_wheel;
private:
        inline void cb_bpm_wheel_i(fltk::ThumbWheel*, void*);
        static void cb_bpm_wheel(fltk::ThumbWheel*, void*);
public:
        fltk::ValueInput *measures_until_record;
private:
        inline void cb_measures_until_record_i(fltk::ValueInput*, void*);
        static void cb_measures_until_record(fltk::ValueInput*, void*);
public:
        fltk::CheckButton *check_alwayscopy;
private:
        inline void cb_check_alwayscopy_i(fltk::CheckButton*, void*);
        static void cb_check_alwayscopy(fltk::CheckButton*, void*);
public:
        fltk::CheckButton *check_autotrackname;
private:
        inline void cb_check_autotrackname_i(fltk::CheckButton*, void*);
        static void cb_check_autotrackname(fltk::CheckButton*, void*);
public:
        fltk::CheckButton *check_passthru;
private:
        inline void cb_check_passthru_i(fltk::CheckButton*, void*);
        static void cb_check_passthru(fltk::CheckButton*, void*);
public:
        fltk::CheckButton *check_playinsert;
private:
        inline void cb_check_playinsert_i(fltk::CheckButton*, void*);
        static void cb_check_playinsert(fltk::CheckButton*, void*);
public:
        fltk::CheckButton *check_recordonchan;
private:
        inline void cb_check_recordonchan_i(fltk::CheckButton*, void*);
        static void cb_check_recordonchan(fltk::CheckButton*, void*);
public:
        fltk::CheckButton *check_playmove;
private:
        inline void cb_check_playmove_i(fltk::CheckButton*, void*);
        static void cb_check_playmove(fltk::CheckButton*, void*);
public:
        fltk::CheckButton *check_follow;
private:
        inline void cb_check_follow_i(fltk::CheckButton*, void*);
        static void cb_check_follow(fltk::CheckButton*, void*);
public:
        fltk::ValueInput *default_velocity;
private:
        inline void cb_default_velocity_i(fltk::ValueInput*, void*);
        static void cb_default_velocity(fltk::ValueInput*, void*);
public:
        fltk::Choice *menu_recordmode;
private:
          inline void cb_merge_i(fltk::Item*, void*);
          static void cb_merge(fltk::Item*, void*);
          inline void cb_overwrite_i(fltk::Item*, void*);
          static void cb_overwrite(fltk::Item*, void*);
          inline void cb_layer_i(fltk::Item*, void*);
          static void cb_layer(fltk::Item*, void*);
public:
        fltk::Choice *menu_rob;
private:
          inline void cb_do_i(fltk::Item*, void*);
          static void cb_do(fltk::Item*, void*);
          inline void cb_new_i(fltk::Item*, void*);
          static void cb_new(fltk::Item*, void*);
          inline void cb_extend_i(fltk::Item*, void*);
          static void cb_extend(fltk::Item*, void*);
public:
        KeyGrabber *kg_l0;
        KeyGrabber *kg_l1;
        KeyGrabber *kg_l2;
        KeyGrabber *kg_l3;
        KeyGrabber *kg_l4;
        KeyGrabber *kg_l5;
        KeyGrabber *kg_l6;
        KeyGrabber *kg_l7;
        KeyGrabber *kg_l8;
        KeyGrabber *kg_l9;
        KeyGrabber *kg_l10;
private:
        inline void cb_kg_l10_i(KeyGrabber*, void*);
        static void cb_kg_l10(KeyGrabber*, void*);
public:
        KeyGrabber *kg_l11;
        KeyGrabber *kg_l12;
        KeyGrabber *kg_l13;
        KeyGrabber *kg_l14;
        KeyGrabber *kg_l15;
        KeyGrabber *kg_l16;
        KeyGrabber *kg_u0;
        KeyGrabber *kg_u1;
        KeyGrabber *kg_u2;
        KeyGrabber *kg_u3;
        KeyGrabber *kg_u4;
        KeyGrabber *kg_u5;
        KeyGrabber *kg_u6;
        KeyGrabber *kg_u7;
        KeyGrabber *kg_u8;
        KeyGrabber *kg_u9;
        KeyGrabber *kg_u10;
        KeyGrabber *kg_u11;
        KeyGrabber *kg_u12;
        KeyGrabber *kg_u13;
        KeyGrabber *kg_u14;
        KeyGrabber *kg_u15;
        KeyGrabber *kg_u16;
        KeyGrabber *kg_u17;
        KeyGrabber *kg_u18;
        KeyGrabber *kg_u19;
        KeyGrabber *kg_u20;
        KeyGrabber *kg_zi;
        KeyGrabber *kg_zo;
        KeyGrabber *kg_ou;
        KeyGrabber *kg_od;
  fltk::Window *help_window;
        fltk::Input *title_text;
        fltk::Input *author_text;
        fltk::TextEditor *info_text;
  fltk::Window *action_window;
private:
    inline void cb_new1_i(fltk::Button*, void*);
    static void cb_new1(fltk::Button*, void*);
    inline void cb_save_i(fltk::Button*, void*);
    static void cb_save(fltk::Button*, void*);
    inline void cb_save1_i(fltk::Button*, void*);
    static void cb_save1(fltk::Button*, void*);
    inline void cb_load_i(fltk::Button*, void*);
    static void cb_load(fltk::Button*, void*);
    inline void cb_import_i(fltk::Button*, void*);
    static void cb_import(fltk::Button*, void*);
    inline void cb_export_i(fltk::Button*, void*);
    static void cb_export(fltk::Button*, void*);
public:
  fltk::Window *scope_window;
    fltk::TextDisplay *scope;
private:
      inline void cb_on_i(fltk::Button*, void*);
      static void cb_on(fltk::Button*, void*);
};
#endif
