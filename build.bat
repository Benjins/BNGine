@echo off

::C4530 C4577

cl /Od /Zi /wd4530 /wd4577 /W3 /DBNS_DEBUG src/app/win_main.cpp src/core/*.cpp src/gfx/*.cpp ext/3dbasics/*.cpp ext/CppUtils/*.cpp kernel32.lib user32.lib Gdi32.lib Opengl32.lib /FeBNgine.exe