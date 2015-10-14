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
static void handleActivate();
static void handleDeactivate();

/*
 * EXPORT PAGE HANDLERS INTERFACE
 */
PAGE_HANDLERS SetupMode = {	handleInit, handleGridPress, handleMenuButton, handleActivate, handleDeactivate};

#define ROWCOL(r,c) (((int)(r) << 8)|(int)(c))

#define COLOUR_ACTIVEMIDICHAN		0x00FFFF
#define COLOUR_MIDICHANS			0x0000FF
#define COLOUR_ACTIVEVELOCITYMODE 	0x00FFFF
#define COLOUR_VELOCITYMODE 		0x0000FF
#define COLOUR_ACTIVEAFTERMODE 		0x00FFFF
#define COLOUR_AFTERMODE 			0x0000FF
#define COLOUR_ACTIVEAFTERTHRE 		0x00FFFF
#define COLOUR_AFTERTHRE			0x0000FF

/* ------------------------------------------------
 *
 *
 * HELPER FUNCTIONS
 *
 *
 * ------------------------------------------------
 */

static void repaint() {
	CTRL_MODE mode = CtrlGetMode();
	XCls();
	XSetGridLed(0,0,(mode == CTRL_MODE_NOTE)? 0xFFFF00 : 0x333300);
	XSetGridLed(0,1,(mode == CTRL_MODE_KIT)? 0x00FFFF : 0x003333);



	AFTERTOUCH_MODE afterMode = XGetAftertouchMode();
	AFTERTOUCH_THRESHOLD afterThreshold = XGetAftertouchThreshold();
	VELOCITY_MODE velocityMode = XGetVelocityMode();
	byte midiChannel = XGetMidiChannel();

	XSetGridLed(2,0,(velocityMode == VELOCITY_LOW)? COLOUR_ACTIVEVELOCITYMODE: COLOUR_VELOCITYMODE);
	XSetGridLed(2,1,(velocityMode == VELOCITY_MEDIUM)? COLOUR_ACTIVEVELOCITYMODE: COLOUR_VELOCITYMODE);
	XSetGridLed(2,2,(velocityMode == VELOCITY_HIGH)? COLOUR_ACTIVEVELOCITYMODE: COLOUR_VELOCITYMODE);
	XSetGridLed(2,3,(velocityMode == VELOCITY_OFF)? COLOUR_ACTIVEVELOCITYMODE: COLOUR_VELOCITYMODE);

	XSetGridLed(2,5,(afterMode == AFTERTOUCH_OFF)? COLOUR_ACTIVEAFTERMODE: COLOUR_AFTERMODE);
	XSetGridLed(2,6,(afterMode == AFTERTOUCH_POLY)? COLOUR_ACTIVEAFTERMODE: COLOUR_AFTERMODE);
	XSetGridLed(2,7,(afterMode == AFTERTOUCH_CHANNEL)? COLOUR_ACTIVEAFTERMODE: COLOUR_AFTERMODE);

	XSetGridLed(3,5,(afterThreshold == AFTERTOUCH_LOW)? COLOUR_ACTIVEAFTERTHRE: COLOUR_AFTERTHRE);
	XSetGridLed(3,6,(afterThreshold == AFTERTOUCH_MEDIUM)? COLOUR_ACTIVEAFTERTHRE: COLOUR_AFTERTHRE);
	XSetGridLed(3,7,(afterThreshold == AFTERTOUCH_HIGH)? COLOUR_ACTIVEAFTERTHRE: COLOUR_AFTERTHRE);

	// midi channel
	int c = 0;
	for(int row = 6; row <8; ++row) {
		for(int col = 0; col < 8; ++col) {
			XSetGridLed(row,col,(c == midiChannel)? COLOUR_ACTIVEMIDICHAN: COLOUR_MIDICHANS);
			++c;
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
}

/*
 * BUTTON PRESSED IN GRID
 */
void handleGridPress(byte row, byte col, BUTTON_ACTION type, byte press) {

	if(type == BUTTON_PRESS) {
		byte doRepaint = 1;
		if(row >= 6) {
			byte ch = 8*(row-6)+col;
			XSetMidiChannel(ch);
		}
		else {
			switch(ROWCOL(row,col)) {
			case ROWCOL(0,0): CtrlSetMode(CTRL_MODE_NOTE);break;
			case ROWCOL(0,1): CtrlSetMode(CTRL_MODE_KIT); break;
			case ROWCOL(2,0): XSetVelocityMode(VELOCITY_LOW); break;
			case ROWCOL(2,1): XSetVelocityMode(VELOCITY_MEDIUM); break;
			case ROWCOL(2,2): XSetVelocityMode(VELOCITY_HIGH); break;
			case ROWCOL(2,3): XSetVelocityMode(VELOCITY_OFF); break;
			case ROWCOL(2,5): XSetAftertouchMode(AFTERTOUCH_OFF); break;
			case ROWCOL(2,6): XSetAftertouchMode(AFTERTOUCH_POLY); break;
			case ROWCOL(2,7): XSetAftertouchMode(AFTERTOUCH_CHANNEL); break;
			case ROWCOL(3,5): XSetAftertouchThreshold(AFTERTOUCH_LOW); break;
			case ROWCOL(3,6): XSetAftertouchThreshold(AFTERTOUCH_MEDIUM); break;
			case ROWCOL(3,7): XSetAftertouchThreshold(AFTERTOUCH_HIGH); break;
			default: doRepaint = 0;
			}
		}
		if(doRepaint) {
			repaint();
		}
	}
}

/*
 * MENU BUTTON PRESSED
 */
void handleMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press) {
}

/*
 * PAGE ACTIVATED
 */
void handleActivate() {
	repaint();
}

/*
 * PAGE DEACTIVATED
 */
void handleDeactivate() {

}


