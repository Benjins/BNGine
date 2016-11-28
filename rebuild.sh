#This performs a quick rebuild of game code.  Does not build unity build generator, or build or run meta code generator.

set -e

if [ -z "$CXX" ]; then
	echo "Could not find CXX variable, using g++ as default."
	CXX="g++"
fi

./unityBuild.out x11 $1

eval "$CXX -std=c++11  -O0 -g -Wall -DBNS_DEBUG gen/*.cpp -lX11 -lGL -o BNgine_$1.out"
