set -e

if [ -z "$CXX" ]; then
	echo "Could not find CXX variable, using g++ as default."
	CXX="g++"
fi


eval "$CXX  -std=c++11 src/unity/UnityBuildGenerator.cpp -o unityBuild.out"

echo "Built unity build generator."

./unityBuild.out console metagen

eval "$CXX -std=c++11 -Og -g -Wall -DBNS_DEBUG gen/UnityBuild.cpp -o BNSMetaGen.out"

echo "Built metaGen parser."

./BNSMetaGen.out

echo "Ran metaGen parser"

./unityBuild.out osmesa test

eval "$CXX -std=c++11 -Og -g -Wall -DBNS_DEBUG -DBNS_OS_MESA gen/*.cpp -lOSMesa -o BNgine_test.out"

echo "Built test"

./BNgine_test.out --compare
valgrind --suppressions=valgrind-suppressions.txt --num-callers=15 --leak-check=full --error-exitcode=12 ./BNgine_test.out
