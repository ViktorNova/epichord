AM_CPPFLAGS = -DROOT_DATA_DIR="\"$(datadir)/@PACKAGE@/\""
CFLAGS = -g -O2 -Wall
AM_LDFLAGS = -lfltk2 -ljack

bin_PROGRAMS = epichord
epichord_SOURCES = main.cpp seq.cpp jack.cpp ui.cpp \
                   pianoroll.cpp trackselect.cpp eventedit.cpp timeline.cpp \
                   sampleview.cpp trackinfo.cpp arranger.cpp keyboard.cpp \
                   trackmodule.cpp util.cpp midi.cpp uihelper.cpp \
                   saveload.cpp
