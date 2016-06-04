@echo off

cl /Fobuild/ src/unity/UnityBuildGenerator.cpp /FeunityBuild.exe

echo Built unity build generator.

unityBuild.exe -mg

cl /Od /Zi /wd4530 /wd4577 /W3 /Fobuild/ /D_CRT_SECURE_NO_WARNINGS /DBNS_DEBUG src/app/metagen_main.cpp gen/UnityBuild.cpp kernel32.lib /FeBNSMetaGen.exe

echo Built metaGen parser

BNSMetaGen.exe

echo Ran metaGen parser

unityBuild.exe -e

cl /Od /Zi /wd4530 /wd4577 /W3 /Fobuild/ /D_CRT_SECURE_NO_WARNINGS /DBNS_DEBUG src/app/win_editor.cpp gen/*.cpp kernel32.lib user32.lib Gdi32.lib Opengl32.lib /FeBNgine_editor.exe