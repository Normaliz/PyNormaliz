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

from itertools import product
from random import shuffle

methods = [
         "AffineDim",
         "Automorphisms",
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
    "VerticesOfPolyhedron": sorted,
}

class GenericPyNormalizTest:
    def __init__(self, verbosity=1):
        self.verbosity = verbosity

    def expected_answers(self):
        ans = {}
        for meth in methods:
            try:
                obj = getattr(self, meth)
            except AttributeError:
                continue
            else:
                ans[meth] = obj
        return ans

    def run(self, repeat=1):
        tested = failed = 0
        if self.verbosity >= 1:
            print("Unittest {} ({} repetitions)".format(self, repeat))

        ans = self.expected_answers()
        def_methods = sorted(ans.keys())

        if self.verbosity >= 2:
            undef_methods = sorted(set(methods).difference(def_methods))
            print("Testing: {}".format(", ".join(meth for meth in def_methods)))
            print("Skipping: {}".format(", ".join(meth for meth in undef_methods)))

        for i in range(repeat):
            if self.verbosity >= 3:
                print("*"*80)
                print("{} run {}".format(self, i))
                stdout.flush()
            polytope = NmzCone(**self.init_data)

            shuffle(def_methods)
            for meth in def_methods:
                tested += 1

                obj = ans[meth] 
                if self.verbosity >= 3:
                    print("Testing {}... ".format(meth), end='')
                    stdout.flush()

                # if we expect an exception we just check that calling
                # the method indeed raises exception
                if type(obj) is type and issubclass(obj, Exception):
                    try:
                        NmzResult(polytope, meth)
                    except obj:
                        if self.verbosity >= 3:
                            print("pass")
                            stdout.flush()
                        continue
                    except Exception as obj2:
                        print("*"*50)
                        print("Unittest {}, failure {}".format(self, meth))
                        print("{}: got {} instead of {}".format(meth, obj2, obj))
                        print("*"*50)
                        stdout.flush()
                        failed += 1

                # otherwise we just check equality of results
                # (possibly after some postprocessing)
                else:
                    obj2 = NmzResult(polytope, meth)
                    post = postprocess.get(meth)
                    if post is not None:
                        obj = post(obj)
                        obj2 = post(obj2)

                    if obj != obj2:
                        print("*"*50)
                        print("Unittest {}, failure {}".format(self, meth))
                        print("{}: got {} instead of {}".format(meth, obj2, obj))
                        print("*"*50)
                        stdout.flush()
                        failed += 1
                    elif self.verbosity >= 3:
                        print("pass")
                        stdout.flush()

        return failed

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
    Triangulation = [[[[0, 1, 2, 3], 10, []]], [[-3, -2, -1, 1], [-1, 1, -1, 2], [1, 1, -1, 1], [1, 1, 1, 1]]]
    TriangulationSize = 1
    UnitGroupIndex = normaliz_error
    VerticesOfPolyhedron = [[-3,-2,-1,1], [-1,1,-1,2], [1,1,-1,1], [1,1,1,1]]
    Volume = [5,1]
    WitnessNotIntegrallyClosed = normaliz_error

class Test2(GenericPyNormalizTest):
    "A cube in a subspace"

    init_data = {"vertices": list((2,) + p+(-2,1,) for p in product([-1,0,1],repeat=2))}

    # expected results
    AffineDim = 2
    Automorphisms = [8, True, True, [[], []], [[[0, 2, 1, 3], [1, 0, 3, 2]], [[0, 1, 2, 3]]], [[[1, 0, 3, 2], [0, 2, 1, 3]], [[0, 1, 2, 3]]]]
    Congruences = []
    Dehomogenization = normaliz_error
    EmbeddingDim = 5
    EhrhartQuasiPolynomial = [[1,4,4],1]
    Equations = [[1,0,0,0,-2],[0,0,0,1,2]]
    ExcludedFaces = normaliz_error
    ExternalIndex = 1
    ExtremeRays = []
    GeneratorOfInterior = normaliz_error
    Grading = normaliz_error
    GradingDenom = normaliz_error
    HilbertQuasiPolynomial = normaliz_error
    InclusionExclusionData = normaliz_error
    IsInhomogeneous = True
    IsIntegrallyClosed = normaliz_error
    IsPointed = True
    IsTriangulationPartial = Exception # what is the actual error we got here?
    IsTriangulationNested = Exception # idem
    MaximalSubspace = []
    ModuleGenerators = [[2,-1,-1,-2,1], [2,-1,0,-2,1], [2,-1,1,-2,1], [2,0,-1,-2,1],
           [2,0,0,-2,1], [2,0,1,-2,1], [2,1,-1,-2,1], [2,1,0,-2,1], [2,1,1,-2,1]]
    ModuleRank = 9
    Rank = 3
    ReesPrimaryMultiplicity = normaliz_error
    RecessionRank = 0
    SupportHyperplanes = [[0,-1,0,0,1],[0,0,-1,0,1],[0,0,1,0,1],[0,1,0,0,1]]
    Triangulation = [[[[0, 1, 3], 1, []], [[1, 2, 3], 1, []], [[2, 3, 4], 1, []], [[2, 4, 5], 1, []], [[3, 4, 6], 1, []], [[4, 5, 6], 1, []], [[5, 6, 7], 1, []], [[5, 7, 8], 1, []]], [[2, -1, -1, -2, 1], [2, -1, 0, -2, 1], [2, -1, 1, -2, 1], [2, 0, -1, -2, 1], [2, 0, 0, -2, 1], [2, 0, 1, -2, 1], [2, 1, -1, -2, 1], [2, 1, 0, -2, 1], [2, 1, 1, -2, 1]]] 
    TriangulationSize = 8
    UnitGroupIndex = normaliz_error
    VerticesOfPolyhedron = [[2,-1,-1,-2,1], [2,-1,1,-2,1], [2,1,-1,-2,1], [2,1,1,-2,1]]
    Volume = [8,1]
    WitnessNotIntegrallyClosed = normaliz_error

class Test3(GenericPyNormalizTest):
    "A 2-dimensional linear subspace"

    init_data = {"vertices": [[1,2,3,5]], "cone": [[1,0,-1],[-1,0,1]]}

    # expected results
    AffineDim = 1
    Congruences = []
    Dehomogenization = normaliz_error
    EmbeddingDim = 4
    EhrhartQuasiPolynomial = [[1], [0], [0], [0], [0], 1]
    Equations = [[1,-2,1,0],[0,5,0,-2]]
#    Generators = ...
# apparently not well defined -- and therefore no longer in use
    Grading = normaliz_error
    HilbertQuasiPolynomial = normaliz_error
    IsInhomogeneous = True
    IsIntegrallyClosed = normaliz_error
    IsPointed = False
    VerticesOfPolyhedron = [[0,2,4,5]]
    ExtremeRays = []
    MaximalSubspace = [[1,0,-1,0]]
    ModuleGenerators = []
    ModuleRank = 0
#    SupportHyperplanes = [[0,0,0,1]] # [[0,-2,0,1]]
# not well defined
    RecessionRank = 1
    TriangulationSize = 1
    Volume = normaliz_error
    WitnessNotIntegrallyClosed = normaliz_error


class Test4(GenericPyNormalizTest):
    init_data = {"vertices": [[0,0,1],[1,0,1],[0,1,1]], "cone": [[1, 1]]}

    # expected results
    Equations = []
    ExtremeRays = [[1,1,0]]
    MaximalSubspace = []
    ModuleGenerators = [[0,0,1],[0,1,1],[1,0,1]]
    SupportHyperplanes = [[-1,1,1],[0,1,0],[1,-1,1],[1,0,0]]
    VerticesOfPolyhedron = [[0,0,1],[0,1,1],[1,0,1]]
    Volume = normaliz_error

tests = [Test1, Test2, Test3, Test4]

if __name__ == '__main__':
    for test in tests:
        test().run()
