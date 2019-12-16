[![Build Status](https://travis-ci.org/Normaliz/PyNormaliz.svg)](https://travis-ci.org/Normaliz/PyNormaliz)
[![Code Coverage](https://codecov.io/github/Normaliz/PyNormaliz/coverage.svg)](https://codecov.io/gh/Normaliz/PyNormaliz)

# PyNormaliz - A python interface to Normaliz


PyNormaliz provides an interface to [Normaliz](https://www.normaliz.uni-osnabrueck.de) via libNormaliz.
It offers the complete functionality of Normaliz, and can be used interactively from python.
For a first example, see [this introduction](doc/PyNormaliz_Tutorial.pdf) by Richard Sieg (Slighty outdated: for the installation follow the instructions below).


## Requirements

* python 2.7 or higher or python 3.4 or higher
* Normaliz 3.8.3 or higher <https://github.com/Normaliz/Normaliz/releases>

The source packages of the Normaliz realeases contain PyNormaliz.

## Installation

The PyNormaliz install script assumes that you have executed

    ./install_normaliz_with_eantic.sh

within the Normaliz directory. To install PyNormaliz navigate to the Normaliz directory and type

    ./install_pynormaliz.sh --user

Also see Appendix E of the
[Normaliz manual](https://github.com/Normaliz/Normaliz/blob/master/doc/Normaliz.pdf).

## Usage

The command Cone creates a cone (and a lattice), and the member functions
of Cone compute its properties. For a full list of input and output
properties, see the Normaliz manual.

We assume that you are running python 3.

Start by

    import PyNormaliz
    from PyNormaliz import *

To create a simple example, type

    C = Cone(cone = [[1,0],[0,1]])


All possible Normaliz input types can be given as keyword arguments.

The member functions allow the computation of the data of our cone.  For example,

    C.HilbertBasis()

returns what its name says:

    [[0, 1], [1, 0]]

is the matrix of the two Hilbert basis vectors.

One can pass options to the compute functions as in

    C.HilbertSeries(HSOP = True)

Note that some Normaliz output types must be specially encoded for python. Our Hilbert Series is returned as

    [[1], [1, 1], 0]

to be read as follows: [1] is the numerator polynomial, [1,1] is the vector of exponents of t that occur in the denominator, which is (1-t)(1-t) in our case, and 0 is the shift.  So the Hilbert series is given by the rational function 1/(1-t)(1-t). (Aoso see ee [this introduction](doc/PyNormaliz_Tutorial.pdf).)

One can also compute several data simultaneously and specify options ("PrimalMode" only added as an example, not because ot is particularly useful here):

    C.Compute("LatticePoints", "Volume", "PrimalMode")
    
Then

    C.Volume()
    
retrieves the already computed result [1,1], which encodes the fraction 1/1 = 1. This is the lattice length of the diagonal in the square. The euclidean length, that has been computed simultaneously, is returned by

    C.EuclideanVolume()
    
with the expected value

    1.4142135623730951

By using Python functions, the functuionality of Normaliz can be extended. For example, 
    
    def intersection(cone1, cone2):
        intersection_ineq = cone1.SupportHyperplanes()+cone2.SupportHyperplanes()
        C = Cone(inequalities = intersection_ineq)
        return C
        
computes the intersection of two cones. So

    C1 = Cone(cone=[[1,2],[2,1]])
    C2 = Cone(cone=[[1,1],[1,3]])
    intersection(C1,C2).ExtremeRays()
    
yeilds the result

    [[1, 1], [1, 2]]
    
If you want to see what Normaliz is doing (especually in longer computations) activate the terminal output by

    C.setVerbose(True)
