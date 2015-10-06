#include "app.h"
#include "app_defs.h"
#include "xapp.h"
#include "control_surface.h"

static void implInit();
static void implDone();
static void implEvent(XAPP_EVENT event);
static void implGridPress(byte row, byte col, BUTTON_ACTION type, byte press);
static void implMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press);
static void implMidiMsg(MIDI_MSG *msg);
static void implSysExMsg(byte *data, int count);

XAPP_HANDLERS theApp = {implInit, implDone, implEvent, implGridPress, implMenuButton, implMidiMsg, implSysExMsg};

typedef struct _CONTROLLER_STATE {
	CTRL_MODE controllerMode;
	PAGE_HANDLERS *currentPage;
} CONTROLLER_STATE;

#pragma GCC diagnostic ignored "-Wmissing-braces"
static CONTROLLER_STATE Me = {0};
#pragma GCC diagnostic warning "-Wmissing-braces"




static void implInit() {
	NoteMode.init();
	KitMode.init();
	SetupMode.init();
	Me.controllerMode = CTRL_MODE_NOTE;
	Me.currentPage = &NoteMode;
	Me.currentPage->repaint();
}
static void implDone() {

}
static void implEvent(XAPP_EVENT event) {

}
static void implGridPress(byte row, byte col, BUTTON_ACTION type, byte press) {
	Me.currentPage->gridPress(row, col, type, press);
}

static void implMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press) {
	if(which == MNU_SETUP) {
		if(type == BUTTON_PRESS) {
			if(Me.currentPage == &SetupMode) {
				switch(Me.controllerMode) {
				case CTRL_MODE_KIT:
					Me.currentPage = &KitMode;
					break;
				case CTRL_MODE_NOTE:
				default:
					Me.currentPage = &NoteMode;
					break;
				}
			}
			else {
				Me.currentPage = &SetupMode;
			}
			Me.currentPage->repaint();
		}
	} else {
		Me.currentPage->menuButton(which, type, press);
	}
}
static void implMidiMsg(MIDI_MSG *msg) {

}
static void implSysExMsg(byte *data, int count) {

}





CTRL_MODE CtrlGetMode() {
	return Me.controllerMode;
}
void CtrlSetMode(CTRL_MODE mode) {
	Me.controllerMode = mode;
}
