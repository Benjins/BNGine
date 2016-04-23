#ifndef LEVELLOADING_H
#define LEVELLOADING_H

#pragma once

#include <type_traits>
#include "../../ext/CppUtils/vector.h"
#include "../../ext/CppUtils/idbase.h"

template<typename T>
void LoadVectorToIDTracker(const Vector<T>& from, IDTracker<T>& to) {
	to.SetSize(from.count);
	MemCpy(to.vals, from.data, sizeof(T)*from.count);
	to.currentCount = from.count;
	if (from.count > 0) {
		to.currentMaxId = from.data[from.count - 1].id + 1;
	}
}

#endif
