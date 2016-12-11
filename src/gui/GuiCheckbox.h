#ifndef GUICHECKBOX_H
#define GUICHECKBOX_H

#pragma once

#include "GuiRect.h"
#include "../../gen/Actions.h"

struct GuiCheckbox : IDBase {
	IDHandle<GuiRect> rect;

	bool isChecked;
	Action onChange;

	GuiCheckbox() {
		isChecked = false;
	}

	GuiCheckbox(const GuiCheckbox& orig) {
		rect = orig.rect;
		onChange = orig.onChange;
		isChecked = false;
	}
};


#endif
