/*
 * LAUNCHPAD PRO SEQUENCER FRAMEWORK
 * hotchk155/2015
 *
 */
#include "app.h"
#include "xapp.h"



typedef unsigned int LED_STATE;
#define NUM_LEDS 100




typedef struct _XAPP_STATE {
	AFTERTOUCH_MODE			afterMode;
	AFTERTOUCH_THRESHOLD	afterThreshold;
	VELOCITY_MODE			velocityMode;
	byte 					midiChannel;
	byte 					keyAfterTouch[128];
	byte 					channelAfterTouch;
	LED_STATE 				ledStates[NUM_LEDS];
	byte 					blinkToggle;
	unsigned long 			milliseconds;
} XAPP_STATE;

static XAPP_STATE Me = {0};






/* ----------------------------------------------------------------------------
 *
 *
 * INTERNAL HELPER FUNCTIONS
 *
 *
 * ----------------------------------------------------------------------------
 */

static int getMenuLedIndex(MNU_BUTTON button) {
	switch(button) {
		case MNU_ARROW0:
		case MNU_ARROW1:
		case MNU_ARROW2:
		case MNU_ARROW3:
		case MNU_ARROW4:
		case MNU_ARROW5:
		case MNU_ARROW6:
		case MNU_ARROW7:
			return 89 - 10*(button - MNU_ARROW0);

		case MNU_UP:
		case MNU_DOWN:
		case MNU_LEFT:
		case MNU_RIGHT:
		case MNU_SESSION:
		case MNU_NOTE:
		case MNU_DEVICE:
		case MNU_USER:
			return 91 + (button - MNU_UP);

		case MNU_SHIFT:
		case MNU_CLICK:
		case MNU_UNDO:
		case MNU_DELETE:
		case MNU_QUANTISE:
		case MNU_DUPLICATE:
		case MNU_DOUBLE:
		case MNU_RECORD:
			return 80 - 10*(button - MNU_SHIFT);

		case MNU_RECORDARM:
		case MNU_TRACKSELECT:
		case MNU_MUTE:
		case MNU_SOLO:
		case MNU_VOLUME:
		case MNU_PAN:
		case MNU_SENDS:
		case MNU_STOPCLIP:
			return 1 + (button - MNU_RECORDARM);

		case MNU_SETUP:
		default:
			return 0;
	}
}


/*
 * HELPER FUNCTION TO PHYSICALLY UPDATE LED BASED ON
 * ACTUAL COLOUR AND EFFECTS
 */
static void updateLed(int index) {
	byte red =   (Me.ledStates[index] & 0x00FF0000) >> 16;
	byte green = (Me.ledStates[index] & 0x0000FF00) >> 8;
	byte blue =  (Me.ledStates[index] & 0x000000FF);
	if(Me.ledStates[index] & 0xFF000000) { // some kind of effect in .. effect?
		if((Me.ledStates[index] & 0xFF000000) == 0x80000000) { // blink
			if(Me.blinkToggle) {
				red = green = blue = 0xFF;
			}
		}
		else { // highlight or fade
			red = green = blue = (Me.ledStates[index] & 0x7F000000) >> 23;
		}
	}
	if(index == 0) {
		hal_plot_led(TYPESETUP, 0, red>>2, green>>2, blue>>2);
	}
	else {
		hal_plot_led(TYPEPAD, index, red>>2, green>>2, blue>>2);
	}
}

/*
 * HELPER FUNCTION TO UPDATE LEDS THAT ARE SET TO "FADE"
 */
static void updateFade() {
	/*
	for(int i=0; i<NUM_LEDS; ++i) {
		unsigned int state = Me.ledStates[i];
		if((state & 0xFF000000) && (state & 0xFF000000) != 0x80000000) {
			unsigned int z = (state & 0x7F000000);
			z >>= 1;
			z &= 0x7F000000;
			if(!z) {
				Me.ledStates[i] &= 0x00FFFFFF;
			} else {
				Me.ledStates[i] = (state & 0x80FFFFFF) | z;
			}
			updateLed(i);
		}
	}*/
}

/*
 * FORCE ALL BLINKING LEDS TO BE REPAINTED (WHEN THE STATE OF THE
 * BLINK TOGGLE IS CHANGED)
 */
static void refreshBlinkLeds() {
	for(int i=0; i<NUM_LEDS; ++i) {
		if(0x80000000 == (Me.ledStates[i] & 0xFF000000)) {
			updateLed(i);
		}
	}
}

/*
 * CANCEL AFTERTOUCH
 */
void stopAfterTouch(byte chan) {
	if(Me.afterMode == AFTERTOUCH_POLY) {
		for(int note = 0; note < 128; ++note) {
			if(Me.keyAfterTouch[note]) {
				hal_send_midi(DINMIDI, POLYAFTERTOUCH | chan, note, 0);
			}
		}
	}
	else if(Me.afterMode == AFTERTOUCH_CHANNEL) {
		hal_send_midi(DINMIDI, CHANNELAFTERTOUCH | chan, 0, 0);
	}
	for(int i=0; i<100; ++i) {
		Me.keyAfterTouch[i] = 0;
	}
	Me.channelAfterTouch = 0;
}

/* ----------------------------------------------------------------------------
 *
 *
 * EXPORTED FUNCTIONS FOR "EXTENDED" API
 *
 *
 * ----------------------------------------------------------------------------
 */

/*
 * Clear all grid and menu buttons
 */
void XCls() {
	for(int index=0; index<100; ++index) {
		Me.ledStates[index] = 0;
		hal_plot_led(TYPEPAD, index, 0, 0, 0);
	}
}

/*
 * LED ACCESS BASED ON RAW LED INDEX
 * (0 = SETUP LED)
 */
void XSetLed(int index, COLOUR colour) {
	if(index >= 0 && index < NUM_LEDS) {
		Me.ledStates[index] = colour;
		updateLed(index);
	}
}
COLOUR XGetLed(int index) {
	if(index >= 0 && index < NUM_LEDS) {
		return Me.ledStates[index];
	}
	return 0;
}
void XSetLedEffect(int index, LED_EFFECT effect, byte param) {
	if(index >= 0 && index < NUM_LEDS) {
		switch(effect) {
		case EFFECT_NONE:
			Me.ledStates[index] &= 0x00FFFFFF;
			break;
		case EFFECT_HIGHLIGHT:
			param >>= 1;
			Me.ledStates[index] &= 0x00FFFFFF;
			Me.ledStates[index] |= ((unsigned int)param) << 24;
			break;
		case EFFECT_FADE:
			Me.ledStates[index] &= 0xFFFFFFFF;
			break;
		case EFFECT_BLINK:
			Me.ledStates[index] &= 0x00FFFFFF;
			Me.ledStates[index] |= 0x8000000F;
			break;
		}
		updateLed(index);
	}
}

/*
 * GRID LED ACCESS USING 8X8 GRID COORDINATES
 */
void XSetGridLed(int row, int col, COLOUR colour) {
	if(row >= 0 && row < 8 && col >= 0 && col < 8) {
		int index = col + 81 - 10*row;
		Me.ledStates[index] = colour;
		updateLed(index);
	}
}
COLOUR XGetGridLed(int row, int col) {
	if(row >= 0 && row < 8 && col >= 0 && col < 8) {
		int index = col + 81 - 10*row;
		return Me.ledStates[index];
	}
	return 0;
}
void XSetGridLedEffect(int row, int col, LED_EFFECT effect, byte param) {
	if(row >= 0 && row < 8 && col >= 0 && col < 8) {
		int index = col + 81 - 10*row;
		XSetLedEffect(index, effect, param);
	}
}

/*
 * ACCESS TO MENU BUTTON LEDS USING MENU IDENTIFIERS
 */
void XSetMenuLed(MNU_BUTTON button, COLOUR colour) {
	if(button == MNU_SETUP) {
		Me.ledStates[0] = colour;
		updateLed(0);
	}
	else {
		int index = getMenuLedIndex(button);
		Me.ledStates[index] = colour;
		updateLed(index);
	}
}
COLOUR XGetMenuLed(MNU_BUTTON button) {
	int index = getMenuLedIndex(button);
	return (COLOUR)(Me.ledStates[index] & 0x00FFFFFF);
}

void XSetMenuLedEffect(MNU_BUTTON button, LED_EFFECT effect, byte param) {
	int index = getMenuLedIndex(button);
	XSetLedEffect(index, effect, param);
}


/*
 * SET MIDI CHANNEL FOR OUTPUT
 */
void XSetMidiChannel(byte chan) {
	if(chan >= 0 && chan < 16) {
		stopAfterTouch(Me.midiChannel);
		Me.midiChannel = chan;
	}
}
byte XGetMidiChannel() {
	return Me.midiChannel;
}

/*
 * MIDI NOTE MESSAGES
 */
void XStartNote(byte note, byte velocity) {
	hal_send_midi(DINMIDI, NOTEON | Me.midiChannel, note, velocity);
}
void XStopNote(byte note) {
	hal_send_midi(DINMIDI, NOTEON | Me.midiChannel, note, 0);
}

/*
 * UPDATE AFTERTOUCH
 */
void XAfterTouch(byte note, byte pressure) {
	if(note >= 0 && note < 128) {
		Me.keyAfterTouch[note] = pressure;
		if(Me.afterMode == AFTERTOUCH_POLY) {
			hal_send_midi(DINMIDI, POLYAFTERTOUCH | Me.midiChannel, note, pressure);
		}
		if(Me.afterMode == AFTERTOUCH_CHANNEL) {
			char maxPressure = 0;
			for(int i=0; i<128; ++i) {
				if(Me.keyAfterTouch[i] > maxPressure) {
					maxPressure = Me.keyAfterTouch[i];
				}
			}
			if(maxPressure != Me.channelAfterTouch) {
				Me.channelAfterTouch = maxPressure;
				hal_send_midi(DINMIDI, CHANNELAFTERTOUCH | Me.midiChannel, Me.channelAfterTouch, 0);
			}
		}
	}
}


/* ----------------------------------------------------------------------------
 *
 *
 * CALLBACK INTERFACE TO THE LAUNCHPAD PRO API
 *
 *
 * ----------------------------------------------------------------------------
 */

/*
 * Handler for when a button is pressed on the Launchpad
 */
void app_surface_event(u8 type, u8 index, u8 value)
{
	//TODO: Apply velocity scaling
	BUTTON_ACTION action = value ? BUTTON_PRESS : BUTTON_RELEASE;
	switch (type)
	{
		case  TYPEPAD:
			if(index < 10) {
				// bottom row of menu buttons
				theApp.menuButton(MNU_RECORDARM + (index-1), action, value);
			}
			else if(index < 89) {
				switch(index%10) {
				case 0:
					// left side column of menu buttons
					theApp.menuButton(MNU_SHIFT + (8-index/10), action, value);
					break;
				case 9:
					// right side column of menu buttons
					theApp.menuButton(MNU_ARROW0 + (8-index/10), action, value);
					break;
				default:
					// grid
					theApp.gridPress(8-index/10, index%10-1, action, value);
					break;
				}
			}
			else if(index < 99) {
				// top row of menu button
				theApp.menuButton(MNU_UP+ (index-91), action, value);
			}
			break;

		case  TYPESETUP:
			// setup button
			theApp.menuButton(MNU_SETUP, action, value);
			break;
	}
}

/*
 * Handler for incoming MIDI event
 */
void app_midi_event(u8 port, u8 status, u8 d1, u8 d2)
{
}

/*
 * Handler for incoming MIDI SYSEX event
 */
void app_sysex_event(u8 port, u8 * data, u16 count)
{
}

/*
 * Handler for incoming MIDI AFTERTOUCH event
  */
void app_aftertouch_event(u8 index, u8 value)
{
	//TODO: Apply aftertouch threshold and scaling

	if(index >= 10 && index < 89) {
		switch(index%10) {
		case 0:
		case 9:
			break;
		default:
			theApp.gridPress(8-index/10, index%10-1, BUTTON_AFTERTOUCH, value);
			break;
		}
	}
}

/*
 * Handler for MIDI cable being plugged/unplugged
 */
void app_cable_event(u8 type, u8 value)
{
}

/*
 * Handler for millisecond timer events
 */
void app_timer_event()
{
	++Me.milliseconds;
	if(!(Me.milliseconds & 0xFF)) {
		updateFade();
	}
	if(!(Me.milliseconds & 0x1FF)) {
		Me.blinkToggle = !Me.blinkToggle;
		refreshBlinkLeds();
	}
}

/*
 * Handler for initialisation
 */
void app_init()
{
	Me.afterMode = AFTERTOUCH_POLY;
	Me.afterThreshold = AFTERTOUCH_MEDIUM;
	Me.velocityMode = VELOCITY_MEDIUM;
	Me.midiChannel = 0;

	theApp.init();
}
