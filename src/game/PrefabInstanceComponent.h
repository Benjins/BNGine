#ifndef PREFABINSTANCE_H
#define PREFABINSTANCE_H

struct Prefab;

struct PrefabInstanceComponent : Component {
	IDHandle<Prefab> prefab;
};

#endif
