#ifndef HEALTHCOMPONENT_H
#define HEALTHCOMPONENT_H

#pragma once

#include "../core/Component.h"

struct HealthComponent : Component {
	float currentHealth;
	float maxHealth;

	HealthComponent() {
		currentHealth = 5.0f;
		maxHealth = 5.0f;
	}

	void TakeDamage(float dmg) {
		currentHealth -= maxHealth;
	}

	bool IsDead() {
		return currentHealth > 0.0f;
	}
};

#endif
