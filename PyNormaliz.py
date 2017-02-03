import PyNormaliz_cpp
from PyNormaliz_cpp import *

class Cone:
    
    def __init__(self, *args, **kwargs):
        input_list = [ k for k in args ]
        for i in kwargs:
            input_list.append(i)
            current_input = kwargs[i]
            if type(current_input) == list and len(current_input) > 0 and type(current_input[0]) != list:
                current_input=[current_input]
            input_list.append(current_input)
        self.cone = PyNormaliz_cpp.NmzCone( input_list )


    def Compute(self,*args):
        return PyNormaliz_cpp.NmzCompute(self.cone, args)

    def HilbertBasis(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "HilbertBasis")


    def Generators(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Generators")


    def ExtremeRays(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "ExtremeRays")


    def VerticesOfPolyhedron(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "VerticesOfPolyhedron")


    def SupportHyperplanes(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "SupportHyperplanes")


    def HilbertBasis(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "HilbertBasis")


    def ModuleGenerators(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "ModuleGenerators")


    def Deg1Elements(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Deg1Elements")


    def ModuleGeneratorsOverOriginalMonoid(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "ModuleGeneratorsOverOriginalMonoid")


    def Sublattice(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Sublattice")


    def ExcludedFaces(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "ExcludedFaces")


    def OriginalMonoidGenerators(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "OriginalMonoidGenerators")


    def MaximalSubspace(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "MaximalSubspace")


    def Equations(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Equations")


    def Congruences(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Congruences")


    def Grading(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Grading")


    def Dehomogenization(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Dehomogenization")


    def WitnessNotIntegrallyClosed(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "WitnessNotIntegrallyClosed")


    def TriangulationSize(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "TriangulationSize")


    def TriangulationDetSum(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "TriangulationDetSum")


    def ReesPrimaryMultiplicity(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "ReesPrimaryMultiplicity")


    def GradingDenom(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "GradingDenom")


    def UnitGroupIndex(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "UnitGroupIndex")


    def InternalIndex(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "InternalIndex")


    def ExternalIndex(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "ExternalIndex")


    def Multiplicity(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Multiplicity")


    def RecessionRank(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "RecessionRank")


    def AffineDim(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "AffineDim")


    def ModuleRank(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "ModuleRank")


    def Rank(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Rank")


    def EmbeddingDim(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "EmbeddingDim")


    def IsPointed(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "IsPointed")


    def IsDeg1ExtremeRays(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "IsDeg1ExtremeRays")


    def IsDeg1HilbertBasis(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "IsDeg1HilbertBasis")


    def IsIntegrallyClosed(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "IsIntegrallyClosed")


    def IsReesPrimary(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "IsReesPrimary")


    def IsInhomogeneous(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "IsInhomogeneous")


    def Triangulation(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "Triangulation")


    def HilbertSeries(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "HilbertSeries")


    def InclusionExclusionData(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "InclusionExclusionData")


    def StanleyDec(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "StanleyDec")


    def ClassGroup(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "ClassGroup")


    def IntegerHull(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "IntegerHull")


    def ConeDecomposition(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "ConeDecomposition")


    def HilbertQuasiPolynomial(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "HilbertQuasiPolynomial")


    def IsTriangulationNested(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "IsTriangulationNested")


    def IsTriangulationPartial(self):
        return PyNormaliz_cpp.NmzResult(self.cone, "IsTriangulationPartial")




