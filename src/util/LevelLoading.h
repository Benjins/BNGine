#ifndef LEVELLOADING_H
#define LEVELLOADING_H

#pragma once

#include <type_traits>
#include "../../ext/CppUtils/vector.h"
#include "../../ext/CppUtils/idbase.h"

template<typename T>
void LoadVectorToIDTracker(const Vector<T>& from, IDTracker<T>& to) {
	to.SetSize(from.count);
	for (int i = 0; i < from.count; i++) {
		new (&to.vals[i]) T();
		to.vals[i] = from.data[i];
	}
	to.currentCount = from.count;

	uint32 maxId = 0;
	BNS_VEC_FOREACH(from) {
		maxId = BNS_MAX(maxId, ptr->id);
	}

	to.currentMaxId = maxId + 1;
}

template<typename T>
void LoadIDTrackerToVector(const IDTracker<T>& from, Vector<T>& to) {
	to.EnsureCapacity(from.currentCount);

	to.Clear();
	for (int i = 0; i < from.currentCount; i++) {
		new (&to.data[i]) T();
	}

	for (int i = 0; i < from.currentCount; i++) {
		to.data[i] = from.vals[i];
	}

	to.count = from.currentCount;
}

#endif
