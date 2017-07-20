#ifndef PATROL_POINT_H
#define PATROL_POINT_H

#pragma once

#include "../core/Component.h"

struct PatrolPointComponent : Component {
	void EditorGui(bool isSelected);
};

#endif
