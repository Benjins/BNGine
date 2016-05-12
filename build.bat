@echo off

cl /Od /Zi /wd4530 /wd4577 /W3 /Fobuild/ /D_CRT_SECURE_NO_WARNINGS /DBNS_DEBUG src/app/metagen_main.cpp src/metagen/*.cpp ext/CppUtils/*.cpp kernel32.lib /FeBNSMetaGen.exe

BNSMetaGen.exe

cl /Od /Zi /wd4530 /wd4577 /W3 /Fobuild/ /D_CRT_SECURE_NO_WARNINGS /DBNS_DEBUG src/app/win_main.cpp gen/*.cpp src/util/*.cpp src/gui/*.cpp src/metagen/*.cpp src/physics/*.cpp src/game/*.cpp src/assets/*.cpp src/core/*.cpp src/gfx/*.cpp ext/3dbasics/*.cpp ext/CppUtils/*.cpp kernel32.lib user32.lib Gdi32.lib Opengl32.lib /FeBNgine.exe