#!/bin/bash

# For this command valgrind Version 3.10.1 was in use.
# Install waver with 'make install' and run the valgrind check.
# Pass the arguments -b -c -n to this script.

# valgrind to check for memory leaks
valgrind --leak-check=full --show-reachable=yes waver -b ${1} -c ${2} -n ${3} -s -t 4 > cur_valgrind_rpt.txt 2>&1

# valgrind (helgrind) to detect thread conflict errors
valgrind --tool=helgrind --log-file=cur_helgrind_rpt.txt waver -b ${1} -c ${2} -n ${3} -s -t 4

