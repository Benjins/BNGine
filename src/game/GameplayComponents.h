#ifndef GAMEPLAY_COMPONENTS_H
#define GAMEPLAY_COMPONENTS_H

#pragma once

#include "Player.h"
#include "BulletComponent.h"
#include "EnemyComponent.h"
#include "HealthComponent.h"
#include "WaterVolumeComponent.h"
#include "LadderComponent.h"
#include "PrefabInstanceComponent.h"
#include "PatrolPoint.h"

struct GameplayComponents {
	//TODO: Automate
	IDTracker<PlayerComponent> players;
	IDTracker<BulletComponent> bullets;
	IDTracker<EnemyComponent> enemies;
	IDTracker<HealthComponent> healthComps;
	IDTracker<WaterVolumeComponent> waterComps;
	IDTracker<LadderComponent> ladderComps;
	IDTracker<PrefabInstanceComponent> prefabInsts;
	IDTracker<PatrolPointComponent> patrolPoints;
};

#endif
