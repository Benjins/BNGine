#ifndef GUI_HEALTH_FORM_H
#define GUI_HEALTH_FORM_H

#pragma once

#include "GuiSystem.h"

/*[GuiForm]*/
bool GuiHealthForm(GuiSystem* sys, float w, float h, float health, float maxHealth) {

	sys->SimpleSlider(health / maxHealth, w - 300, h - 100, 150, 60);
	sys->DrawTextLabel(StringStackBuffer<64>("%.2f/%.2f", health, maxHealth).buffer,
		IDHandle<BitmapFont>(0), 14, w - 120, h - 100, 100, 60);

	return true;
}


/*[GuiForm]*/
bool GuiCanDoForm(GuiSystem* sys, float w, float h, float xp, int* outLevel) {


	return true;
}

#endif
