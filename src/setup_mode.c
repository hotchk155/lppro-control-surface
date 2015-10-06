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
PAGE_HANDLERS SetupMode = {	handleInit, handleGridPress, handleMenuButton, handleRepaint};

#define ROWCOL(r,c) (((int)(r) << 8)|(int)(c))

typedef struct _SETUP_DATA {
	byte dummy;
} SETUP_DATA;


/*
 * DEFINE THE INSTANCE DATA
 */
#pragma GCC diagnostic ignored "-Wmissing-braces"
static SETUP_DATA Me = {0};
#pragma GCC diagnostic warning "-Wmissing-braces"

/* ------------------------------------------------
 *
 *
 * HELPER FUNCTIONS
 *
 *
 * ------------------------------------------------
 */

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
}

/*
 * BUTTON PRESSED IN GRID
 */
void handleGridPress(byte row, byte col, BUTTON_ACTION type, byte press) {
	if(type == BUTTON_PRESS) {
		switch(ROWCOL(row,col)) {
		case ROWCOL(0,0) :
				CtrlSetMode(CTRL_MODE_NOTE);
				handleRepaint();
				break;
		case ROWCOL(0,1) :
				CtrlSetMode(CTRL_MODE_KIT);
				handleRepaint();
				break;
		}
	}
}

/*
 * MENU BUTTON PRESSED
 */
void handleMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press) {
}

/*
 * REPAINT
 */
void handleRepaint() {
	CTRL_MODE mode = CtrlGetMode();
	XCls();
	XSetGridLed(0,0,(mode == CTRL_MODE_NOTE)? 0xFFFF00 : 0x333300);
	XSetGridLed(0,1,(mode == CTRL_MODE_KIT)? 0x00FFFF : 0x003333);
}

