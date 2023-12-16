# Adapted from Makefile for Dingux by Rikku2000

TARGET = sdlpal

HOST =

ADPLUG_FILES = adplug/rix.cpp adplug/player.cpp adplug/binio.cpp \
	adplug/fprovide.cpp adplug/binfile.cpp adplug/dosbox_opl.cpp \
	adplug/fmopl.c adplug/surroundopl.cpp adplug/emuopl.cpp

FILES = rixplay.cpp text.c font.c itemmenu.c scene.c palcommon.c script.c \
	util.c play.c getopt.c input.c uibattle.c game.c magicmenu.c map.c \
	aviplay.c uigame.c rngplay.c ui.c global.c main.c fight.c \
	video.c palette.c sound.c res.c battle.c yj2.cpp \
	native_midi/native_midi_common.c native_midi/native_midi_win32.c

FILES += $(ADPLUG_FILES)

CFLAGS = `sdl-config --cflags` -g -Wall -O2 -fno-strict-aliasing
LDFLAGS = `sdl-config --libs` -lstdc++ -lm

$(TARGET): $(FILES)
	$(HOST)gcc $(CFLAGS) -o $(TARGET) $(FILES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
