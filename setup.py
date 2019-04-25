#!/usr/bin/env python

from distutils.core import setup, Extension
from distutils.cmd import Command

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

try:
    normaliz_enf = os.environ["WITHOUT_ENFNORMALIZ"]
except KeyError:
    normaliz_enf = True
else:
    normaliz_enf = False

if normaliz_enf:
    extra_kwds["libraries"] = [ 'arb', 'normaliz', 'gmp', 'flint', 'eanticxx' ]
else:
    extra_kwds["libraries"] = [ 'normaliz', 'gmp', 'flint' ]


class TestCommand(Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        import subprocess

        old_path = os.getcwd()
        setup_path = os.path.dirname(__file__)
        tests_path = os.path.join(setup_path, 'tests')
        try:
            os.chdir(tests_path)

            if subprocess.call([sys.executable, 'runtests.py']):
                raise SystemExit("Doctest failures")

        finally:
            os.chdir(old_path)

setup(
    name = 'PyNormaliz',
    version = '2.1',
    description = 'An interface to Normaliz',
    author = 'Sebastian Gutsche, Richard Sieg',
    author_email = 'sebastian.gutsche@gmail.com',
    url = 'https://github.com/Normaliz/PyNormaliz',
    py_modules = [ "PyNormaliz" ],
    ext_modules = [ Extension( "PyNormaliz_cpp",
                              [ "NormalizModule.cpp" ],
                              extra_compile_args=['-std=c++11'],
                              **extra_kwds) ],
    
    package_data = {'': [ "COPYING", "GPLv2", "Readme.md" ] },
    cmdclass = {'test': TestCommand},
)
