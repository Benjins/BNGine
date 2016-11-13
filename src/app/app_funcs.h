#ifndef APPFUNCS_H
#define APPFUNCS_H

void AppPreInit(int argc, char** argv);

void AppPostInit(int argc, char** argv);

bool AppUpdate(int argc, char** argv);

void AppShutdown(int argc, char** argv);

void AppMouseMove(int x, int y);

void AppMouseUp(int button);

void AppMouseDown(int button);

void AppKeyUp(unsigned char key);

void AppKeyDown(unsigned char key);

void AppSetWindowSize(int w, int h);


#endif
