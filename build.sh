set -e

if [ -z "$CXX" ]; then
	echo "Could not find CXX variable, using g++ as default."
	CXX="g++"
fi


eval "$CXX -std=c++11 src/unity/UnityBuildGenerator.cpp -o unityBuild.out"

echo "Built unity build generator."

./unityBuild.out console metagen

eval "$CXX -std=c++11  -O0 -g -Wall -DBNS_DEBUG gen/UnityBuild.cpp -lpthread -o BNSMetaGen.out"

echo "Built metaGen parser."

./BNSMetaGen.out

echo "Ran metaGen parser"

./rebuild.sh $1

echo "Built $1"