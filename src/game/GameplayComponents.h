#ifndef GAMEPLAY_COMPONENTS_H
#define GAMEPLAY_COMPONENTS_H

#pragma once

#include "Player.h"
#include "BulletComponent.h"
#include "EnemyComponent.h"

struct GameplayComponents {
	IDTracker<PlayerComponent> players;
	IDTracker<BulletComponent> bullets;
	IDTracker<EnemyComponent> enemies;
};

#endif
