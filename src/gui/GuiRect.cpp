#include "GuiRect.h"


GuiRect GuiRect::GetFinalRect(){
	if (parent == -1) {
		return *this;
	}
	else {
		GuiRect* rect = GlobalScene->gui.rects.GetById(parent);
		ASSERT(rect != nullptr);

		GuiRect parentRect = rect->GetFinalRect();
		GuiRect finalRect = *this;

		finalRect.position = finalRect.position + parentRect.position;

		return finalRect;
	}
}