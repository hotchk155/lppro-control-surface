/*
 * control_surface.h
 *
 *  Created on: 3 Oct 2015
 *      Author: Jason
 */

#ifndef CONTROL_SURFACE_H_
#define CONTROL_SURFACE_H_


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


typedef enum _CTRL_MODE {
	CTRL_MODE_NOTE,
	CTRL_MODE_KIT
} CTRL_MODE;


extern PAGE_HANDLERS SetupMode;
extern PAGE_HANDLERS NoteMode;
extern PAGE_HANDLERS KitMode;

CTRL_MODE CtrlGetMode();
void CtrlSetMode(CTRL_MODE mode);


#endif /* CONTROL_SURFACE_H_ */
