#!/usr/bin/env bash

make OS_MESA=1

valgrind --error-exitcode=12 --leak-check=full ./BNGine.out

if [[ $? -eq 0 ]] ; then
	# Success
else
	# Failure
fi
