set -e

make genCode
make -B BNGine.out
make -B OS_MESA=1 BNGine.out
./BNGine.out --compare
valgrind --suppressions=valgrind-suppressions.txt --num-callers=15 --leak-check=full --error-exitcode=12 ./BNGine.out
