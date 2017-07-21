#ifndef GAME_CONSOLE_H
#define GAME_CONSOLE_H

#pragma once

#include "../../ext/CppUtils/strings.h"
#include "../../ext/CppUtils/vector.h"

#include "GuiSystem.h"

typedef void (ConsoleCommandFunc)(Vector<SubString>* args, String* outString);

struct GameConsole {
	bool shouldDisplayConsole;

	String currentLine;
	Vector<String> pastLines;

	float consoleHeightRatio;

	StringMap<ConsoleCommandFunc*> bindings;

	GameConsole() {
		shouldDisplayConsole = false;
		consoleHeightRatio = 0.7f;

		InitCommandBindings();
	}

	void InitCommandBindings();

	void Render(GuiSystem* gui);
};


#endif
