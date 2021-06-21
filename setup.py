#!/usr/bin/env python

from setuptools import setup
from distutils.core import Extension
from distutils.cmd import Command
from distutils.command.build_ext import build_ext as _build_ext

import sys, os, subprocess

from copy import copy

has_extra_dir = False

try:
    normaliz_dir = os.environ["NORMALIZ_LOCAL_DIR"]
except KeyError:
    extra_kwds = {}
else:
    has_extra_dir = True
    extra_kwds = {
      "include_dirs": [ normaliz_dir + '/include'],
      "library_dirs": [ normaliz_dir + '/lib'],
      "runtime_library_dirs": [ normaliz_dir + '/lib'],
    }

from os import path
import io

setup(
    py_modules = [ "PyNormaliz" ],
    ext_modules = [ Extension( "PyNormaliz_cpp",
                              [ "NormalizModule.cpp" ],
                              extra_compile_args=['-std=c++14'],
                              libraries=[ 'normaliz' ],
                              **extra_kwds) ],
    
    package_data = {'': [ "COPYING", "GPLv2", "README.md" ] },
)
