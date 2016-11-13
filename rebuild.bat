@echo off

:: This performs a quick rebuild of game code.  Does not build unity build generator, or build or run meta code generator.

unityBuild.exe win32 %1

cl /Od /Zi /wd4530 /wd4577 /wd4244 /W3 /Fobuild/ /D_CRT_SECURE_NO_WARNINGS /DBNS_DEBUG gen/*.cpp kernel32.lib user32.lib Gdi32.lib Opengl32.lib /FeBNgine_%1.exe
