#This performs a quick rebuild of game code.  Does not build unity build generator, or build or run meta code generator.

unityBuild.exe x11 $1

g++ -Og -g -Wall -DBNS_DEBUG gen/*.cpp -o BNgine_$1
