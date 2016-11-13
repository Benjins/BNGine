g++ -std=c++11 src/unity/UnityBuildGenerator.cpp -o unityBuild

echo "Built unity build generator."

./unityBuild console metagen

g++ -Og -g -Wall -DBNS_DEBUG gen/UnityBuild.cpp -o BNSMetaGen

echo "Built metaGen parser."

./BNSMetaGen.exe

echo "Ran metaGen parser"

./rebuild.sh $1

echo "Built $1"