#!/usr/bin/env python

from distutils.core import setup, Extension
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
this_directory = path.abspath(path.dirname(__file__))
with io.open(path.join(this_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name = 'PyNormaliz',
    version = '2.9',
    description = 'An interface to Normaliz',
    long_description=long_description,
    long_description_content_type='text/markdown',
    author = 'Sebastian Gutsche, Richard Sieg',
    author_email = 'sebastian.gutsche@gmail.com',
    url = 'https://github.com/Normaliz/PyNormaliz',
    py_modules = [ "PyNormaliz" ],
    ext_modules = [ Extension( "PyNormaliz_cpp",
                              [ "NormalizModule.cpp" ],
                              extra_compile_args=['-std=c++11'],
                              libraries=[ 'normaliz' ],
                              **extra_kwds) ],
    
    package_data = {'': [ "COPYING", "GPLv2", "README.md" ] },
)
