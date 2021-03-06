#ifndef GUIRECT_H
#define GUIRECT_H

#pragma once

#include "../../ext/CppUtils/idbase.h"
#include "../../ext/3dbasics/Vector2.h"

struct GuiRect : IDBase {
	union {
		struct{
			float x;
			float y;
		};

		Vector2 position;
	};

	union {
		struct {
			float width;
			float height;
		};

		Vector2 size;
	};

	IDHandle<GuiRect> parent;
	bool isDirty;
	bool clipChildren;

	GuiRect() {
		isDirty = true;
		clipChildren = false;
	}

	GuiRect(const GuiRect& orig) : IDBase(orig) {
		parent = orig.parent;
		isDirty = orig.isDirty;
		clipChildren = orig.clipChildren;

		position = orig.position;
		size = orig.size;
	}

	GuiRect GetFinalRect();

	bool ContainsPoint(Vector2 pt) {
		return (pt.x > x && pt.y > y && pt.x < x + width && pt.y < y + height);
	}
};

#endif
