/* ------------------------------------------------
 *
 * KIT MODE
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
PAGE_HANDLERS KitMode = {	handleInit, handleGridPress, handleMenuButton, handleRepaint};

#define CMD_PAGE1		MNU_RECORDARM
#define CMD_PAGE2		MNU_TRACKSELECT
#define CMD_LAYOUT		MNU_MUTE
#define CMD_CANCEL		MNU_STOPCLIP

#define COLOUR_PAGE1	0x00FF33
#define COLOUR_PAGE2	0x0033FF
#define COLOUR_LAYOUT	0x3333FF
#define COLOUR_MAPPED	0xFFFF00
#define COLOUR_CANCEL	0xFFFFFF
#define COLOUR_SELECTED	0xFF0000

enum {
	PAGE_NOTES1 = 0x01,
	PAGE_NOTES2 = 0x02,
	PAGE_LAYOUT = 0x04
};

enum {
	SHIFT_NOTES1 = 0x01,
	SHIFT_NOTES2 = 0x02,
	SHIFT_LAYOUT = 0x04,
	SHIFT_CANCEL  = 0x08
};


enum {
	FLAG_MAPPED = 0x01,	// Set if this note is mapped on the kit page
	FLAG_HELD = 0x02,	// Set if the note is currently being played
	FLAG_SELECTED = 0x04
};

/*
 * This structure is used to keep information about
 * a MIDI note 0-127
 */
typedef struct _NOTEINFO {
	byte flags;
} NOTEINFO;

#define NO_NOTE	((byte)0xFF)
typedef struct _KITLOCATION {
	byte note;
} KITLOCATION;


#define MAX_SELECTION 20

typedef struct _KIT_DATA {
	NOTEINFO notes[128];
	KITLOCATION kit[64];
	byte selection[MAX_SELECTION];
	byte selectionCount;
	byte page;
	byte shift;
} KIT_DATA;




/*
 * DEFINE THE INSTANCE DATA
 */
#pragma GCC diagnostic ignored "-Wmissing-braces"
static KIT_DATA Me = {0};
#pragma GCC diagnostic warning "-Wmissing-braces"

/* ------------------------------------------------
 *
 *
 * HELPER FUNCTIONS
 *
 *
 * ------------------------------------------------
 */

void repaintNotes1() {
	int index = 0;
	for(int row = 7; row >= 0; --row) {
		for(int col = 0; col < 8; ++col) {
			if(Me.notes[index].flags & FLAG_SELECTED) {
				XSetGridLed(row, col, COLOUR_SELECTED);
			}
			else if(Me.notes[index].flags & FLAG_MAPPED) {
				XSetGridLed(row, col, COLOUR_MAPPED);
			} else {
				XSetGridLed(row, col, COLOUR_PAGE1);
			}
			++index;
		}
	}
}
void repaintNotes2() {
	int index = 64;
	for(int row = 7; row >= 0; --row) {
		for(int col = 0; col < 8; ++col) {
			if(Me.notes[index].flags & FLAG_SELECTED) {
				XSetGridLed(row, col, COLOUR_SELECTED);
			}
			else if(Me.notes[index].flags & FLAG_MAPPED) {
				XSetGridLed(row, col, COLOUR_MAPPED);
			} else {
				XSetGridLed(row, col, COLOUR_PAGE2);
			}
			++index;
		}
	}
}
void repaintLayout() {
	int index = 0;
	for(int row = 0; row < 8; ++row) {
		for(int col = 0; col < 8; ++col) {
			int note = Me.kit[index].note;
			if(note == NO_NOTE) {
				XSetGridLed(row, col, 0);
			}
			else if(Me.notes[note].flags & FLAG_SELECTED) {
				XSetGridLed(row, col, COLOUR_SELECTED);
			}
			else {
				XSetGridLed(row, col, COLOUR_MAPPED);
			}
			++index;
		}
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
	int index;
	for(index= 0; index<64; ++index) {
		Me.kit[index].note = NO_NOTE;
	}
	for(index= 0; index<128; ++index) {
		Me.notes[index].flags = 0;
	}
	Me.page = PAGE_NOTES1;
	Me.shift = 0;
	Me.selectionCount = 0;
}


/*
 * HANDLE A KEYPRESS ON GRID NOTE
 */
void playNote(byte note, byte row, byte col, BUTTON_ACTION type, byte press) {
	switch(type) {
	case BUTTON_PRESS:
		XStartNote(note, press);
		XSetGridLedEffect(row, col, EFFECT_HIGHLIGHT, press);
		Me.notes[note].flags |= FLAG_HELD;
		break;
	case BUTTON_RELEASE:
		XStopNote(note);
		XSetGridLedEffect(row, col, EFFECT_NONE, press);
		Me.notes[note].flags &= ~FLAG_HELD;
		break;
	case BUTTON_AFTERTOUCH:
		XAfterTouch(note, press);
		XSetGridLedEffect(row, col, EFFECT_HIGHLIGHT, press);
		Me.notes[note].flags |= FLAG_HELD;
		break;
	}
}

static void pasteNote(byte row, byte col) {

	int index = 8*row + col;

	// make sure the kit location is free and that there is
	// a note available to paste
	if(Me.kit[index].note == NO_NOTE && Me.selectionCount > 0) {

		// get the next note to paste
		byte note = Me.selection[0];

		// if the note is already pasted elsewhere in the kit
		// then it is removed from that other location
		for(int i = 0; i < 64; ++i) {
			if(Me.kit[i].note == note) {
				Me.kit[i].note = NO_NOTE;
			}
		}

		// paste note into kit
		Me.kit[index].note = note;
		Me.notes[note].flags &= ~FLAG_SELECTED;
		Me.notes[note].flags |= FLAG_MAPPED;

		// remove note from buffer
		--Me.selectionCount;
		for(int i = 0; i < Me.selectionCount; ++i ) {
			Me.selection[i] = Me.selection[i+1];
		}

		handleRepaint();
	}
}

static void cutCopyNote(byte note) {

	// is the note already in the selection buffer?
	if(Me.notes[note].flags & FLAG_SELECTED) {

		// remove it
		int i;
		Me.notes[note].flags &= ~FLAG_SELECTED;
		for(i = 0; i < Me.selectionCount; ++i) {
			if(Me.selection[i] == note) {
				break;
			}
		}
		for( ; i < Me.selectionCount - 1; ++i) {
			Me.selection[i] = Me.selection[i+1];
		}
		--Me.selectionCount;
	}
	// otherwise is there room for another selection?
	else if(Me.selectionCount < MAX_SELECTION - 1) {
		// select the note
		Me.selection[Me.selectionCount++] = note;
		Me.notes[note].flags = FLAG_SELECTED;
	}
	handleRepaint();
}



static void deleteNote(byte note) {
	if(note != NO_NOTE) {
		if(Me.notes[note].flags & FLAG_MAPPED) {
			Me.notes[note].flags &= ~FLAG_MAPPED;
			for(int i=0; i<64; ++i) {
				if(Me.kit[i].note == note) {
					Me.kit[i].note = NO_NOTE;
				}
			}
			handleRepaint();
		}
	}
}


static void cancelSelection() {
	byte refresh = 0;
	for(int i=0; i<128; ++i) {
		if(Me.notes[i].flags & FLAG_SELECTED) {
			Me.notes[i].flags &= ~FLAG_SELECTED;
			refresh = 1;
		}
	}
	if(Me.selectionCount > 0 || refresh) {
		Me.selectionCount = 0;
		handleRepaint();
	}
}


/*
 * BUTTON PRESSED IN GRID
 */
void handleGridPress(byte row, byte col, BUTTON_ACTION type, byte press) {
	byte note = NO_NOTE;
	switch(Me.page) {
	case PAGE_LAYOUT:
		note = Me.kit[8*row + col].note;
		break;
	case PAGE_NOTES2:
		note = 64 + 8*(7-row) + col;
		break;
	case PAGE_NOTES1:
	default:
		note = 8*(7-row) + col;
		break;
	}

	if(type == BUTTON_PRESS) {
		// pressing an empty location?
		if(note == NO_NOTE) {
			// are we on the kit page, with the page button down
			// (paste gesture)
			if(Me.page == PAGE_LAYOUT && (Me.shift & SHIFT_LAYOUT)) {
				pasteNote(row, col);
			}
		}
		else if(Me.shift == Me.page) {
			// cut/copy the note
			cutCopyNote(note);
		}
		else if(Me.shift & SHIFT_CANCEL) {
			deleteNote(note);
		}
		else {
			// play the note
			playNote(note, row, col, type, press);
		}
	}
	else if(note != NO_NOTE){
		// play the note
		playNote(note, row, col, type, press);
	}
}

/*
 * MENU BUTTON PRESSED
 */
void handleMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press) {

	int page = Me.page;
	int shift = 0;
	switch(which) {
		case CMD_PAGE1:
			page = PAGE_NOTES1;
			shift = SHIFT_NOTES1;
			break;
		case CMD_PAGE2:
			page = PAGE_NOTES2;
			shift = SHIFT_NOTES2;
			break;
		case CMD_LAYOUT:
			page = PAGE_LAYOUT;
			shift = SHIFT_LAYOUT;
			break;
		case CMD_CANCEL:
			shift = SHIFT_CANCEL;
			break;
	}

	if(type == BUTTON_PRESS) {
		if(which == CMD_CANCEL) {
			cancelSelection();
		}
		Me.shift |= shift;
		if(page != Me.page) {
			Me.page = page;
			handleRepaint();
		}
	}
	else if(type == BUTTON_RELEASE) {
		Me.shift &= ~shift;
	}
}

/*
 * REPAINT
 */
void handleRepaint() {
	XCls();
	XSetMenuLed(CMD_PAGE1, COLOUR_PAGE1);
	XSetMenuLed(CMD_PAGE2, COLOUR_PAGE2);
	XSetMenuLed(CMD_LAYOUT, Me.selectionCount > 0 ? COLOUR_SELECTED : COLOUR_LAYOUT);
	XSetMenuLed(CMD_CANCEL, COLOUR_CANCEL);
	switch(Me.page) {
	case PAGE_NOTES1:
		repaintNotes1();
		break;
	case PAGE_NOTES2:
		repaintNotes2();
		break;
	case PAGE_LAYOUT:
		repaintLayout();
		break;
	}
}
