set -e

g++ -std=c++11 src/unity/UnityBuildGenerator.cpp -o unityBuild

echo "Built unity build generator."

./unityBuild console metagen

g++ -Og -g -Wall -DBNS_DEBUG gen/UnityBuild.cpp -o BNSMetaGen

echo "Built metaGen parser."

./BNSMetaGen.exe

echo "Ran metaGen parser"

unityBuild.exe osmesa test

g++ -Og -g -Wall -DBNS_DEBUG gen/*.cpp -o BNgine_$1

echo "Built $1"

./BNGine.out --compare
valgrind --suppressions=valgrind-suppressions.txt --num-callers=15 --leak-check=full --error-exitcode=12 ./BNGine.out
