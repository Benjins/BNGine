#ifndef GUIBUTTON_H
#define GUIBUTTON_H

#pragma once

#include "GuiRect.h"
#include "GuiContent.h"
#include "../../gen/Actions.h"

enum GuiButtonState {
	GBS_Off,
	GBS_Down
};

struct GuiButton : IDBase {
	uint32 rect;
	GuiContent content;

	GuiButtonState state;

	Action onClick;

	GuiButton() {
		rect = -1;
		state = GBS_Off;
	}

	GuiButton(const GuiButton& orig) 
	: content(orig.content) {
		rect = orig.rect;
		state = orig.state;
		onClick = orig.onClick;
	}
};


#endif
