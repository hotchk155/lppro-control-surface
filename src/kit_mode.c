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

#define COLOUR_PAGE1	0x00FF33
#define COLOUR_PAGE2	0x0033FF
#define COLOUR_LAYOUT	0x3333FF

enum {
	PAGE_NOTES1,
	PAGE_NOTES2,
	PAGE_LAYOUT
};

enum {
	FLAG_NOTE = 0x01,
	FLAG_HELD = 0x02
};
typedef struct {
	byte note;
	byte flags;
} CELL;


typedef struct _KIT_DATA {
	CELL notes1[64];	// notes 0-63
	CELL notes2[64];  // notes 64-127
	CELL layout[64];  // user layout
	byte page;
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
	for(int row = 0; row < 8; ++row) {
		for(int col = 0; col < 8; ++col) {
			XSetGridLed(row, col, COLOUR_PAGE1);
		}
	}
}

void repaintNotes2() {
	for(int row = 0; row < 8; ++row) {
		for(int col = 0; col < 8; ++col) {
			XSetGridLed(row, col, COLOUR_PAGE2);
		}
	}
}

void repaintLayout() {
	for(int row = 0; row < 8; ++row) {
		for(int col = 0; col < 8; ++col) {
			XSetGridLed(row, col, 0);
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
	CELL *pcell;
	int note = 127;
	for(int row = 0; row<8; ++row) {
		for(int col = 0; col<8; ++col) {

			pcell = &Me.notes2[row*8+col];
			pcell->note = note;
			pcell->flags = FLAG_NOTE;

			pcell = &Me.notes1[row*8+col];
			pcell->note = note - 64;
			pcell->flags = FLAG_NOTE;

			pcell = &Me.layout[row*8+col];
			pcell->note = 0;
			pcell->flags = 0;

			--note;
		}
	}
	Me.page = PAGE_NOTES1;
}


/*
 * BUTTON PRESSED IN GRID
 */
void handleGridPress(byte row, byte col, BUTTON_ACTION type, byte press) {
	CELL *pcell;
	switch(Me.page) {
	case PAGE_LAYOUT:
		pcell = &Me.layout[8*row+col];
		break;
	case PAGE_NOTES2:
		pcell = &Me.notes2[8*row+col];
		break;
	case PAGE_NOTES1:
	default:
		pcell = &Me.notes1[8*row+col];
		break;
	}

	if(pcell->flags & FLAG_NOTE) {
		switch(type) {
		case BUTTON_PRESS:
			XStartNote(pcell->note, press);
			XSetGridLedEffect(row, col, EFFECT_HIGHLIGHT, press);
			pcell->flags |= FLAG_HELD;
			break;
		case BUTTON_RELEASE:
			XStopNote(pcell->note);
			XSetGridLedEffect(row, col, EFFECT_NONE, press);
			pcell->flags &= ~FLAG_HELD;
			break;
		case BUTTON_AFTERTOUCH:
			XAfterTouch(pcell->note, press);
			XSetGridLedEffect(row, col, EFFECT_HIGHLIGHT, press);
			pcell->flags |= FLAG_HELD;
			break;
		}
	}
}

/*
 * MENU BUTTON PRESSED
 */
void handleMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press) {
	if(type == BUTTON_PRESS) {
		int nextPage = PAGE_NOTES1;
		switch(which) {
			case CMD_PAGE1:
				nextPage = PAGE_NOTES1;
				break;
			case CMD_PAGE2:
				nextPage = PAGE_NOTES2;
				break;
			case CMD_LAYOUT:
				nextPage = PAGE_LAYOUT;
				break;
			default:
				break;
		}
		if(nextPage != Me.page) {
			Me.page = nextPage;
			handleRepaint();
		}
	}
}

/*
 * REPAINT
 */
void handleRepaint() {
	XCls();
	XSetMenuLed(CMD_PAGE1, COLOUR_PAGE1);
	XSetMenuLed(CMD_PAGE2, COLOUR_PAGE2);
	XSetMenuLed(CMD_LAYOUT, COLOUR_LAYOUT);
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
