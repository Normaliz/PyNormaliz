[![Build Status](https://github.com/Normaliz/PyNormaliz/workflows/Run%20tests/badge.svg)](https://github.com/Normaliz/PyNormaliz/actions)

# PyNormaliz - A python interface to Normaliz


PyNormaliz provides an interface to [Normaliz](https://www.normaliz.uni-osnabrueck.de) via libNormaliz.
It offers the complete functionality of Normaliz, and can be used interactively from python.
For a first example, see [this introduction](doc/PyNormaliz_Tutorial.pdf) by Richard Sieg (Slighty outdated: for the installation follow the instructions below).

A full documentation is conatined in [Appendix E](doc/PyNormaliz.pdf) of the Normaliz manial.


## Requirements

* python 2.7 or higher or python 3.4 or higher
* Normaliz 3.8.10 or higher <https://github.com/Normaliz/Normaliz/releases>

(The current version of PyNormaliz is under construction. Normaliz 3.8.10 not yet released. Use current master of Normaliz for current master of PyNormaliz.) The source packages of the Normaliz realeases contain PyNormaliz.

## Installation

The PyNormaliz install script assumes that you have executed

    ./install_normaliz_with_eantic.sh

within the Normaliz directory. To install PyNormaliz navigate to the Normaliz directory and type

    ./install_pynormaliz.sh --user

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

is the matrix of the two Hilbert basis vectors. The ouput matrices of PyNormaliz can be used also in Normaliz input files.

One can pass options to the compute functions as in

    C.HilbertSeries(HSOP = True)

Note that some Normaliz output types must be specially encoded for python. Our Hilbert Series is returned as

    [[1], [1, 1], 0]

to be read as follows: [1] is the numerator polynomial, [1,1] is the vector of exponents of t that occur in the denominator, which is (1-t)(1-t) in our case, and 0 is the shift.  So the Hilbert series is given by the rational function 1/(1-t)(1-t). (Aoso see ee [this introduction](doc/PyNormaliz_Tutorial.pdf).) But we can use

    print_series(C.HilbertSeries(HSOP = True))
    
with the result

        (1)
    ---------
    (1 - t)^2


One can also compute several data simultaneously and specify options ("PrimalMode" only added as an example, not because ot is particularly useful here):

    C.Compute("LatticePoints", "Volume", "PrimalMode")
    
Then

    C.Volume()
    
with the result

    1

This is the lattice length of the diagonal in the square. The euclidean length, that has been computed simultaneously, is returned by

    C.EuclideanVolume()
    
with the expected value

    '1.4142'
    
Floating point numbers are formatted with 4 decimal places and returned as strings (may change). If you want the euclideal volume at the maximum floating point precision, you can use the low level interface which is intermediate between the class Cone and libnormaliz:

    NmzResult(C.cone,"EuclideanVolume")
    1.4142135623730951
    
One can find out whether a single goal has been computed by asking

    C.IsComputed("Automorphisms")
    False
    
If you use Compute instead of IsComputed, then Normaliz tries to compute the goal, and there are situations in which the computation is undesirable.

Algebraic polyhedra can be computed by PyNormaliz as well:

    nf = [ "a2-2", "a", "1.4+/-0.1" ]
    D = Cone( number_field = nf, cone = [["1/7a+3/2", "-5a"],["4/83a-1","97/81"]])

It is important to note that fractions and algebraic numbers must be encoded as strings for the input.

    S = D.SupportHyperplanes()
    S
    [['-1470/433*a+280/433', '-1'], ['-32204/555417*a-668233/555417', '-1']]

Very hard to read! Somewhat better:

    print_matrix(S)
    
              -1470/433*a+280/433 -1
    -32204/555417*a-668233/555417 -1

But we can also get floating point approximations:

    print_matrix(D.SuppHypsFloat())

    -4.1545 -1.0000
    -1.2851 -1.0000

By using Python functions, the functionality of Normaliz can be extended. For example, 
    
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
