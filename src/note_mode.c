/* ------------------------------------------------
 *
 * NOTE MODE
 *
 */

/*
 * INCLUDE FILES
 */
#include "app.h"
#include "app_defs.h"
#include "xapp.h"
#include "control_surface.h"

/*
 * FORWARD DECLARATIONS OF HANDLER FUNCTIONS
 */
static void handleInit();
static void handleGridPress(byte row, byte col, BUTTON_ACTION type, byte press);
static void handleMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press);
static void handleRepaint();

/*
 * EXPORT PAGE HANDLERS INTERFACE
 */
PAGE_HANDLERS NoteMode = {	handleInit, handleGridPress, handleMenuButton, handleRepaint};

#define NO_NOTE ((byte)0xff)
#define MIN_OCTAVE 0
#define MAX_OCTAVE 7
#define DEFAULT_OCTAVE 4
#define MIDDLE_C 60

enum {
	CMD_OCTAVE_UP = MNU_STOPCLIP,
	CMD_OCTAVE_DOWN = MNU_SENDS,
	CMD_TRANSPOSE_UP = MNU_RIGHT,
	CMD_TRANSPOSE_DOWN = MNU_LEFT,
	CMD_DEFAULT_OCTAVE = 1000,
	CMD_DEFAULT_TRANSPOSE = 1001
};
enum {
	KEYSTATE_UP = 0x01,
	KEYSTATE_DOWN = 0x02,
	KEYSTATE_LEFT = 0x04,
	KEYSTATE_RIGHT = 0x08
};

enum {
	FLAG_NOTE = 0x01,
	FLAG_WHITENOTE = 0x02,
	FLAG_HELD = 0x04
};

typedef struct _CELL {
	byte note;
	byte flags;
} CELL;

typedef struct _PIANO_DATA {
	CELL grid[8][8];
	byte octave;	// octave shift
	int transpose;  // note transpose
	byte keyState;
} NOTE_DATA;

/*
 * DEFINE THE INSTANCE DATA
 */
#pragma GCC diagnostic ignored "-Wmissing-braces"
static NOTE_DATA Me = {0};
#pragma GCC diagnostic warning "-Wmissing-braces"

/* ------------------------------------------------
 *
 *
 * HELPER FUNCTIONS
 *
 *
 * ------------------------------------------------
 */

/*
 * Determine if a given MIDI note number is a "white note"
 */
static inline int is_white_note(int n) {
	switch(n%12) {
	case 0:	case 2:	case 4:	case 5:
	case 7:	case 9:	case 11:
		return TRUE;
	default:
		return FALSE;
	}
}

/*
 * Clear the grid
 */
static void clearGrid() {
	for(int row = 0; row < 8; ++row) {
		for(int col = 0; col < 8; ++col) {
			Me.grid[row][col].note = 0;
			Me.grid[row][col].flags = 0;
		}
	}
}

/*
 * Configure with 4 octave piano style keyboard
 */
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
				Me.grid[row][col].flags |= FLAG_NOTE|FLAG_WHITENOTE;
				if(note < 127) {
					switch(note % 12) {
					case 0: case 2: case 5: case 7: case 9:
						++note;
						Me.grid[row-1][col].note = note;
						Me.grid[row-1][col].flags |= FLAG_NOTE;
					}
				}
			}
			++note;
		}
	}
}

/*
 * repaint the grid
 */
static void repaintGrid() {
	for(int row=0; row < 8; ++row) {
		for(int col=0; col < 8; ++col) {
			if(Me.grid[row][col].flags & FLAG_NOTE) {
				if(Me.grid[row][col].note == MIDDLE_C) {
					XSetGridLed(row, col, 0x00FF00);
				}
				else if(Me.grid[row][col].flags & FLAG_WHITENOTE)  {
					XSetGridLed(row, col, 0x008888);
				}
				else {
					XSetGridLed(row, col, 0x0000FF);
				}
			}
			else {
				XSetGridLed(row, col, 0x000000);
			}
		}
	}
}

/*
 * repaint the "scroll bar" which shows octave position
 */
static void repaintScrollbar() {
	int oct = 7;
	for(int row=0; row < 8; ++row) {
		XSetGridLed(row, 7, (oct-- == Me.octave) ? 0xFFFF00 : 0x333300);
	}
}

/*
 * transpose the keyboard
 */
static void transpose(int cmd) {
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
		if(octave > MIN_OCTAVE) {
			--octave;
		}
		break;
	case CMD_TRANSPOSE_UP:
		++transpose;
		if(!is_white_note(transpose)) {
			++transpose;
		}
		if(transpose > 11) {
			transpose = 0;
		}
		break;
	case CMD_TRANSPOSE_DOWN:
		--transpose;
		if(!is_white_note(transpose)) {
			--transpose;
		}
		if(transpose < 0) {
			transpose = 11;
		}
		break;
	}
	if(octave != Me.octave || transpose != Me.transpose) {
		for(int row = 0; row < 8; ++row) {
			for(int col = 0; col < 8; ++col) {
				if(Me.grid[row][col].flags & FLAG_HELD) {
					handleGridPress(row, col, BUTTON_RELEASE, 0);
				}
			}
		}
		Me.octave = octave;
		Me.transpose = transpose;
		configGridChromatic();
		handleRepaint();
	}
}

/* ------------------------------------------------
 *
 *
 * HANDLER FUNCTION ENTRY POINTS FOR THIS PAGE
 *
 *
 * ------------------------------------------------
 */

/*
 * INITIALISE
 */
void handleInit() {
	Me.octave = DEFAULT_OCTAVE;
	Me.transpose = 0;

	configGridChromatic();
}

/*
 * BUTTON PRESSED IN GRID
 */
void handleGridPress(byte row, byte col, BUTTON_ACTION type, byte press) {
	if(col<7) {
		if(Me.grid[row][col].flags & FLAG_NOTE) {
			switch(type) {
			case BUTTON_PRESS:
				XStartNote(Me.grid[row][col].note, press);
				XSetGridLedEffect(row, col, EFFECT_HIGHLIGHT, press);
				Me.grid[row][col].flags |= FLAG_HELD;
				break;
			case BUTTON_RELEASE:
				XStopNote(Me.grid[row][col].note);
				XSetGridLedEffect(row, col, EFFECT_NONE, press);
				Me.grid[row][col].flags &= ~FLAG_HELD;
				break;
			case BUTTON_AFTERTOUCH:
				XAfterTouch(Me.grid[row][col].note, press);
				XSetGridLedEffect(row, col, EFFECT_HIGHLIGHT, press);
				Me.grid[row][col].flags |= FLAG_HELD;
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

/*
 * MENU BUTTON PRESSED
 */
void handleMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press) {
	switch(type) {
		case BUTTON_PRESS:
			switch(which) {
			case CMD_OCTAVE_UP:
				Me.keyState |= KEYSTATE_UP;
				transpose((Me.keyState & KEYSTATE_DOWN)? CMD_DEFAULT_OCTAVE : CMD_OCTAVE_UP);
				break;
			case CMD_OCTAVE_DOWN:
				Me.keyState |= KEYSTATE_DOWN;
				transpose((Me.keyState & KEYSTATE_UP)? CMD_DEFAULT_OCTAVE : CMD_OCTAVE_DOWN);
				break;
			case CMD_TRANSPOSE_DOWN:
				Me.keyState |= KEYSTATE_LEFT;
				transpose((Me.keyState & KEYSTATE_RIGHT)? CMD_DEFAULT_TRANSPOSE : CMD_TRANSPOSE_DOWN);
				break;
			case CMD_TRANSPOSE_UP:
				Me.keyState |= KEYSTATE_RIGHT;
				transpose((Me.keyState & KEYSTATE_LEFT)? CMD_DEFAULT_TRANSPOSE : CMD_TRANSPOSE_UP);
				break;
			default:
				break;
			}
			break;
		case BUTTON_RELEASE:
			switch(which) {
			case CMD_OCTAVE_UP:
				Me.keyState &= ~KEYSTATE_UP;
				break;
			case CMD_OCTAVE_DOWN:
				Me.keyState &= ~KEYSTATE_DOWN;
				break;
			case CMD_TRANSPOSE_DOWN:
				Me.keyState &= ~KEYSTATE_LEFT;
				break;
			case CMD_TRANSPOSE_UP:
				Me.keyState &= ~KEYSTATE_RIGHT;
				break;
			default:
				break;
			}
			break;
		default:
			break;
	}
}

/*
 * REPAINT
 */
void handleRepaint() {
	XCls();
	XSetMenuLed(CMD_OCTAVE_UP, COLOUR_WHITE);
	XSetMenuLed(CMD_OCTAVE_DOWN, COLOUR_WHITE);
	XSetMenuLed(CMD_TRANSPOSE_UP, COLOUR_WHITE);
	XSetMenuLed(CMD_TRANSPOSE_DOWN, COLOUR_WHITE);

	repaintGrid();
	repaintScrollbar();
}
