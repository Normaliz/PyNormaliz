# encoding=utf8

import PyNormaliz_cpp
from PyNormaliz_cpp import *


class Cone:

    def __init__(self, *args, **kwargs):
        input_list = [k for k in args]
        for i in kwargs:
            current_input = kwargs[i]
            if type(current_input) == list and len(current_input) > 0 and type(current_input[0]) != list:
                kwargs[i] = [current_input]
            elif type(current_input) == bool and current_input == True:
                kwargs[i] = current_input = [[]]
            elif type(current_input) == bool and current_input == False:
                kwargs.pop(i)
        self.cone = PyNormaliz_cpp.NmzCone(input_list,**kwargs)

    def __process_keyword_args(self, keywords):
        input_list = []
        for i in keywords:
            if keywords[i] == True:
                input_list.append(i)
        return input_list

    def print_properties(self):
        props = PyNormaliz_cpp.NmzListConeProperties()
        goals = props[0]
        for x in goals:
            if (PyNormaliz_cpp.NmzIsComputed(self.cone, x)):
                print(x + ":")
                print(PyNormaliz_cpp.NmzResult(self.cone, x))
                print("\n")

    def __str__(self):
        return "<Normaliz Cone>"

    def __repr__(self):
        return "<Normaliz Cone>"

    def Compute(self, *args):
        return PyNormaliz_cpp.NmzCompute(self.cone, args)

    def setVerbose(self, verbose=True):
        return NmzSetVerbose(self.cone, verbose)

    # This one is not like the others!
    def IntegerHull(self, **kwargs):
        input_list = self.__process_keyword_args(kwargs)
        input_list.append("IntegerHull")
        PyNormaliz_cpp.NmzCompute(self.cone, input_list)
        new_inner_cone = PyNormaliz_cpp.NmzResult(self.cone, "IntegerHull")
        return_cone = Cone.__new__(Cone)
        return_cone.cone = new_inner_cone
        return return_cone

    def ProjectCone(self, **kwargs):
        input_list = self.__process_keyword_args(kwargs)
        input_list.append("ProjectCone")
        PyNormaliz_cpp.NmzCompute(self.cone, input_list)
        new_inner_cone = PyNormaliz_cpp.NmzResult(self.cone, "ProjectCone")
        return_cone = Cone.__new__(Cone)
        return_cone.cone = new_inner_cone
        return return_cone

    def EuclideanVolume(self, **kwargs):
        input_list = self.__process_keyword_args(kwargs)
        input_list.append("Volume")
        PyNormaliz_cpp.NmzCompute(self.cone, input_list)
        return PyNormaliz_cpp.NmzResult(self.cone, "EuclideanVolume")

    def HilbertSeries(self, **kwargs):
        try:
            as_hsop = kwargs["HSOP"]
        except KeyError:
            as_hsop = 28
        input_list = self.__process_keyword_args(kwargs)
        input_list.append("HilbertSeries")
        PyNormaliz_cpp.NmzCompute(self.cone, input_list)
        if as_hsop == 28:
            return PyNormaliz_cpp.NmzHilbertSeries(self.cone)
        if type(as_hsop) == bool:
            return PyNormaliz_cpp.NmzHilbertSeries(self.cone, as_hsop)
        raise TypeError("If HSOP is given, it must be True or False")

    def EhrhartSeries(self, **kwargs):
        try:
            as_hsop = kwargs["HSOP"]
        except KeyError:
            as_hsop = 28
        input_list = self.__process_keyword_args(kwargs)
        input_list.append("EhrhartSeries")
        PyNormaliz_cpp.NmzCompute(self.cone, input_list)
        if as_hsop == 28:
            return PyNormaliz_cpp.NmzHilbertSeries(self.cone)
        if type(as_hsop) == bool:
            return PyNormaliz_cpp.NmzHilbertSeries(self.cone, as_hsop)
        raise TypeError("If HSOP is given, it must be True or False")

    def Polynomial(self, **kwargs):
        return PyNormaliz_cpp.NmzGetPolynomial(self.cone)

    def NrCoeffQuasiPol(self, bound=-1):
        return PyNormaliz_cpp.NmzSetNrCoeffQuasiPol(self.cone, bound)

    def SymmetrizedCone(self, **kwargs):
        new_inner_cone = PyNormaliz_cpp.NmzSymmetrizedCone(self.cone)
        if new_inner_cone == None:
            return None
        return_cone = Cone.__new__(Cone)
        return_cone.cone = new_inner_cone
        return return_cone

    def HilbertSeriesExpansion(self,degree):
        return NmzGetHilbertSeriesExpansion(self.cone,degree)

    def WeightedEhrhartSeriesExpansion(self,degree):
        return NmzGetWeightedEhrhartSeriesExpansion(self.cone,degree)

    def PrettyPolynomialTuple(self, numCoefficients, denCoefficients):
        """
        Strings for numerator and denominator of the a hilbert series.

        Parameters
        ----------
        numCoefficients : list
            The coefficients for the numerator.
        denCofficients : list
            The coefficients for the denominator where the value represents the
            exponent of 't' and the frequency indicates the outer coefficient.

        Returns
        -------
        PrettyPolynomialTuple: tuple of strings

        Examples
        --------

        >>> numCoefficients = [3, 7, 4, -4, -6, 5]
        >>> denCoefficients = [1, 1, 2, 2, 2, 4]
        >>> PrettyPolynomialTuple(numCoefficients,denCoefficients)

        ('(3 + 7t + 4t² - 4t³ - 6t⁴ + 5t⁵)', '(1 - t)² (1 - t²)³ (1 - t⁴)')

        """
        def to_sup(s):
            sups = {u'0': u'\u2070',
                    u'1': u'\xb9',
                    u'2': u'\xb2',
                    u'3': u'\xb3',
                    u'4': u'\u2074',
                    u'5': u'\u2075',
                    u'6': u'\u2076',
                    u'7': u'\u2077',
                    u'8': u'\u2078',
                    u'9': u'\u2079'}
            if s is 1:
                return ''
            # lose the list comprehension
            return ''.join(sups.get(str(char), str(char)) for char in str(s))

        def getNumerator(coefficients):

            numerator = ''

            def isPositive(x):
                return x > 0

            firstNonZero = next(
                (i for i, x in enumerate(coefficients) if x != 0), 0)
            for exp, coefficient in enumerate(coefficients):
                if coefficient is 0:
                    continue
                # Exponent is 0 so keep only the coefficient
                if exp is 0:
                    numerator += '({}{!s}'.format('-' if not isPositive(coefficient)
                                                  else '', abs(coefficient))
                # Only include sign if `coefficient` is negative
                elif exp is firstNonZero:
                    numerator += '{}{!s}t{}'.format('-' if not isPositive(
                        coefficient) else '', abs(coefficient), to_sup(exp))
                else:
                    numerator += ' {}{!s}t{}'.format('+ ' if isPositive(
                        coefficient) else '- ', abs(coefficient), to_sup(exp))
            numerator += ')'
            return numerator

        def getDenominator(coefficients):
            exponents = [(inner, coefficients.count(inner))
                         for inner in set(coefficients)]
            denominator = ' '.join('(1 - t{}){}'. format(to_sup(x[0]) if x[0] is not 1 else '', to_sup(x[1]) if x[1] is not 1 else '') for x in exponents)
            return denominator

        num = getNumerator(numCoefficients)
        den = getDenominator(denCoefficients)
        prettyPolynomial = (num, den)
        return prettyPolynomial

    def PrintPrettyHilbertSeries(self, numCoefficients, denCoefficients):
        """
        Make a pretty hilbert series string

        Parameters
        ----------
        numCoefficients : list of ints
            The coefficients for the numerator.
        denCofficients : list of ints
            The coefficients for the denominator where the value represents
            the exponent of 't' and the frequency indicates the outer
            coefficient.

        Returns
        -------
        PrintPrettyHilbertSeries : string

        Examples
        --------

        >>> numCoefficients = [3, 7, 4, -4, -6, 5]
        >>> deCoefficients = [1, 1, 2, 2, 2, 4]
        >>> PrintPrettyHilbertSeries(numCoefficients,deCoefficients)

        (3 + 7t + 4t² - 4t³ - 6t⁴ + 5t⁵)
        --------------------------------
           (1 - t)² (1 - t²)³ (1 - t⁴)

        """
        num, den = self.PrettyPolynomialTuple(numCoefficients, denCoefficients)
        prettyPolynomial = '{:^}\n{:-^{width}}\n{:^{width}}'.format(
            num, '', den, width=max(len(den),len(num)))
        return prettyPolynomial

    def PrintHilbertSeries(self):
        hilbert_series=self.HilbertSeries()
        shift=hilbert_series[2]
        shift=[ 0 for x in range(1,shift) ]
        numerator=shift+hilbert_series[0]
        denominator=hilbert_series[1]
        print(self.PrintPrettyHilbertSeries(numerator,denominator))
        return None

    def _generic_getter(self, name, **kwargs):
        input_list = self.__process_keyword_args(kwargs)
        input_list.append(name)
        PyNormaliz_cpp.NmzCompute(self.cone, input_list)
        return PyNormaliz_cpp.NmzResult(self.cone, name)


# Generate getters for a bunch of Normalize properties
def add_dyn_getter(name):
    if hasattr(Cone, name):
        return
    def inner(self, **kwargs):
        return self._generic_getter(name, **kwargs)
    inner.__doc__ = "docstring for %s" % name
    inner.__name__ = name
    setattr(Cone, name, inner)

add_dyn_getter("AffineDim")
add_dyn_getter("AmbientAutomorphisms")
#add_dyn_getter("Approximate")
add_dyn_getter("Automorphisms")
#add_dyn_getter("BigInt")
#add_dyn_getter("BottomDecomposition")
add_dyn_getter("ClassGroup")
add_dyn_getter("CombinatorialAutomorphisms")
add_dyn_getter("ConeDecomposition")
add_dyn_getter("Congruences")
#add_dyn_getter("DefaultMode")
add_dyn_getter("Deg1Elements")
add_dyn_getter("Dehomogenization")
#add_dyn_getter("Descent")
#add_dyn_getter("DualMode")
#add_dyn_getter("Dynamic")
add_dyn_getter("EhrhartQuasiPolynomial")
add_dyn_getter("EhrhartSeries") # already defined above
add_dyn_getter("EmbeddingDim")
add_dyn_getter("Equations")
add_dyn_getter("EuclideanAutomorphisms")
add_dyn_getter("EuclideanIntegral")
add_dyn_getter("EuclideanVolume") # already defined above
add_dyn_getter("ExcludedFaces")
#add_dyn_getter("ExploitAutomsMult")
#add_dyn_getter("ExploitAutomsVectors")
add_dyn_getter("ExternalIndex")
add_dyn_getter("ExtremeRays")
add_dyn_getter("FVector")
add_dyn_getter("FaceLattice")
add_dyn_getter("GeneratorOfInterior")
add_dyn_getter("Generators")
add_dyn_getter("Grading")
add_dyn_getter("GradingDenom")
#add_dyn_getter("GradingIsPositive")
#add_dyn_getter("HSOP")
add_dyn_getter("HilbertBasis")
add_dyn_getter("HilbertQuasiPolynomial")
add_dyn_getter("HilbertSeries") # already defined above
add_dyn_getter("Incidence")
add_dyn_getter("InclusionExclusionData")
add_dyn_getter("IntegerHull") # already defined above
add_dyn_getter("Integral")
add_dyn_getter("InternalIndex")
add_dyn_getter("IsDeg1ExtremeRays")
add_dyn_getter("IsDeg1HilbertBasis")
add_dyn_getter("IsGorenstein")
add_dyn_getter("IsInhomogeneous")
add_dyn_getter("IsIntegrallyClosed")
add_dyn_getter("IsPointed")
add_dyn_getter("IsReesPrimary")
add_dyn_getter("IsTriangulationNested")
add_dyn_getter("IsTriangulationPartial")
#add_dyn_getter("KeepOrder")
add_dyn_getter("LatticePoints")
add_dyn_getter("MaximalSubspace")
add_dyn_getter("ModuleGenerators")
add_dyn_getter("ModuleGeneratorsOverOriginalMonoid")
add_dyn_getter("ModuleRank")
add_dyn_getter("Multiplicity")
#add_dyn_getter("NoBottomDec")
#add_dyn_getter("NoDescent")
#add_dyn_getter("NoGradingDenom")
#add_dyn_getter("NoLLL")
#add_dyn_getter("NoNestedTri")
#add_dyn_getter("NoPeriodBound")
#add_dyn_getter("NoProjection")
#add_dyn_getter("NoRelax")
#add_dyn_getter("NoSubdivision")
#add_dyn_getter("NoSymmetrization")
add_dyn_getter("NumberLatticePoints")
add_dyn_getter("OriginalMonoidGenerators")
#add_dyn_getter("PrimalMode")
add_dyn_getter("ProjectCone") # already defined above
#add_dyn_getter("Projection")
#add_dyn_getter("ProjectionFloat")
add_dyn_getter("Rank")
add_dyn_getter("RationalAutomorphisms")
add_dyn_getter("RecessionRank")
add_dyn_getter("ReesPrimaryMultiplicity")
add_dyn_getter("RenfVolume")
add_dyn_getter("StanleyDec")
#add_dyn_getter("Static")
add_dyn_getter("Sublattice")
add_dyn_getter("SuppHypsFloat")
add_dyn_getter("SupportHyperplanes")
#add_dyn_getter("Symmetrize")
add_dyn_getter("Triangulation")
add_dyn_getter("TriangulationDetSum")
add_dyn_getter("TriangulationSize")
add_dyn_getter("UnitGroupIndex")
add_dyn_getter("VerticesFloat")
add_dyn_getter("VerticesOfPolyhedron")
add_dyn_getter("VirtualMultiplicity")
add_dyn_getter("Volume")
add_dyn_getter("WeightedEhrhartQuasiPolynomial")
add_dyn_getter("WeightedEhrhartSeries")
add_dyn_getter("WitnessNotIntegrallyClosed")
