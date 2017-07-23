#include "GameConsole.h"

#include "../core/Scene.h"
#include "../util/Serialization.h"

struct ConsoleCommandBinding {
	const char* name;
	ConsoleCommandFunc* func;
};

void Console_EchoArgs(Vector<SubString>* args, String* outString) {
	// TODO: Better way to do this?
	StringStackBuffer<256> buff;
	BNS_VEC_FOREACH(*args) {
		buff.AppendFormat("%.*s", ptr->length, ptr->start);
	}

	*outString = buff.buffer;
}

void Console_Add(Vector<SubString>* args, String* outString) {
	if (args->count != 2) {
		*outString = "Error: add expects two args";
	}
	else {
		int num1 = Atoi(args->data[0].start);
		int num2 = Atoi(args->data[1].start);

		*outString = Itoa(num1 + num2);
	}
}

void Console_Reload(Vector<SubString>* args, String* outString) {
	GlobalScene->ReloadAssets();
}

void Console_SetVariable(Vector<SubString>* args, String* outString) {
	if (args->count != 2) {
		*outString = "Error: set expects two args";
	}
	else {
		if (ConfigVariable* configVar = globalConfigTable.GetVar(args->data[0])) {
			if (configVar->value.IsConfigFloat()) {
				*configVar->value.AsConfigFloat() = Atof(args->data[1].start);
			}
			else if (configVar->value.IsConfigInt()) {
				*configVar->value.AsConfigInt() = Atoi(args->data[1].start);
			}
			else if (configVar->value.IsConfigBool()) {
				*configVar->value.AsConfigBool() =  (args->data[1].start != "F");
			}
			else if (configVar->value.IsConfigString()) {
				*configVar->value.AsConfigString() = args->data[1].start;
			}
			else {
				// TODO: Othe types
				ASSERT(false);
			}
		}
		else {
			*outString = "Error: var does not exist";
		}
	}
}

void Console_ReloadAsset(Vector<SubString>* args, String* outString) {
	if (args->count != 1) {
		*outString = "Error: reload_asset expects one arg";
	}
	else {
		String assetName = args->data[0];
		GlobalScene->res.ReloadSingleAsset(assetName.string);
	}
}

ConsoleCommandBinding defaultBindings[] = {
	{ "echo", Console_EchoArgs },
	{ "add", Console_Add },
	{ "reload", Console_Reload },
	{ "set", Console_SetVariable },
	{ "reload_asset", Console_ReloadAsset }
};

void GameConsole::InitCommandBindings() {
	BNS_ARRAY_FOR_I(defaultBindings) {
		bindings.Insert(defaultBindings[i].name, defaultBindings[i].func);
	}
}

void GameConsole::Render(GuiSystem* gui) {
	if (shouldDisplayConsole) {
		const float lineHeight = 12.0f;

		float startX = 0;
		float width = GlobalScene->cam.widthPixels;
		float startY = GlobalScene->cam.heightPixels * (1 - consoleHeightRatio);
		float height = GlobalScene->cam.heightPixels * consoleHeightRatio;
		gui->ColoredBox(startX, startY, width, height, Vector4(0.4f, 0.4f, 0.4f, 0.7f));

		String newLine = gui->TextInput(currentLine, 0, lineHeight, startX, startY, width);
		if (newLine != currentLine && newLine != "") {
			pastLines.PushBack(newLine);

			Vector<SubString> parts;
			SplitStringIntoParts(newLine, " ", &parts, true);
			if (parts.count > 0) {
				String cmd = parts.data[0];
				ConsoleCommandFunc* func = nullptr;
				if (cmd == "clear") {
					pastLines.Clear();
				}
				else if (bindings.LookUp(cmd, &func)) {
					parts.Remove(0);
					String response;
					ASSERT(func != nullptr);
					func(&parts, &response);
					if (response != "") {
						pastLines.PushBack(response);
					}
				}
				else {
					pastLines.PushBack("Unknown command");
				}
			}
			else {
				ASSERT(false);
			}

			currentLine = "";
		}

		float y = startY + lineHeight;
		for (int i = pastLines.count - 1; i >= 0; i--) {
			gui->DrawTextLabel(pastLines.data[i].string, IDHandle<BitmapFont>(0), lineHeight, startX, y);
			y += lineHeight;
		}

		
	}
}
