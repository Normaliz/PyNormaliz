# PyNormaliz - Python interface to LibNormaliz

## Install
Make sure Normaliz 3.1.4 is properly installed (using make install).

If you installed them in a prefix hierarchy other than /usr/local,
set environment variables as follows:

  export LDFLAGS=-L/SOME/PREFIX/lib
  export CPATH=/SOME/PREFIX/include

Then install it using

    python setup.py install

or

    pip install .

See `examples/first.py` on how to use it.
