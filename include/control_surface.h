/*
 * control_surface.h
 *
 *  Created on: 3 Oct 2015
 *      Author: Jason
 */

#ifndef CONTROL_SURFACE_H_
#define CONTROL_SURFACE_H_

typedef struct _CONTROLLER_STATE {
	byte dummy;
} CONTROLLER_STATE;

extern CONTROLLER_STATE Controller;

typedef void(*PageInitFunc)();
typedef void(*PageGridPressFunc)(byte row, byte col, BUTTON_ACTION type, byte press);
typedef void(*PageMenuButtonFunc)(MNU_BUTTON which, BUTTON_ACTION type, byte press);
typedef void(*PageRepaintFunc)();

typedef struct _PAGE_HANDLERS {
	PageInitFunc init;
	PageGridPressFunc gridPress;
	PageMenuButtonFunc menuButton;
	PageRepaintFunc repaint;
} PAGE_HANDLERS;

extern PAGE_HANDLERS PianoMode;


#endif /* CONTROL_SURFACE_H_ */
