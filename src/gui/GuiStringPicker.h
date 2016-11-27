#ifndef GUISTRINGPICKER_H
#define GUISTRINGPICKER_H

#include "GuiRect.h"

#include "../../ext/CppUtils/strings.h"
#include "../../ext/CppUtils/vector.h"
#include "../../gen/Actions.h"

enum GuiSringPickerOptions {
	GSPO_SingleChoice,
	GSPO_MultipleChoice,
	GSPO_Count
};

struct GuiStringPicker : IDBase {
	int rect;

	Vector<String> choices;
	GuiSringPickerOptions options;

	int choice;

	bool witnessedMouseDown;

	Action onSelect;

	GuiStringPicker() {
		options = GSPO_MultipleChoice;
		rect = -1;
		choice = -1;
		witnessedMouseDown = false;
	}

	GuiStringPicker(const GuiStringPicker& orig) {
		rect = orig.rect;
		options = orig.options;
	}
};

#endif
