set -e

make genCode
make -B BNGine.out
make -B OS_MESA=1 BNGine.out
#./BNGine.out --compare
valgrind --num-callers=15 --leak-check=full --error-exitcode=12 ./BNGine.out

#cppcheck --force --inline-suppr --enable=all src/*/*.cpp --quiet --verbose --template '{file},{line},{severity},{id},{message}' --suppress=unusedFunction --suppress=purgedConfiguration --suppress=noExplicitConstructor --suppress=unusedStructMember --suppress=missingIncludeSystem --suppress=cstyleCast

