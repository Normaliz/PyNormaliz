#!/usr/bin/env python
###############################################################################
# Test script for PyNormaliz                                                  #
###############################################################################
from __future__ import print_function, division

import sys
import os
import glob
import doctest

attempted = failed = 0
for filename in os.listdir(os.curdir):
    if filename.endswith('.txt'):
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


sys.exit(failed)
