AM_CPPFLAGS = -DROOT_DATA_DIR="\"$(datadir)/@PACKAGE@/\""
CFLAGS = -g -O2 -Wall
AM_LDFLAGS = `fltk2-config --ldflags` -ljack
if HAVE_LASH
AM_CPPFLAGS += @LASH_CFLAGS@
AM_LDFLAGS +=  @LASH_LIBS@
endif

bin_PROGRAMS = epichord
epichord_SOURCES = main.cpp seq.cpp jack.cpp ui.cpp \
pianoroll.cpp trackselect.cpp eventedit.cpp timeline.cpp \
sampleview.cpp trackinfo.cpp arranger.cpp keyboard.cpp \
trackmodule.cpp util.cpp midi.cpp uihelper.cpp \
saveload.cpp seq.h backend.h ui.h pianoroll.h \
trackselect.h eventedit.h timeline.h sampleview.h \
trackinfo.h arranger.h keyboard.h trackmodule.h \
util.h midi.h uihelper.h saveload.h metronome.cpp \
metronome.h eventmenu.cpp eventmenu.h \
theme.cpp theme.h dragbar.cpp dragbar.h
