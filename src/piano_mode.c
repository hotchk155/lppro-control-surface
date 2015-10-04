#include "app.h"
#include "app_defs.h"
#include "xapp.h"
#include "control_surface.h"


static void pianoInit();
static void pianoGridPress(byte row, byte col, BUTTON_ACTION type, byte press);
static void pianoMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press);
static void pianoRepaint();

PAGE_HANDLERS PianoMode = {	pianoInit, pianoGridPress, pianoMenuButton, pianoRepaint};

#define NO_NOTE ((byte)0xff)
#define MAX_OCTAVE 7
#define DEFAULT_OCTAVE 4
#define MIDDLE_C 60

enum {
	CMD_DEFAULT_OCTAVE = -1,
	CMD_DEFAULT_TRANSPOSE = -2,
	CMD_OCTAVE_UP = MNU_STOPCLIP,
	CMD_OCTAVE_DOWN = MNU_SENDS,
	CMD_TRANSPOSE_UP = MNU_RIGHT,
	CMD_TRANSPOSE_DOWN = MNU_LEFT
};
enum {
	KEY_UP = 0x01,
	KEY_DOWN = 0x02,
	KEY_LEFT = 0x04,
	KEY_RIGHT = 0x08
};

enum {
	FLAG_HELD = 1
};

typedef struct _CELL {
	byte note;
	byte flags;
} CELL;

typedef struct _PIANO_DATA {
	CELL grid[8][8];
	byte octave;	// octave shift
	int transpose;  // note transpose
	byte keyDown;
} PIANO_DATA;

static PIANO_DATA Me = {{{0}},0};



static void clearGrid() {
	for(int row = 0; row < 8; ++row) {
		for(int col = 0; col < 8; ++col) {
			Me.grid[row][col].note = NO_NOTE;
		}
	}
}
static void configGridChromatic() {
	clearGrid();
	int note = 12 * Me.octave + Me.transpose;

	// The four octaves of notes on screen
	for(int row = 7; row > 0; row -=2)
	{
		// the 7 colums of notes per octave
		for(int col=0; col<7; ++col) {
			if(note >= 0 && note < 128) {
				Me.grid[row][col].note = note;
				if(note < 127) {
					switch(note % 12) {
					case 0: case 2: case 5: case 7: case 9:
						++note;
						Me.grid[row-1][col].note = note;
					}
				}
			}
			++note;
		}
	}
}

static void repaintGrid() {
	for(int row=0; row < 8; ++row) {
		for(int col=0; col < 8; ++col) {
			if(Me.grid[row][col].note == NO_NOTE) {
				XSetGridLed(row, col, 0x000000);
			}
			else if(Me.grid[row][col].note == MIDDLE_C) {
				XSetGridLed(row, col, 0x00FF88);
			}
			else {
				switch(Me.grid[row][col].note % 12) {
					case 0://C
					case 2://D
					case 4://E
					case 5://F
					case 7://G
					case 9://A
					case 11://B
						XSetGridLed(row, col, 0x00AAAA);
						break;
					case 1://C#
					case 3://D#
					case 6://F#
					case 8://G#
					case 10://A#
						XSetGridLed(row, col, 0x0000FF);
						break;
				}
			}
		}
	}
}
static void repaintScrollbar() {
	int oct = 7;
	for(int row=0; row < 8; ++row) {
		XSetGridLed(row, 7, (oct-- == Me.octave) ? 0xFFFF00 : 0x888800);
	}
}
void pianoRepaint() {
	repaintGrid();
	repaintScrollbar();
}

void pianoInit() {
	Me.octave = DEFAULT_OCTAVE;
	Me.transpose = 0;

	configGridChromatic();
	XSetMenuLed(CMD_OCTAVE_UP, COLOUR_WHITE);
	XSetMenuLed(CMD_OCTAVE_DOWN, COLOUR_WHITE);
	XSetMenuLed(CMD_TRANSPOSE_UP, COLOUR_WHITE);
	XSetMenuLed(CMD_TRANSPOSE_DOWN, COLOUR_WHITE);
}

void pianoGridPress(byte row, byte col, BUTTON_ACTION type, byte press) {
	if(col<7) {
		byte note = Me.grid[row][col].note;
		if(note != NO_NOTE) {
			switch(type) {
			case BUTTON_PRESS:
				XStartNote(note, press);
				XSetGridLedEffect(row, col, EFFECT_HIGHLIGHT, press);
				Me.grid[row][col].flag |= FLAG_HELD;
				break;
			case BUTTON_RELEASE:
				XStopNote(note);
				XSetGridLedEffect(row, col, EFFECT_NONE, press);
				Me.grid[row][col].flag &= ~FLAG_HELD;
				break;
			case BUTTON_AFTERTOUCH:
				XAfterTouch(note, press);
				XSetGridLedEffect(row, col, EFFECT_HIGHLIGHT, press);
				Me.grid[row][col].flag |= FLAG_HELD;
				break;
			}
		}
	}
	/*
	else if(col ==7) {
		if(Me.octave != 7-row) {
			Me.octave = 7-row;
			configGridChromatic();
			pianoRepaint();
		}
	}
	*/
}


#define IS_WHITE_NOTE(n) (\
	((n)==0)||\
	((n)==2)||\
	((n)==4)||\
	((n)==5)||\
	((n)==7)||\
	((n)==9)||\
	((n)==11))





void transpose(byte cmd) {
	byte octave = Me.octave;
	byte transpose = Me.transpose;
	switch(cmd) {
	case CMD_DEFAULT_OCTAVE:
		octave = DEFAULT_OCTAVE;
		break;
	case CMD_DEFAULT_TRANSPOSE:
		transpose = 0;
		break;
	case CMD_OCTAVE_UP:
		if(octave < MAX_OCTAVE) {
			++octave;
		}
		break;
	case CMD_OCTAVE_DOWN:
		if(octave < MIN_OCTAVE) {
			--octave;
		}
		break;
	case CMD_TRANSPOSE_UP:
		++transpose;
		if(!IS_WHITE_NOTE(transpose)) ++transpose;
		if(transpose > 11) transpose = 0;
		break;
	case CMD_TRANSPOSE_DOWN:
		--transpose;
		if(!IS_WHITE_NOTE(transpose)) --transpose;
		if(transpose < 0) transpose = 11;
		break;
	}
	if(octave != Me.octave || transpose != Me.transpose) {
		for(int row = 0; row < 8; ++row) {
			for(int col = 0; col < 8; ++col) {
				if(grid[row][col].flags & FLAG_HELD) {
					pianoGridPress(row, col, BUTTON_RELEASE, 0);
					pianoGridPress(row, col, BUTTON_PRESS, 1);
				}
			}
		}
		Me.octave = octave;
		Me.transpose = transpose;
		configGridChromatic();
		pianoRepaint();
	}
}

void pianoMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press) {
	switch(type) {
		case BUTTON_PRESS:
			switch(which) {
			case CMD_OCTAVE_UP:
				Me.keyDown |= KEY_UP;
				transpose((Me.keyDown & KEY_DOWN)? CMD_DEFAULT_OCTAVE : CMD_OCTAVE_UP);
				break;
			case CMD_OCTAVE_DOWN:
				Me.keyDown |= KEY_DOWN;
				transpose((Me.keyDown & KEY_UP)? CMD_DEFAULT_OCTAVE : CMD_OCTAVE_DOWN);
				break;
			case CMD_TRANSPOSE_DOWN:
				Me.keyDown |= KEY_LEFT;
				transpose((Me.keyDown & KEY_RIGHT)? CMD_DEFAULT_TRANSPOSE : CMD_TRANSPOSE_DOWN);
				break;
			case CMD_TRANSPOSE_UP:
				Me.keyDown |= KEY_RIGHT;
				transpose((Me.keyDown & KEY_LEFT)? CMD_DEFAULT_TRANSPOSE : CMD_TRANSPOSE_UP);
			}
			break;
		case BUTTON_RELEASE:
			switch(which) {
			case CMD_OCTAVE_UP:
				Me.keyDown &= ~KEY_UP;
				break;
			case CMD_OCTAVE_DOWN:
				Me.keyDown &= ~KEY_DOWN;
				break;
			case CMD_TRANSPOSE_DOWN:
				Me.keyDown &= ~KEY_LEFT;
				break;
			case CMD_TRANSPOSE_UP:
				Me.keyDown &= ~KEY_RIGHT;
				break;
			}
			break;
	}
}

