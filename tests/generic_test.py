#/usr/bin/env python
# Test file for PyNormaliz module
#
# Just run
#
#    $ python generic_test.py
#
# or alternatively from a Python console
#
#    >>> import generic_test
#    >>> generic_test.Test1(verbose=True).run(repeat=50)

from __future__ import print_function

import unittest
from sys import stdout

from PyNormaliz import NmzCone, NmzResult, normaliz_error
from random import shuffle

methods = [
         "AffineDim",
         "Congruences",
         "Dehomogenization",
         "Equations",
         "EmbeddingDim",
# NOTE: floating point value
#         "EuclideanVolume",
         "EhrhartQuasiPolynomial",
         "ExcludedFaces",
         "ExternalIndex",
         "ExtremeRays",
         "GeneratorOfInterior",
         "Generators",
         "Grading",
         "GradingDenom",
         "HilbertQuasiPolynomial",
         "InclusionExclusionData",
         "IsIntegrallyClosed",
         "IsInhomogeneous",
         "IsPointed",
         "IsTriangulationNested",
         "IsTriangulationPartial",
         "MaximalSubspace",
         "ModuleRank",
         "ModuleGenerators",
         "ReesPrimaryMultiplicity",
         "SupportHyperplanes",
         "Triangulation",
         "TriangulationSize",
         "UnitGroupIndex",
         "VerticesOfPolyhedron",
         "Volume",
         "Rank",
         "RecessionRank",
# NOTE: depends on CoCoALib
#         "WeightedEhrhartQuasiPolynomial",
         "WitnessNotIntegrallyClosed",
         ]

postprocess = {
    "Generators": sorted,
    "VerticesOfPolyhedron": sorted,
}

class GenericPyNormalizTest:
    def __init__(self, verbose=True):
        self.verbose = verbose

    def run(self, repeat=1):

        for i in range(repeat):
            if self.verbose:
                print("*"*80)
                print("{} run {}".format(self, i))
                stdout.flush()
            polytope = NmzCone(**self.init_data)

            shuffle(methods)
            for meth in methods:
                try:
                    obj = getattr(self, meth)
                except AttributeError:
                    print("Skipping unspecified {}".format(meth))
                    stdout.flush()
                    continue

                if self.verbose:
                    print("Testing {}... ".format(meth), end='')
                    stdout.flush()

                # if we expect an exception we just check that calling
                # the method indeed raises exception
                if type(obj) is type and issubclass(obj, Exception):
                    try:
                        NmzResult(polytope, meth)
                    except obj:
                        if self.verbose:
                            print("pass")
                            stdout.flush()
                        continue
                    except Exception as obj2:
                        if self.verbose:
                            print("failed")
                            stdout.flush()
                        raise ValueError("got {} instead of {}".format(obj2, obj))

                # otherwise we just check equality of results
                # (possibly after some postprocessing)
                else:
                    obj2 = NmzResult(polytope, meth)
                    post = postprocess.get(meth)
                    if post is not None:
                        obj = post(obj)
                        obj2 = post(obj2)

                    if obj != obj2:
                        if self.verbose:
                            print("failed")
                            stdout.flush()
                        raise ValueError("got {} instead of {}".format(obj2, obj))
                    elif self.verbose:
                        print("pass")
                        stdout.flush()

class Test1(GenericPyNormalizTest):
    init_data = {"vertices": [(-3,-2,-1,1), (-1,1,-1,2), (1,1,-1,1), (1,1,1,1)]}

    # expected results
    AffineDim = 3
    Congruences = []
    Dehomogenization = [0,0,0,1]
    EhrhartQuasiPolynomial = [[6,7,6,5], 6]
    Equations = []
    EmbeddingDim = 4
    ExcludedFaces = normaliz_error
    ExternalIndex = normaliz_error
    ExtremeRays = []
    GeneratorOfInterior = normaliz_error
    Generators = [[-3,-2,-1,1], [1,1,-1,1], [1,1,1,1], [-1,1,-1,2]]
    Grading = normaliz_error
    GradingDenom = normaliz_error
    HilbertQuasiPolynomial = normaliz_error
    InclusionExclusionData = normaliz_error
    IsIntegrallyClosed = normaliz_error
    IsInhomogeneous = True
    IsPointed = True
    IsTriangulationPartial = Exception # what is the actual error we got here?
    IsTriangulationNested = Exception # idem
    MaximalSubspace = []
    ModuleRank = 4
    ModuleGenerators = [[-3,-2,-1,1], [1,1,-1,1], [1,1,0,1], [1,1,1,1]]
    Rank = 4
    RecessionRank = 0
    ReesPrimaryMultiplicity = normaliz_error
    SupportHyperplanes = [[-3,4,0,-1], [1,-3,0,2], [3,-4,5,6], [7,-6,-5,4]]
    Triangulation = [[[0,1,2,3], 10]]
    TriangulationSize = 1
    UnitGroupIndex = normaliz_error
    VerticesOfPolyhedron = [[-3,-2,-1,1], [-1,1,-1,2], [1,1,-1,1], [1,1,1,1]]
    Volume = [5,1]
    WitnessNotIntegrallyClosed = normaliz_error

if __name__ == '__main__':
    Test1().run(repeat=100)
