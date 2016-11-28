set -e
g++ -std=c++11 src/unity/UnityBuildGenerator.cpp -o unityBuild.out

echo "Built unity build generator."

./unityBuild.out console metagen

g++ -std=c++11  -O0 -g -Wall -DBNS_DEBUG gen/UnityBuild.cpp -o BNSMetaGen.out

echo "Built metaGen parser."

./BNSMetaGen.out

echo "Ran metaGen parser"

./rebuild.sh $1

echo "Built $1"