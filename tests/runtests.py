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
        result = doctest.testfile(filename, 
                        optionflags=doctest.IGNORE_EXCEPTION_DETAIL |
                                    doctest.NORMALIZE_WHITESPACE |
                                    doctest.REPORT_NDIFF)
        failed += result[0]
        attempted += result[1]

sys.exit(failed)
