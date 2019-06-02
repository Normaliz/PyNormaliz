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
      "extra_link_args": ['-Wl,-R' + normaliz_dir + '/lib' ]
    }

libraries = [ 'normaliz', 'gmp', 'flint' ]

class TestCommand(Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        old_path = os.getcwd()
        setup_path = os.path.dirname(__file__)
        tests_path = os.path.join(setup_path, 'tests')
        try:
            os.chdir(tests_path)

            if subprocess.call([sys.executable, 'runtests.py']):
                raise SystemExit("Doctest failures")

        finally:
            os.chdir(old_path)

class build_ext(_build_ext):
    def run(self):
        """
        Run ./configure first and update libraries depending
        on the generated output.
        """
        subprocess.check_call(["make", "configure"])
        global has_extra_dir
        my_env = copy(os.environ)
        if has_extra_dir:
            my_env.update({ 'CPPFLAGS' : '-I' + normaliz_dir + '/include',
                            'LDFLAGS' : '-L' + normaliz_dir + '/lib' })
        try:
            subprocess.check_call(["sh", "./configure"], env = my_env )
        except subprocess.CalledProcessError:
            subprocess.check_call(["cat", "config.log"])
            raise
        # configure created config.py that we now import
        from config import ENFNORMALIZ
        global libraries
        if ENFNORMALIZ:
            print("building with ENFNORMALIZ...")
            libraries.append("arb")
            libraries.append("eanticxx")
        else:
            print("no ENFNORMALIZ support...")

        _build_ext.run(self)

from os import path
import io
this_directory = path.abspath(path.dirname(__file__))
with io.open(path.join(this_directory, 'Readme.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name = 'PyNormaliz',
    version = '2.7',
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
                              libraries=libraries,
                              **extra_kwds) ],
    
    package_data = {'': [ "COPYING", "GPLv2", "Readme.md" ] },
    cmdclass = {'build_ext': build_ext, 'test': TestCommand},
)
