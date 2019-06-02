# PyNormaliz - An interface to Normaliz


## What is PyNormaliz

PyNormaliz provides an interface to Normaliz (https://www.normaliz.uni-osnabrueck.de) via libNormaliz. It offers the complete functionality of Normaliz, and can be used interactively from python. For a first example, see [this introduction](doc/PyNormaliz_Tutorial.pdf) by Richard Sieg.


## Requirements

* python 2.7 or higher or python 3.4 or higher
* Normaliz 3.2.1 or higher (https://github.com/Normaliz/Normaliz/releases)

The source packages of the Normaliz realeases contains PyNormaliz.

## Installation

The PyNormaliz install script assumes that you have executed the
```
install_normaliz_with_qnormaliz_eantic.sh
```
script. To install PyNormaliz and navigate to the Normaliz directory and type
```
python setup.py install --user
```

The script can be customized by some options. See Appendix E of the [Normaliz manual](https://github.com/Normaliz/Normaliz/blob/master/doc/Normaliz.pdf).

## Usage

The main command is Cone to create a cone, and the member functions
of the cone class to compute properties. For a full list of input and output
properties, see the Normaliz manual.

To create a cone, use
```
import PyNormaliz
C = PyNormaliz.Cone(cone = [[1,0],[0,1]])
```

All possible Normaliz input types can be given as keyword arguments.

To compute a property of the cone, use the provided getters, which correspond to Normaliz computation goals.

```
C.HilbertBasis()
```

You can pass options to the compute functions
```
C.HilbertSeries(HSOP = True)
```

## Low level commands

There is also a low-level API, directly using C functions:

To create a cone, use
```
C = NmzCone("cone", [[1,0],[0,1]])
```
or, equivalently,
```
C = NmzCone(["cone", [[1,0],[0,1]]])
```
NmzCone can take an arbitrary number of arguments, either as separated arguments or in a list. First is always a string, describing an input property for Normaliz, followed by a (possibly empty) matrix.

NmzCompute takes a cone as first argument, followed by arbitrary many strings, or a list of strings, describing Normaliz output types. NmzCompute lets Normaliz compute the necessary values, and returns true if everything was computed properly, false otherwise.
```
NmzCompute(C, "HilbertBasis")
```
or
```
NmzCompute(C, ["HilbertBasis"])
```

NmzIsComputed takes a cone and a string representing an output property, and returns true if the property is already computed for the cone, false otherwise. (In contrast to NmzCompute it does not start a computation.)
```
NmzIsComputed(C, "HilbertBasis")
```

NmzResult takes a cone and a string representing an output property, and returns the computed value of this property as a matrix, a list, or as a bool.
```
NmzResult(C, "HilbertBasis")
```
