make -B BNGine.out
make -B OS_MESA=1 BNGine.out
valgrind --quiet --leak-check=full --error-exitcode=12 ./BNGine.out

