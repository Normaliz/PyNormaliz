#!/usr/bin/env python

from distutils.core import setup, Extension
import sys
import os

try:
    normaliz_dir = os.environ["NORMALIZ_LOCAL_DIR"]
except KeyError:
    extra_kwds = {}
else:
    extra_kwds = {
      "include_dirs": [ normaliz_dir + '/include'],
      "library_dirs": [ normaliz_dir + '/lib'],
      "runtime_library_dirs": [ normaliz_dir + '/lib'],
      "extra_link_args": ['-Wl,-R' + normaliz_dir + '/lib' ]
    }


if sys.version_info < (3,5):
    macro_list = [ ( "PYTHON_VERSION_OLDER_THREE_FIVE", "1" ), ( "ENFNORMALIZ", "1" ) ]
else:
    macro_list = [ ( "ENFNORMALIZ", "1" ) ]

setup(
    name = 'PyNormaliz',
    version = '2.0',
    description = 'An interface to Normaliz',
    author = 'Sebastian Gutsche, Richard Sieg',
    author_email = 'sebastian.gutsche@gmail.com',
    url = 'https://github.com/Normaliz/PyNormaliz',
    py_modules = [ "PyNormaliz" ],
    ext_modules = [ Extension( "PyNormaliz_cpp",
                              [ "NormalizModule.cpp" ],
                              libraries=[ 'arb', 'normaliz', 'gmp', 'flint', 'eanticxx' ],
                              extra_compile_args=['-std=c++11'],
                              define_macros = macro_list,
                              **extra_kwds) ],
    
    package_data = {'': [ "COPYING", "GPLv2", "Readme.md" ] },
)
