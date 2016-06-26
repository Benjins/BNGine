#ifndef GAMEPLAY_COMPONENTS_H
#define GAMEPLAY_COMPONENTS_H

#pragma once

#include "Player.h"

struct GameplayComponents {
	IDTracker<PlayerComponent> players;
};

#endif
