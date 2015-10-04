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


CONTROLLER_STATE Controller;

void implInit() {
	PianoMode.init();
	PianoMode.repaint();
}
void implDone() {

}
void implEvent(XAPP_EVENT event) {

}
void implGridPress(byte row, byte col, BUTTON_ACTION type, byte press) {
	PianoMode.gridPress(row, col, type, press);
}
void implMenuButton(MNU_BUTTON which, BUTTON_ACTION type, byte press) {
	PianoMode.menuButton(which, type, press);
}
void implMidiMsg(MIDI_MSG *msg) {

}
void implSysExMsg(byte *data, int count) {

}
