# encoding=utf8

import sys

import PyNormaliz_cpp
from PyNormaliz_cpp import *

def fill_blanks(x,s):
    t = len(x)
    if t >= s:
        return x
    out = ""
    for i in range(s-t):
        out = " " + out
    out = out + x
    return out

def print_perms_and_orbits(data, name):
    print("permutations of ", name)
    for i in range(len(data[0])):
        print(i, ": ", data[0][i])
    print("orbits of ", name)
    for i in range(len(data[1])):
        print(i, ": ", data[1][i])
    return


def print_automs(Automs):
    print("order ", Automs[0])
    if Automs[1]:
        if Automs[2]:
            print("automorphisms are integral")
        else:
            print("automorphisms are not integral")
    else:
        print("integrality of automorphisms unknown")

    gen_name ="extreme rays of (recession) cone"
    if len(Automs) == 7:
        gen_name = "input vectors";
    lf_name = "support hyperplanes"
    if len(Automs) == 7:
        lf_name = "coordinates"

    if len(Automs[3][0]) >0:
        print_perms_and_orbits(Automs[3],gen_name)
    if len(Automs[4][0]) >0:
        print_perms_and_orbits(Automs[4],"vertices of polyhedron")
    if len(Automs[5][0]) >0:
        print_perms_and_orbits(Automs[5],lf_name)

    if len(Automs) == 7:
        print("input vectors")
        print_matrix(Automs[6])
    return

def print_Stanley_dec(dec):
    print("generators")
    print_matrix(dec[1])
    print("simlices and offsets")
    for i in range(len(dec[0])):
        print(dec[0][i][0])
        print()
        print_matrix(dec[0][i][1])
        print("-----------")
    return

def print_matrix(M):
    if  not isinstance(M,list):
        print("print_matrix applied to non-matrix")
        return
    if len(M) == 0:
        return
    if not isinstance(M[0],list):
        print("pretty_print applied to non-matrix")
        return
    L0 = len(M[0])
    CL = []
    for k in range(len(M[0])):
        CL = CL + [0]
    for i in range(len(M)):
        current = M[i]
        if  not isinstance(current,list) or len(current) != L0:
            print("pretty_print applied to non-matrix")
            return
        for j in range(len(current)):
            x = current[j]
            x = str(x)
            l = len(x)
            if l > CL[j]:
                CL[j] = l
    for i in range(len(M)):
        current = M[i]
        current_line =""
        for j in range(len(current)):
            s= 0
            if j > 0:
                s= 1
            x = current[j]
            x = str(x)
            x = fill_blanks(x,s+CL[j])
            current_line = current_line + x
        print(current_line)
    return


def our_rat_handler(list):
    if list[0] == 0:
        return 0
    if list[1] == 1:
        return list[0]
    return str(list[0])+"/"+str(list[1])


def our_renf_handler(list):
    out = ""
    non_zero = False
    for i in range(len(list)):
        j = len(list) - 1 -i
        current = str(list[j])
        if current[0] == '0':
            if non_zero or j !=0:
                continue
            else:
                out="0"
                return out
        non_zero = True
        sign ="+"
        if current[0] == '-' or out == "":
            sign = ""
        if j>0 and current == "-1":
                sign ="-"
        if j == 0:
            power = ""
        if j == 1:
            power = name_of_indeterminate
        if j > 1:
            power = name_of_indeterminate + "^"+str(j)
        coeff = current
        star = "*"
        if coeff == "1" or coeff == "-1" or  j==0:
            star = ""
        if (coeff == "1" or coeff == "-1") and  j>0:
            coeff = ""
        out = out + sign + coeff + star +power
    return out

def our_float_handler(x):
    return "{:.4f}".format(x)

def PrettyPolynomialTuple(numCoefficients, denCoefficients):
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
        if str(s) == '1':
            return ''
        if sys.version == 3:
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
            # lose the list comprehension
            return ''.join(sups.get(str(char), str(char)) for char in str(s))

        return "^"+str(s)

    def getNumerator(coefficients):

        numerator = ''

        def isPositive(x):
            return x > 0

        firstNonZero = next(
            (i for i, x in enumerate(coefficients) if x != 0), 0)
        for exp, coefficient in enumerate(coefficients):
            if coefficient == 0:
                continue
            coeff_str = str(abs(coefficient))
            if exp != 0:
                if coeff_str == "1":
                    coeff_str = " "
            # Exponent is 0 so keep only the coefficient
            if exp == 0:
                numerator += '({}{!s}'.format('-' if not isPositive(coefficient)
                                                else '',coeff_str)
            # Only include sign if `coefficient` is negative
            elif exp is firstNonZero:
                numerator += '{}{!s}t{}'.format('-' if not isPositive(
                    coefficient) else '', coeff_str, to_sup(exp))
            else:
                numerator += ' {}{!s}t{}'.format('+ ' if isPositive(
                    coefficient) else '- ',coeff_str, to_sup(exp))
        numerator += ')'
        return numerator

    def getDenominator(coefficients):
        exponents = [(inner, coefficients.count(inner))
                        for inner in set(coefficients)]
        denominator = ' '.join('(1 - t{}){}'. format(to_sup(x[0]) if x[0] != 1 else '', to_sup(x[1]) if x[1] != 1 else '') for x in exponents)
        return denominator

    num = getNumerator(numCoefficients)
    den = getDenominator(denCoefficients)
    prettyPolynomial = (num, den)
    return prettyPolynomial

def PrintPrettyHilbertSeries(numCoefficients, denCoefficients):
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
    num, den = PrettyPolynomialTuple(numCoefficients, denCoefficients)
    prettyPolynomial = '{:^}\n{:-^{width}}\n{:^{width}}'.format(
        num, '', den, width=max(len(den),len(num)))
    return prettyPolynomial

def print_series(series):
    shift=series[2]
    Shift = []
    if shift >= 0:
        Shift=[ 0 for x in range(1,shift) ]
    numerator=Shift+series[0]
    denominator=series[1]
    print(PrintPrettyHilbertSeries(numerator,denominator))
    if shift < 0:
        print("shift ", shift)
    if len(series) > 3 and series[3] !=1:
        print("dvide numerator by ",series[3])
    return

def print_quasipol(poly):
    pp = []
    for i in range(len(poly)-1):
        pp = pp + [poly[i]]
    print_matrix(pp)
    if poly[len(poly)-1] != 1:
           print ("divide all coefficients by ", poly[len(poly)-1])
    return


name_of_indeterminate = ""

class Cone:

    def __init__(self,**kwargs):
        global name_of_indeterminate
        pop_list = []
        for entry in kwargs.items():
            current_input=entry[1];
            key = entry[0]
            if type(current_input) == bool and current_input == True:
                kwargs[key] = current_input = [[]]
            elif type(current_input) == bool and current_input == False:
                poplist = pop_list + [key]
        for k in pop_list:
            kwargs.pop(k)
        self.cone = PyNormaliz_cpp.NmzCone(**kwargs)
        name_of_indeterminate = PyNormaliz_cpp.NmzFieldGenName(self.cone)

    def ModifyCone(self, *args):
        PyNormaliz_cpp.NmzModifyCone(self.cone, *args)

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
            if x == "Generators":
                continue
            if (PyNormaliz_cpp.NmzIsComputed(self.cone, x)):
                print(x + ":")
                print(PyNormaliz_cpp.NmzResult(self.cone, x))

    def __str__(self):
        return "<Normaliz Cone>"

    def __repr__(self):
        return "<Normaliz Cone>"

    def GetFieldGenName(self):
        return PyNormaliz_cpp.NmzFieldGenName(self.cone)

    def Compute(self, *args):
        return PyNormaliz_cpp.NmzCompute(self.cone, args)

    def IsComputed(self, *args):
        if len(args) != 1:
            raise ValueError("IsComputed must have exactly one argument")
        return PyNormaliz_cpp.NmzIsComputed(self.cone, args[0])

    def SetVerbose(self, verbose=True):
        return NmzSetVerbose(self.cone, verbose)

    # This one is not like the others!
    def IntegerHull(self):
        input_list=["IntegerHull"]
        PyNormaliz_cpp.NmzCompute(self.cone, input_list)
        new_inner_cone = PyNormaliz_cpp.NmzResult(self.cone, "IntegerHull")
        return_cone = Cone.__new__(Cone)
        return_cone.cone = new_inner_cone
        return return_cone

    def ProjectCone(self):
        input_list=["ProjectCone"]
        PyNormaliz_cpp.NmzCompute(self.cone, input_list)
        new_inner_cone = PyNormaliz_cpp.NmzResult(self.cone, "ProjectCone")
        return_cone = Cone.__new__(Cone)
        return_cone.cone = new_inner_cone
        return return_cone

    def SymmetrizedCone(self, **kwargs):
        new_inner_cone = PyNormaliz_cpp.NmzSymmetrizedCone(self.cone)
        if new_inner_cone == None:
            return None
        return_cone = Cone.__new__(Cone)
        return_cone.cone = new_inner_cone
        return return_cone

    def Polynomial(self):
        return PyNormaliz_cpp.NmzGetPolynomial(self.cone)

    def SetNrCoeffQuasiPol(self, bound=-1):
        return PyNormaliz_cpp.NmzSetNrCoeffQuasiPol(self.cone, bound)

    def SetFaceCodimBound(self, bound=-1):
        return PyNormaliz_cpp.NmzSetFaceCodimBound(self.cone, bound)

    def SetGBDegreeBound(self, bound=-1):
        return PyNormaliz_cpp.NmzSetGBDegreeBound(self.cone, bound)

    def SetGBMinDegree(self, bound=-1):
        return PyNormaliz_cpp.NmzSetGBMinDegree(self.cone, bound)

    def SetDecimalDigits(self, digits=100):
        return PyNormaliz_cpp.NmzSetDecimalDigits(self.cone, digits)

    def SetPolynomial(self, poly =""):
        return PyNormaliz_cpp.NmzSetPolynomial(self.cone, poly)

    def SetPolynomialEquations(self, polys =[]):
        return PyNormaliz_cpp.NmzSetPolynomialEquations(self.cone, polys)

    def SetPolynomialInequalities(self, polys =[]):
        return PyNormaliz_cpp.NmzSetPolynomialInequalities(self.cone, polys)

    def SetGrading(self, grading):
        return PyNormaliz_cpp.NmzSetGrading(self.cone, grading)

    def HilbertSeriesExpansion(self,degree):
        return NmzGetHilbertSeriesExpansion(self.cone,degree)

    def EhrhartSeriesExpansion(self,degree):
        return NmzGetEhrhartSeriesExpansion(self.cone,degree)

    def WeightedEhrhartSeriesExpansion(self,degree):
        return NmzGetWeightedEhrhartSeriesExpansion(self.cone,degree)

    def WriteOutputFile(self, project):
        return NmzWriteOutputFile(self.cone, project)

    def WritePrecompData(self, project):
        return NmzWritePrecompData(self.cone, project)

    def NumberFieldData(self):
        return NmzGetRenfInfo(self.cone)

    def _generic_getter(self, name, **kwargs):
        input_list = self.__process_keyword_args(kwargs)
        input_list.append(name)
        PyNormaliz_cpp.NmzCompute(self.cone, input_list)
        return PyNormaliz_cpp.NmzResult(self.cone, name,RationalHandler = our_rat_handler, \
               NumberfieldElementHandler=our_renf_handler, FloatHandler = our_float_handler)

# Generate getters for a bunch of Normalize properties
def add_dyn_getter(name):
    if hasattr(Cone, name):
        return
    def inner(self, **kwargs):
        return self._generic_getter(name, **kwargs)
    inner.__doc__ = "docstring for %s" % name
    inner.__name__ = name
    setattr(Cone, name, inner)

for name in PyNormaliz_cpp.NmzListConeProperties()[0]:
    add_dyn_getter(name)
