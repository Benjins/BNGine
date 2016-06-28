#ifndef GAMEPLAY_COMPONENTS_H
#define GAMEPLAY_COMPONENTS_H

#pragma once

#include "Player.h"
#include "BulletComponent.h"

struct GameplayComponents {
	IDTracker<PlayerComponent> players;
	IDTracker<BulletComponent> bullets;
};

#endif
