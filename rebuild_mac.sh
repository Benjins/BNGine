#This performs a quick rebuild of game code.  Does not build unity build generator, or build or run meta code generator.

set -e

if [ -z "$CXX" ]; then
	echo "Could not find CXX variable, using clang++ as default."
	CXX="clang++"
fi

./unityBuild.out osx $1

eval "$CXX -std=c++11  -O0 -g -DBNS_DEBUG src/platform/osx_entry_main.cpp gen/UnityBuild.cpp -framework Carbon -framework OpenGL -framework AGL -pthread -o BNgine_$1 -arch i386"

eval "rm -rf BNgine_$1.app"
eval "mkdir BNgine_$1.app"
eval "mkdir BNgine_$1.app/Contents"
eval "mkdir BNgine_$1.app/Contents/MacOS"
eval "cp BNgine_$1 BNgine_$1.app/Contents/MacOS/"
