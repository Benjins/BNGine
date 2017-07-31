#ifndef NAV_SYSTEM_H
#define NAV_SYSTEM_H

#pragma once

#include "NavMesh.h"

struct NavSystem {
	IDTracker<NavMesh> navMeshes;

	IDHandle<NavMesh> currentNavMesh;

	NavSystem() {
		currentNavMesh.id = navMeshes.CreateAndAdd()->id;
	}
};


#endif
