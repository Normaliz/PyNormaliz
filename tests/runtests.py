#!/usr/bin/env python
###############################################################################
# Test script for PyNormaliz                                                  #
###############################################################################
from __future__ import print_function, division

import sys
import os
import glob
import doctest

# some doctest are conditional on the presence of some libraries
# and normaliz configuration
import PyNormaliz
skip = set()
if not PyNormaliz.NmzHasEAntic():
    skip.add("equation-41.txt")
    skip.add("segfault-27.txt")
    skip.add("segfault-35.txt")
    skip.add("vertex_denom-37.txt")
    skip.add("modify_cone_renf.txt")
    
if not PyNormaliz.NmzHasCocoa():
    skip.add("test_rational_cones.txt")

# run doctests
attempted = failed = 0
dir = os.path.dirname(os.path.realpath(__file__))
for filename in os.listdir(dir):
    if filename.endswith('.txt'):
        if filename in skip:
            print("Skip {}".format(filename))
            continue

        print("Doctest {}".format(filename))
        result = doctest.testfile(filename, 
                        optionflags=doctest.IGNORE_EXCEPTION_DETAIL |
                                    doctest.NORMALIZE_WHITESPACE |
                                    doctest.REPORT_NDIFF)
        print("  {} tests".format(result[1]))
        if result[0]:
            print("  {} FAILURES".format(result[0]))
        failed += result[0]
        attempted += result[1]


# unit tests
from generic_test import tests

for test in tests:
    failed += test(verbosity=2).run(repeat=10)

sys.exit(failed)
