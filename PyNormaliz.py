import PyNormaliz_cpp
from PyNormaliz_cpp import *

class Cone:
    
    def __init__(self, *args, **kwargs):
        input_list = [ k for k in args ]
        for i in kwargs:
            current_input = kwargs[i]
            if type(current_input) == list and len(current_input) > 0 and type(current_input[0]) != list:
                kwargs[i] = [current_input]
            elif type(current_input) == bool and current_input == True:
                kwargs[i] = current_input=[[]]
            elif type(current_input) == bool and current_input == False:
                kwargs.pop( i )
        
        for i in kwargs:
            input_list.append(i)
            input_list.append(kwargs[i])
        
        self.cone = PyNormaliz_cpp.NmzCone( input_list )
    
    def __process_keyword_args(self,keywords):
        input_list = [ ]
        for i in keywords:
            if keywords[i] == True:
                input_list.append( i )
        return input_list

    def __str__(self):
        return "<Normaliz Cone>"
    
    def __repr__(self):
        return "<Normaliz Cone>"

    def Compute(self,*args):
        return PyNormaliz_cpp.NmzCompute(self.cone, args)

    def Generators(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Generators")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Generators")


    def ExtremeRays(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "ExtremeRays")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "ExtremeRays")


    def VerticesOfPolyhedron(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "VerticesOfPolyhedron")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "VerticesOfPolyhedron")


    def SupportHyperplanes(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "SupportHyperplanes")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "SupportHyperplanes")


    def HilbertBasis(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "HilbertBasis")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "HilbertBasis")


    def ModuleGenerators(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "ModuleGenerators")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "ModuleGenerators")


    def Deg1Elements(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Deg1Elements")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Deg1Elements")


    def ModuleGeneratorsOverOriginalMonoid(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "ModuleGeneratorsOverOriginalMonoid")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "ModuleGeneratorsOverOriginalMonoid")


    def Sublattice(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Sublattice")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Sublattice")


    def ExcludedFaces(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "ExcludedFaces")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "ExcludedFaces")


    def OriginalMonoidGenerators(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "OriginalMonoidGenerators")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "OriginalMonoidGenerators")


    def MaximalSubspace(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "MaximalSubspace")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "MaximalSubspace")


    def Equations(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Equations")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Equations")


    def Congruences(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Congruences")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Congruences")


    def Grading(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Grading")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Grading")


    def Dehomogenization(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Dehomogenization")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Dehomogenization")


    def WitnessNotIntegrallyClosed(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "WitnessNotIntegrallyClosed")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "WitnessNotIntegrallyClosed")


    def TriangulationSize(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "TriangulationSize")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "TriangulationSize")


    def TriangulationDetSum(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "TriangulationDetSum")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "TriangulationDetSum")


    def ReesPrimaryMultiplicity(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "ReesPrimaryMultiplicity")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "ReesPrimaryMultiplicity")


    def GradingDenom(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "GradingDenom")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "GradingDenom")


    def UnitGroupIndex(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "UnitGroupIndex")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "UnitGroupIndex")


    def InternalIndex(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "InternalIndex")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "InternalIndex")


    def ExternalIndex(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "ExternalIndex")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "ExternalIndex")


    def Multiplicity(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Multiplicity")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Multiplicity")


    def RecessionRank(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "RecessionRank")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "RecessionRank")


    def AffineDim(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "AffineDim")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "AffineDim")


    def ModuleRank(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "ModuleRank")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "ModuleRank")


    def Rank(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Rank")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Rank")


    def EmbeddingDim(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "EmbeddingDim")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "EmbeddingDim")


    def IsPointed(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "IsPointed")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "IsPointed")


    def IsDeg1ExtremeRays(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "IsDeg1ExtremeRays")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "IsDeg1ExtremeRays")


    def IsDeg1HilbertBasis(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "IsDeg1HilbertBasis")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "IsDeg1HilbertBasis")


    def IsIntegrallyClosed(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "IsIntegrallyClosed")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "IsIntegrallyClosed")


    def IsReesPrimary(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "IsReesPrimary")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "IsReesPrimary")


    def IsInhomogeneous(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "IsInhomogeneous")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "IsInhomogeneous")


    def Triangulation(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "Triangulation")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "Triangulation")


    def HilbertSeries(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "HilbertSeries")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "HilbertSeries")


    def InclusionExclusionData(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "InclusionExclusionData")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "InclusionExclusionData")


    def StanleyDec(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "StanleyDec")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "StanleyDec")


    def ClassGroup(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "ClassGroup")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "ClassGroup")


    def IntegerHull(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "IntegerHull")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "IntegerHull")


    def ConeDecomposition(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "ConeDecomposition")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "ConeDecomposition")


    def HilbertQuasiPolynomial(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "HilbertQuasiPolynomial")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "HilbertQuasiPolynomial")


    def IsTriangulationNested(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "IsTriangulationNested")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "IsTriangulationNested")


    def IsTriangulationPartial(self, **kwargs ):
        input_list = self.__process_keyword_args( kwargs )
        input_list.append( "IsTriangulationPartial")
        PyNormaliz_cpp.NmzCompute( self.cone, input_list );
        return PyNormaliz_cpp.NmzResult(self.cone, "IsTriangulationPartial")


