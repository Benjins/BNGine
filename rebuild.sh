#This performs a quick rebuild of game code.  Does not build unity build generator, or build or run meta code generator.

set -e

./unityBuild.out x11 $1

g++ -std=c++11  -O0 -g -Wall -DBNS_DEBUG gen/*.cpp -lX11 -lGL -o BNgine_$1.out
