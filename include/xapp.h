
#ifndef XAPP_H_
#define XAPP_H_

/*
 * Some basic definitions
 */
#define NULL ((void*)0L)
#define TRUE 1
#define FALSE 0
typedef unsigned char byte;

/*
 * Define some basic colours
 */
typedef unsigned int COLOUR;
#define COLOUR_NONE		0x000000U
#define COLOUR_RED 		0xFF0000U
#define COLOUR_GREEN 	0x00FF00U
#define COLOUR_BLUE 	0x0000FFU
#define COLOUR_YELLOW 	(COLOUR_RED|COLOUR_GREEN)
#define COLOUR_MAGENTA 	(COLOUR_RED|COLOUR_BLUE)
#define COLOUR_CYAN 	(COLOUR_GREEN|COLOUR_BLUE)
#define COLOUR_WHITE	(COLOUR_RED|COLOUR_GREEN|COLOUR_BLUE)


typedef enum _AFTERTOUCH_MODE {
	AFTERTOUCH_OFF,
	AFTERTOUCH_POLY,
	AFTERTOUCH_CHANNEL,
} AFTERTOUCH_MODE;

typedef enum _AFTERTOUCH_THRESHOLD {
	AFTERTOUCH_LOW,
	AFTERTOUCH_MEDIUM,
	AFTERTOUCH_HIGH
} AFTERTOUCH_THRESHOLD;

typedef enum _VELOCITY_MODE {
	VELOCITY_OFF,
	VELOCITY_LOW,
	VELOCITY_MEDIUM,
	VELOCITY_HIGH
} VELOCITY_MODE;

/*
 * Enumeration of the menu buttons around the outside of
 * the launchpad grid
 */
typedef enum {
	MNU_ARROW0 	= 0,
	MNU_ARROW1,
	MNU_ARROW2,
	MNU_ARROW3,
	MNU_ARROW4,
	MNU_ARROW5,
	MNU_ARROW6,
	MNU_ARROW7,

	MNU_UP,
	MNU_DOWN,
	MNU_LEFT,
	MNU_RIGHT,
	MNU_SESSION,
	MNU_NOTE,
	MNU_DEVICE,
	MNU_USER,

	MNU_SHIFT,
	MNU_CLICK,
	MNU_UNDO,
	MNU_DELETE,
	MNU_QUANTISE,
	MNU_DUPLICATE,
	MNU_DOUBLE,
	MNU_RECORD,

	MNU_RECORDARM,
	MNU_TRACKSELECT,
	MNU_MUTE,
	MNU_SOLO,
	MNU_VOLUME,
	MNU_PAN,
	MNU_SENDS,
	MNU_STOPCLIP,

	MNU_SETUP

} MNU_BUTTON;

typedef enum {
	BUTTON_PRESS,
	BUTTON_RELEASE,
	BUTTON_AFTERTOUCH
} BUTTON_ACTION;

typedef enum {
	EFFECT_NONE,		// no effect
	EFFECT_HIGHLIGHT,	// show in white with specified intensity
	EFFECT_FADE,		// fade out from max intensity white
	EFFECT_BLINK,		// blink white
} LED_EFFECT;

typedef enum {
	EVENT_NONE = 0,
	EVENT_ACTIVATE,
	EVENT_DEACTIVATE
} XAPP_EVENT;

typedef struct _MIDI_MSG {
	byte chan;
	byte type;
	byte param1;
	byte param2;
} MIDI_MSG;

typedef void(*XInitFunc)();
typedef void(*XDoneFunc)();
typedef void(*XEventFunc)(XAPP_EVENT event);
typedef void(*XGridPressFunc)(byte row, byte col, BUTTON_ACTION type, byte press);
typedef void(*XMenuButtonFunc)(MNU_BUTTON which, BUTTON_ACTION type, byte press);
typedef void(*XMidiMsgFunc)(MIDI_MSG *msg);
typedef void(*XSysExMsgFunc)(byte *data, int count);





typedef struct _XAPP_HANDLERS {
	XInitFunc			init;
	XDoneFunc			done;
	XEventFunc			event;
	XGridPressFunc		gridPress;
	XMenuButtonFunc		menuButton;
	XMidiMsgFunc		midiMsg;
	XSysExMsgFunc		sysEx;
} XAPP_HANDLERS;

extern XAPP_HANDLERS theApp;


extern void XCls();
extern void XSetLed(int index, COLOUR colour);
extern COLOUR XGetLed(int index);
extern void XSetLedEffect(int index, LED_EFFECT effect, byte param);
extern void XSetGridLed(int row, int col, COLOUR colour);
extern COLOUR XGetGridLed(int row, int col);
extern void XSetGridLedEffect(int row, int col, LED_EFFECT effect, byte param);
extern void XSetMenuLed(MNU_BUTTON button, COLOUR colour);
extern COLOUR XGetMenuLed(MNU_BUTTON button);
extern void XSetMenuLedEffect(MNU_BUTTON button, LED_EFFECT effect, byte param);
extern void XSetMidiChannel(byte chan);
extern byte XGetMidiChannel();
extern void XStartNote(byte note, byte velocity);
extern void XStopNote(byte note);
extern void XAfterTouch(byte note, byte pressure);
extern AFTERTOUCH_MODE XGetAftertouchMode();
extern void XSetAftertouchMode(AFTERTOUCH_MODE afterMode);
extern AFTERTOUCH_THRESHOLD XGetAftertouchThreshold();
extern void XSetAftertouchThreshold(AFTERTOUCH_THRESHOLD afterThreshold);
extern VELOCITY_MODE XGetVelocityMode();
extern void XSetVelocityMode(VELOCITY_MODE velocityMode);

#endif /* XAPP_H_ */
