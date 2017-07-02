#ifndef GUI_HEALTH_FORM_H
#define GUI_HEALTH_FORM_H

#pragma once

#include "GuiSystem.h"

#include "../core/Scene.h"

/*[GuiForm]*/
bool GuiHealthForm(GuiSystem* sys, float w, float h, float health, float maxHealth) {

	sys->SimpleSlider(health / maxHealth, w - 300, h - 100, 150, 60);
	sys->DrawTextLabel(StringStackBuffer<64>("%.2f/%.2f", health, maxHealth).buffer,
		IDHandle<BitmapFont>(0), 14, w - 120, h - 100, 100, 60);

	return true;
}

/*[GuiForm]*/
bool GuiIPConnectForm(GuiSystem* sys, float w, float h) {
	// Bah...
	static String remoteAddr = "127.0.0.1";
	static String remotePort = "4554";
	static String  localPort = "4554";

	sys->ColoredBox(10, h - 160, 320, 150, Vector4(0.2f, 0.2f, 0.2f, 0.5f));
	remoteAddr = sys->TextInput(remoteAddr, 0, 12,  20, h - 100, 90);
	remotePort = sys->TextInput(remotePort, 0, 12, 120, h - 100, 90);
	localPort  = sys->TextInput(localPort,  0, 12, 220, h - 100, 90);

	if (sys->TextButton("Local Setup", 0, 12, 20, h - 140, 90, 30)) {
		short localPortNum = (short)Atoi(localPort.string);
		GlobalScene->net.Initialize(localPortNum);
	}

	if (sys->TextButton("Remote Connect", 0, 12, 120, h - 140, 90, 30)) {
		short remotePortNum = (short)Atoi(remotePort.string);
		IPV4Addr remoteIpAddr = IPV4Addr(remoteAddr.string, remotePortNum);
		GlobalScene->net.OpenNewConnection(remoteIpAddr);
	}

	return true;
}

/*[GuiForm]*/
bool GuiCanDoForm(GuiSystem* sys, float w, float h, float xp, int* outLevel) {


	return true;
}

#endif
