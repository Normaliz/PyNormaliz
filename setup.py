from distutils.core import setup, Extension
import sys

if sys.version_info < (3,5):
    macro_list = [ ( "PYTHON_VERSION_OLDER_THREE_FIVE", "1" ) ]
else:
    macro_list = [ ]

setup(
    name = 'PyNormaliz',
    version = '0.2',
    description = 'A simple interface to LibNormaliz',
    author = 'Sebastian Gutsche',
    author_email = 'sebastian.gutsche@gmail.com',
    url = 'https://github.com/sebasguts/PyNormaliz',
    ext_modules= [ Extension( "PyNormaliz",
                              [ "NormalizModule.cpp" ],
                              include_dirs=['/usr/local/include/LibNormaliz', '/usr/local/include/', '/usr/include' ],
                              library_dirs=['/usr/local/lib', '/usr/lib' ],
                              extra_link_args=['-lnormaliz', '-lgmp' ],
                              define_macros = macro_list ) ],
)
