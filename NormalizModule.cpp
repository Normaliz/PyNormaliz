/***************************************************************************
 *
 * Include
 *
 ***************************************************************************/

#include <Python.h>

#include <string>
using std::string;

/*
#include <libnormaliz/cone.h>
#include <libnormaliz/map_operations.h>
#include <libnormaliz/vector_operations.h>
#include <libnormaliz/automorph.h>
*/

#include <libnormaliz/libnormaliz.h>

using libnormaliz::Cone;
// using libnormaliz::ConeProperty;
using libnormaliz::ConeProperties;
using libnormaliz::Sublattice_Representation;
using libnormaliz::Type::InputType;
using libnormaliz::AutomorphismGroup;

#ifdef LIBNORMALIZ_DYNAMIC_BITSET_H
using libnormaliz::dynamic_bitset;
#else
typedef boost::dynamic_bitset<> dynamic_bitset;
#endif

#include <vector>
using std::map;
using std::pair;
using std::vector;

#include <csignal>

typedef int py_size_t;

/***************************************************************************
 *
 * Macros for exception handling
 *
 ***************************************************************************/

#define FUNC_BEGIN try {

#define FUNC_END                                                             \
    }                                                                        \
    catch (libnormaliz::InterruptException & e)                              \
    {                                                                        \
        libnormaliz::nmz_interrupted = false;                                \
        PyErr_SetString(PyExc_KeyboardInterrupt,                             \
                        "interrupted Normaliz Computation");                 \
        PyErr_SetInterrupt();                                                \
        PyErr_CheckSignals();                                                \
        return NULL;                                                         \
    }                                                                        \
    catch (libnormaliz::NormalizException & e)                               \
    {                                                                        \
        PyErr_SetString(NormalizError, e.what());                            \
        return NULL;                                                         \
    }                                                                        \
    catch (std::exception & e)                                               \
    {                                                                        \
        PyErr_SetString(PyNormaliz_cppError, e.what());                      \
        return NULL;                                                         \
    }


class PyNormalizInputException : public std::exception {
  private:
    std::string message_;

  public:
    explicit PyNormalizInputException(const std::string& message);
    virtual const char* what() const throw()
    {
        return message_.c_str();
    }
    std::string what_message() const throw()
    {
        return message_;
    }
};


PyNormalizInputException::PyNormalizInputException(const std::string& message)
    : message_(message)
{
}

/***************************************************************************
 *
 * Signal handling
 *
 ***************************************************************************/

static void signal_handler(int signal)
{
    libnormaliz::nmz_interrupted = true;
}

// helper class implementing RAII pattern for our custom SIGINT handler;
// it helps ensure we *always* restore the signal handler inside a
// FUNC_BEGIN / FUNC_END block.
class TempSignalHandler {
    PyOS_sighandler_t original_handler;
public:
    TempSignalHandler() {
        original_handler = PyOS_setsig(SIGINT, signal_handler);
    }

    ~TempSignalHandler() {
        PyOS_setsig(SIGINT, original_handler);
    }
};

/***************************************************************************
 *
 * Static objects
 *
 ***************************************************************************/


static PyObject*   NormalizError;
static PyObject*   PyNormaliz_cppError;
static const char* cone_name = "Cone";
static const char* cone_name_long = "Cone<long long>";
static const char* cone_name_renf = "Cone<renf_elem>";

static PyObject* RationalHandler = NULL;
static PyObject* FloatHandler = NULL;

#ifdef ENFNORMALIZ
static PyObject* NumberfieldElementHandler = NULL;
#endif

static PyObject* VectorHandler = NULL;
static PyObject* MatrixHandler = NULL;

/***************************************************************************
 *
 * Call func on one argument
 *
 ***************************************************************************/

static PyObject* CallPythonFuncOnOneArg(PyObject* function, PyObject* single_arg)
{
    PyObject* single_arg_tuple = PyTuple_Pack(1, single_arg);
    PyObject* return_obj = PyObject_CallObject(function, single_arg_tuple);
    Py_DecRef(single_arg);
    Py_DecRef(single_arg_tuple);
    return return_obj;
}

/***************************************************************************
 *
 * Compiler version control
 *
 ***************************************************************************/

#if PY_MAJOR_VERSION >= 3
#define string_check PyUnicode_Check
#else
#define string_check PyString_Check
#endif

#ifndef NMZ_RELEASE
static_assert(
    false,
    "Your Normaliz version (unknown) is to old! Update to 3.8.10 or newer.");
#endif
#if NMZ_RELEASE < 30810
static_assert(false,
              "Your Normaliz version is to old! Update to 3.8.10 or newer.");
#endif

/***************************************************************************
 *
 * Python-C data conversion functions
 *
 ***************************************************************************/

static string PyUnicodeToString(PyObject* in)
{
    if (!string_check(in)) {
        throw PyNormalizInputException("input must be a string");
        return NULL;
    }
#if PY_MAJOR_VERSION >= 3
    string out = "";
    int    length = PyUnicode_GET_SIZE(in);
    for (int i = 0; i < length; i++) {
        out += PyUnicode_READ_CHAR(in, i);
    }
    return out;
#else
    char* out = PyString_AsString(in);
    return string(out);
#endif
}

static PyObject* StringToPyUnicode(const string &in)
{
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString(in.c_str());
#else
    return PyString_FromString(in.c_str());
#endif
}

// Boolean conversion

static inline PyObject* BoolToPyBool(bool in)
{
    if (in)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

// Converting MPZ's to PyLong and back via strings. Worst possible solution
// ever.
static bool PyNumberToNmz(PyObject*, mpz_class&);

static bool PyNumberToNmz(PyObject* in, mpq_class& out)
{
    if (PyFloat_Check(in)) {
        throw PyNormalizInputException("PyFloat not allowed in PyNormaliz input. Must be ecoded as string.");
        return true;
    }
#if PY_MAJOR_VERSION < 3
    if (PyInt_Check(in)) {
        out = PyInt_AsLong(in);
        return true;
    }
#endif
    if (PyLong_Check(in)) {
        mpz_class out_tmp;
        bool      check = PyNumberToNmz(in, out_tmp);
        if (!check) {
            return false;
        }
        out = mpq_class(out_tmp);
        return true;
    }
    if (PyList_CheckExact(in) || PyTuple_CheckExact(in)) {
        PyObject* py_num = PySequence_GetItem(in, 0);
        PyObject* py_denom = PySequence_GetItem(in, 1);
        mpz_class num;
        if (!PyNumberToNmz(py_num, num)) {
            return false;
        }
        mpz_class denom;
        if (!PyNumberToNmz(py_denom, denom)) {
            return false;
        }
        out = mpq_class(num, denom);
        return true;
    }
    PyObject*   in_as_string = PyObject_Str(in);
    string      s = PyUnicodeToString(in_as_string);
    // int         check = out.set_str(s.c_str(), 10);
    libnormaliz::string2coeff(out,s);

    return true;
}

static bool PyNumberToNmz(PyObject* in, mpz_class& out)
{
#if PY_MAJOR_VERSION < 3
    if (PyInt_Check(in)) {
        out = PyInt_AsLong(in);
        return true;
    }
#endif
    if (!PyLong_Check(in)) {
        throw PyNormalizInputException(
            "input coeff must be a PyInt or PyLong");
    }
    int  overflow;
    long input_long = PyLong_AsLongAndOverflow(in, &overflow);
    if (overflow == 0) {
        out = mpz_class(input_long);
        return true;
    }
    PyObject*   in_as_string = PyObject_Str(in);
    string      s = PyUnicodeToString(in_as_string);
    out.set_str(s.c_str(), 10);
    return true;
}

static PyObject* NmzToPyNumber(const mpz_class in)
{
    if (in.fits_slong_p()) {
        return PyLong_FromLong(in.get_si());
    }

    // in Python 2, the first argument to PyLong_FromString is not const, thus
    // we need to perform a const cast here.
    string    mpz_as_string = in.get_str(16);
    char*     mpz_as_c_string = const_cast< char* >(mpz_as_string.c_str());
    return PyLong_FromString(mpz_as_c_string, NULL, 16);
}

static PyObject* NmzToPyNumber(const mpq_class in)
{
    PyObject* out_list = PyList_New(2);
    PyList_SetItem(out_list, 0, NmzToPyNumber(in.get_num()));
    PyList_SetItem(out_list, 1, NmzToPyNumber(in.get_den()));
    if (RationalHandler != NULL)
        out_list = CallPythonFuncOnOneArg(RationalHandler, out_list);
    return out_list;
}

static bool PyNumberToNmz(PyObject* in, long long& out)
{
    int overflow;
    out = PyLong_AsLongLongAndOverflow(in, &overflow);
    if (overflow == -1)
        throw PyNormalizInputException(
            "Cannot store input coefficient in long long");
    return true;
}

static PyObject* NmzToPyNumber(unsigned int in)
{
    return PyLong_FromUnsignedLong(in);
}

static PyObject* NmzToPyNumber(unsigned long in)
{
    return PyLong_FromUnsignedLong(in);
}

static PyObject* NmzToPyNumber(int in)
{
    return PyLong_FromLong(in);
}

static PyObject* NmzToPyNumber(long in)
{
    return PyLong_FromLong(in);
}

static PyObject* NmzToPyNumber(long long in)
{
    return PyLong_FromLongLong(in);
}

static PyObject* NmzToPyNumber(double in)
{
    PyObject* x = PyFloat_FromDouble(in);
    if(FloatHandler == NULL)
        return x;
 
    return CallPythonFuncOnOneArg(FloatHandler, x);
}

template < typename Integer >
static PyObject* NmzVectorToPyList(const vector< Integer >& in,
                            bool                     do_callback = true);

#ifdef ENFNORMALIZ
static PyObject* NmzToPyNumber(const renf_elem_class &in)
{
    // std::cout << "IIIII " << in << std::endl;
    vector< mpz_class > output_nums = in.get_num_vector();
    mpz_class           output_den = in.get_den();
    vector< mpz_class > denoms(output_nums.size(), output_den);
    for(size_t i=0; i< output_nums.size(); ++i){
        mpq_class quot = output_nums[i];
        quot /= output_den;
        output_nums[i] = quot.get_num();
        denoms[i] = quot.get_den();
    }
    /*std::cout << "NNN ";
    for( size_t i = 0; i< output_nums.size(); ++i)
        std::cout << output_nums[i] << " ";
    std::cout << std::endl;
    std::cout << "DDD ";
    for( size_t i = 0; i< output_nums.size(); ++i)
        std::cout << denoms[i] << " ";
    std::cout << std::endl;*/
    // PyObject*           denom_py = NmzToPyNumber(output_den);
    PyObject*           out_list = PyList_New(output_nums.size());
    for (size_t i = 0; i < output_nums.size(); i++) {
        PyObject* current = PyList_New(2);
        PyList_SetItem(current, 0, NmzToPyNumber(output_nums[i]));
        // Py_IncRef(denom_py);
        PyList_SetItem(current, 1, NmzToPyNumber(denoms[i]));
        if (RationalHandler != NULL)
            current = CallPythonFuncOnOneArg(RationalHandler, current);
        PyList_SetItem(out_list, i, current);
    }
    // Py_DecRef(denom_py);
    if (NumberfieldElementHandler != NULL)
        out_list = CallPythonFuncOnOneArg(NumberfieldElementHandler, out_list);
    return out_list;
}
#endif

PyObject* NmzToPyNumber(const dynamic_bitset& in)
{
    size_t    len = in.size();
    PyObject* result = PyList_New(len);
    for (size_t i = 0; i < len; i++) {
        PyList_SetItem(result, i, NmzToPyNumber(in[i] ? 1 : 0));
    }
    return result;
}

template < typename Integer >
static bool PyListToNmz(vector< Integer >& out, PyObject* in)
{
    if (!PySequence_Check(in))
        throw PyNormalizInputException("Input list is not a sequence");
    const int n = PySequence_Size(in);
    out.resize(n);
    for (int i = 0; i < n; ++i) {
        PyObject* tmp = PySequence_GetItem(in, i);
        if (!PyNumberToNmz(tmp, out[i]))
            return false;
    }
    return true;
}

template < typename Integer >
static bool PyIntMatrixToNmz(vector< vector< Integer > >& out, PyObject* in)
{
    if (!PySequence_Check(in))
        throw PyNormalizInputException("Input matrix is not a sequence");
    const int nr = PySequence_Size(in);
    out.resize(nr);
    for (int i = 0; i < nr; ++i) {
        bool okay = PyListToNmz(out[i], PySequence_GetItem(in, i));
        if (!okay)
            return false;
    }
    return true;
}

#ifdef ENFNORMALIZ
template < typename NumberField, typename NumberFieldElem >
static bool prepare_nf_input(vector< vector< NumberFieldElem > >& out,
                      PyObject*                            in,
                      NumberField*                         nf)
{
    if (!PySequence_Check(in))
        throw PyNormalizInputException("Number field data is not a list");
    const int nr = PySequence_Size(in);
    out.resize(nr);
    for (int i = 0; i < nr; ++i) {
        PyObject* current_row = PySequence_GetItem(in, i);
        int       current_length = PySequence_Size(current_row);
        out[i].resize(current_length);
        for (int j = 0; j < current_length; j++) {
            PyObject* current_element = PySequence_GetItem(current_row, j);
            bool      current_res;
            NumberFieldElem current_elem;
    
            if (PyList_CheckExact(current_element) || PyTuple_CheckExact(current_element)) {
                vector< mpq_class > current_vector;
                current_res = PyListToNmz(current_vector, current_element);
                if (!current_res) {
                    return false;
                }
                current_elem = NumberFieldElem(*nf, current_vector);
            }
            if (string_check(current_element)) {
                current_elem = NumberFieldElem(*nf);
                current_elem = PyUnicodeToString(current_element);
            }
            if (PyFloat_Check(current_element)){
                throw PyNormalizInputException("Nonintegral numbers must be given as strings"); 
            }
            if (PyLong_Check(current_element)) {
                mpq_class tmp;
                current_res = PyNumberToNmz(current_element, tmp);
                if (!current_res) {
                    return false;
                }
                current_elem = tmp;
            }
#if PY_MAJOR_VERSION < 3
            if (PyInt_Check(current_element)) {
                current_elem = PyInt_AsLong(current_element);
            }
#endif
            out[i][j] = current_elem;
        }
    }

    return true;
}
#endif

template < typename Integer >
static bool PyInputToNmz(vector< vector< Integer > >& out, PyObject* in)
{
    if (PyIntMatrixToNmz(out, in))
        return true;
    out.resize(1);
    if (PyListToNmz(out[0], in)) {
        return true;
    }
    throw PyNormalizInputException(
        "Input could not be converted to vector or list");
}

template < typename Integer >
static PyObject* NmzVectorToPyList(const vector< Integer >& in, bool do_callback)
{
    PyObject*    vector;
    const size_t n = in.size();
    vector = PyList_New(n);
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(vector, i, NmzToPyNumber(in[i]));
    }
    if (do_callback && VectorHandler != NULL)
        vector = CallPythonFuncOnOneArg(VectorHandler, vector);
    return vector;
}

template < typename Integer >
static PyObject* NmzMatrixToPyList(const vector< vector< Integer > >& in)
{
    PyObject*    matrix;
    const size_t n = in.size();
    matrix = PyList_New(n);
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(matrix, i, NmzVectorToPyList(in[i]));
    }
    if (MatrixHandler != NULL)
        matrix = CallPythonFuncOnOneArg(MatrixHandler, matrix);
    return matrix;
}

static PyObject* NmzHilbertSeriesToPyList(const libnormaliz::HilbertSeries& HS,
                                   bool                              is_HSOP)
{
    PyObject* return_list = PyList_New(3);
    if (is_HSOP) {
        PyList_SetItem(return_list, 0, NmzVectorToPyList(HS.getHSOPNum()));
        PyList_SetItem(
            return_list, 1,
            NmzVectorToPyList(libnormaliz::to_vector(HS.getHSOPDenom())));
        PyList_SetItem(return_list, 2, NmzToPyNumber(HS.getShift()));
    }
    else {
        PyList_SetItem(return_list, 0, NmzVectorToPyList(HS.getNum()));
        PyList_SetItem(
            return_list, 1,
            NmzVectorToPyList(libnormaliz::to_vector(HS.getDenom())));
        PyList_SetItem(return_list, 2, NmzToPyNumber(HS.getShift()));
    }
    return return_list;
}

template < typename Integer >
static PyObject* NmzWeightedEhrhartSeriesToPyList(
    const std::pair< libnormaliz::HilbertSeries, Integer >& HS)
{
    PyObject* return_list = PyList_New(4);
    PyList_SetItem(return_list, 0, NmzVectorToPyList(HS.first.getNum()));
    PyList_SetItem(
        return_list, 1,
        NmzVectorToPyList(libnormaliz::to_vector(HS.first.getDenom())));
    PyList_SetItem(return_list, 2, NmzToPyNumber(HS.first.getShift()));
    PyList_SetItem(return_list, 3, NmzToPyNumber(HS.second));
    return return_list;
}

template < typename Integer >
static PyObject*
NmzHilbertQuasiPolynomialToPyList(const libnormaliz::HilbertSeries& HS)
{
    vector< vector< Integer > > HQ = HS.getHilbertQuasiPolynomial();
    const size_t                n = HS.getPeriod();
    PyObject*                   return_list = PyList_New(n + 1);
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(return_list, i, NmzVectorToPyList(HQ[i]));
    }
    PyList_SetItem(return_list, n,
                   NmzToPyNumber(HS.getHilbertQuasiPolynomialDenom()));
    return return_list;
}

template < typename Integer >
static PyObject* NmzWeightedEhrhartQuasiPolynomialToPyList(
    const libnormaliz::IntegrationData& int_data)
{
    vector< vector< Integer > > ehrhart_qp =
        int_data.getWeightedEhrhartQuasiPolynomial();
    const size_t n = ehrhart_qp.size();
    PyObject*    return_list = PyList_New(n + 1);
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(return_list, i, NmzVectorToPyList(ehrhart_qp[i]));
    }
    PyList_SetItem(
        return_list, n,
        NmzToPyNumber(int_data.getWeightedEhrhartQuasiPolynomialDenom()));
    return return_list;
}

template < typename Integer >
static PyObject* NmzTriangleListToPyList(
    const pair<vector<libnormaliz::SHORTSIMPLEX<Integer> >, libnormaliz::Matrix<Integer> >& in)
{
    const size_t n = in.first.size();
    PyObject*    M = PyList_New(n);
    for (size_t i = 0; i < n; ++i) {
        // convert the pair
        PyObject* triple = PyList_New(3);
        PyList_SetItem(triple, 0,
                       NmzVectorToPyList< libnormaliz::key_t >(in.first[i].key));
        PyList_SetItem(triple, 1, NmzToPyNumber(in.first[i].vol));
        PyList_SetItem(triple, 2, NmzToPyNumber(libnormaliz::bool_to_bitset(in.first[i].Excluded)));
        PyList_SetItem(M, i, triple);
    }
    
    PyObject* Tr = PyList_New(2);
    PyList_SetItem(Tr, 0,M);
    PyList_SetItem(Tr, 1,NmzMatrixToPyList(in.second.get_elements()));
    return Tr;
}

template < typename Integer >
static PyObject* NmzPairVectorToPyList(
    const vector< pair< vector< libnormaliz::key_t >, Integer > >& in)
{
    const size_t n = in.size();
    PyObject*    M = PyList_New(n);
    for (size_t i = 0; i < n; ++i) {
        // convert the pair
        PyObject* pair = PyList_New(2);
        PyList_SetItem(pair, 0,
                       NmzVectorToPyList< libnormaliz::key_t >(in[i].first));
        PyList_SetItem(pair, 1, NmzToPyNumber(in[i].second));
        PyList_SetItem(M, i, pair);
    }
    return M;
}

template < typename Integer >
static PyObject*
NmzStanleyDataToPyList(const libnormaliz::STANLEYDATA< Integer >& StanleyData)
{
    PyObject* pair = PyList_New(2);
    PyList_SetItem(pair, 0,
                   NmzVectorToPyList< libnormaliz::key_t >(StanleyData.key));
    PyList_SetItem(pair, 1,
                   NmzMatrixToPyList(StanleyData.offsets.get_elements()));
    return pair;
}

template < typename Integer >
static PyObject* NmzStanleyDecToPyList(
    const std::pair<std::list<libnormaliz::STANLEYDATA<Integer> >, libnormaliz::Matrix<Integer> > & StanleyDec)
{
    const size_t n = StanleyDec.first.size();
    PyObject*    M = PyList_New(n);
    typename std::list< libnormaliz::STANLEYDATA< Integer > >::const_iterator S =
        StanleyDec.first.begin();
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(M, i, NmzStanleyDataToPyList(*S));
        ++S;
    }
    PyObject*  St=PyList_New(2);
    PyList_SetItem(St,0, M);
    PyList_SetItem(St, 1, NmzMatrixToPyList(StanleyDec.second.get_elements()) );
    return St;
}

template < typename Integer >
static PyObject* _NmzBasisChangeIntern(Cone< Integer >* C)
{
    Sublattice_Representation< Integer > bc = C->getSublattice();

    PyObject* res = PyList_New(3);
    PyList_SetItem(res, 0, NmzMatrixToPyList(bc.getEmbedding()));
    PyList_SetItem(res, 1, NmzMatrixToPyList(bc.getProjection()));
    PyList_SetItem(res, 2, NmzToPyNumber(bc.getAnnihilator()));
    // Dim, Rank, Equations and Congruences are already covered by special
    // functions ditto ExternalIndex
    return res;
}

static PyObject*
NmzFacelatticeToPython(const map<dynamic_bitset, int>& lattice)
{
    ssize_t   len = lattice.size();
    PyObject* list = PyList_New(len);
    ssize_t   curr = 0;
    for (auto it = lattice.begin(); it != lattice.end(); it++) {
        PyObject* list_int = PyList_New(2);
        PyList_SetItem(list_int, 0, NmzToPyNumber(it->first));
        PyList_SetItem(list_int, 1, NmzToPyNumber(it->second));
        PyList_SetItem(list, curr, list_int);
        curr++;
    }
    return list;
}

template < typename Integer >
static PyObject*
NmzAutomorphismsToPython(const AutomorphismGroup< Integer >& grp)
{

    PyObject* list = PyList_New(4);

    PyList_SetItem(list, 0, NmzToPyNumber(grp.getOrder()));

    PyObject* current = PyList_New(2);
    PyList_SetItem(current, 0, NmzMatrixToPyList(grp.getExtremeRaysPerms()));
    PyList_SetItem(current, 1, NmzMatrixToPyList(grp.getExtremeRaysOrbits()));
    PyList_SetItem(list, 1, current);

    current = PyList_New(2);
    PyList_SetItem(current, 0, NmzMatrixToPyList(grp.getVerticesPerms()));
    PyList_SetItem(current, 1, NmzMatrixToPyList(grp.getVerticesOrbits()));
    PyList_SetItem(list, 2, current);

    current = PyList_New(2);
    PyList_SetItem(current, 0,
                   NmzMatrixToPyList(grp.getSupportHyperplanesPerms()));
    PyList_SetItem(current, 1,
                   NmzMatrixToPyList(grp.getSupportHyperplanesOrbits()));
    PyList_SetItem(list, 3, current);

    return list;
}

/***************************************************************************
 *
 * PyCapsule handler functions
 *
 ***************************************************************************/

#ifdef ENFNORMALIZ
struct NumberFieldCone {
    renf_class*              nf;
    Cone< renf_elem_class >* cone;
};
#endif

static void delete_cone_mpz(PyObject* cone)
{
    Cone< mpz_class >* cone_ptr = reinterpret_cast< Cone< mpz_class >* >(
        PyCapsule_GetPointer(cone, cone_name));
    delete cone_ptr;
}

static void delete_cone_long(PyObject* cone)
{
    Cone< long long >* cone_ptr = reinterpret_cast< Cone< long long >* >(
        PyCapsule_GetPointer(cone, cone_name_long));
    delete cone_ptr;
}

#ifdef ENFNORMALIZ
static void delete_cone_renf(PyObject* cone)
{
    NumberFieldCone* cone_ptr = reinterpret_cast< NumberFieldCone* >(
        PyCapsule_GetPointer(cone, cone_name_renf));
    delete cone_ptr->cone;
    // delete cone_ptr->nf;
}
#endif

static Cone< long long >* get_cone_long(PyObject* cone)
{
    return reinterpret_cast< Cone< long long >* >(
        PyCapsule_GetPointer(cone, cone_name_long));
}

static Cone< mpz_class >* get_cone_mpz(PyObject* cone)
{
    return reinterpret_cast< Cone< mpz_class >* >(
        PyCapsule_GetPointer(cone, cone_name));
}

#ifdef ENFNORMALIZ
static Cone< renf_elem_class >* get_cone_renf(PyObject* cone)
{
    NumberFieldCone* cone_ptr = reinterpret_cast< NumberFieldCone* >(
        PyCapsule_GetPointer(cone, cone_name_renf));
    return cone_ptr->cone;
}

static renf_class* get_cone_renf_renf(PyObject* cone)
{
    NumberFieldCone* cone_ptr = reinterpret_cast< NumberFieldCone* >(
        PyCapsule_GetPointer(cone, cone_name_renf));
    return cone_ptr->nf;
}
#endif

static PyObject* pack_cone(Cone< mpz_class >* C, void* dummy = nullptr)
{
    return PyCapsule_New(reinterpret_cast< void* >(C), cone_name,
                         &delete_cone_mpz);
}

static PyObject* pack_cone(Cone< long long >* C, void* dummy = nullptr)
{
    return PyCapsule_New(reinterpret_cast< void* >(C), cone_name_long,
                         &delete_cone_long);
}

#ifdef ENFNORMALIZ
static PyObject* pack_cone(Cone< renf_elem_class >* C, void* nf)
{
    NumberFieldCone* cone_ptr = new NumberFieldCone();
    cone_ptr->nf = reinterpret_cast< renf_class* >(nf);
    cone_ptr->cone = C;
    return PyCapsule_New(reinterpret_cast< void* >(cone_ptr), cone_name_renf,
                         &delete_cone_renf);
}
#endif

static bool is_cone(PyObject* cone)
{
    if (PyCapsule_CheckExact(cone)) {
        const char *name = PyCapsule_GetName(cone);
        return !strcmp(name, cone_name) || !strcmp(name, cone_name_long) ||
               !strcmp(name, cone_name_renf);
    }
    return false;
}

static bool is_cone_mpz(PyObject* cone)
{
    if (PyCapsule_CheckExact(cone)) {
        const char *name = PyCapsule_GetName(cone);
        return !strcmp(name, cone_name);
    }
    return false;
}

static bool is_cone_long(PyObject* cone)
{
    if (PyCapsule_CheckExact(cone)) {
        const char *name = PyCapsule_GetName(cone);
        return !strcmp(name, cone_name_long);
    }
    return false;
}

#ifdef ENFNORMALIZ
static bool is_cone_renf(PyObject* cone)
{
    if (PyCapsule_CheckExact(cone)) {
        const char *name = PyCapsule_GetName(cone);
        return !strcmp(name, cone_name_renf);
    }
    return false;
}
#endif

/***************************************************************************
 *
 * Cone property list
 *
 ***************************************************************************/

/*
@Name NmzListConeProperties
@Arguments none
@Description
Returns two lists of strings.
The first list are all cone properties that define compute
goals in Normaliz (see Normaliz manual for details)
The second list are all cone properties that define internal
control flow control in Normaliz, and which should not be used
to get results of computations.
All entries of the first list can be passed to NmzResult
to get the result of a normaliz computation.
All entries of the second list can be passed to NmzCompute
to set different options for Normaliz computations.
*/
static PyObject* NmzListConeProperties(PyObject* args)
{
    FUNC_BEGIN

    PyObject* return_list = PyList_New(2);

    ConeProperties goals = libnormaliz::all_goals();
    ConeProperties options = libnormaliz::all_options();
    
    int number_goals = goals.count();
    int number_options = options.count();

    PyObject* goal_list = PyList_New(number_goals);
    PyObject* option_list = PyList_New(number_options);

    PyList_SetItem(return_list, 0, goal_list);
    PyList_SetItem(return_list, 1, option_list);

    int list_position = 0;
    for (int i = 0; i < libnormaliz::ConeProperty::EnumSize; i++) {
        if (goals.test(static_cast< libnormaliz::ConeProperty::Enum >(i))) {
            string name = libnormaliz::toString(
                static_cast< libnormaliz::ConeProperty::Enum >(i));
            PyList_SetItem(goal_list, list_position, StringToPyUnicode(name));
            list_position++;
        }
    }

    list_position = 0;
    for (int i = 0; i < libnormaliz::ConeProperty::EnumSize; i++) {
        if (options.test(static_cast< libnormaliz::ConeProperty::Enum >(i))) {
            string name = libnormaliz::toString(
                static_cast< libnormaliz::ConeProperty::Enum >(i));
            PyList_SetItem(option_list, list_position,
                           StringToPyUnicode(name));
            list_position++;
        }
    }

    return return_list;

    FUNC_END
}

/***************************************************************************
 *
 * NmzCone
 *
 ***************************************************************************/

template < typename Integer >
static PyObject* _NmzConeIntern(PyObject* kwargs)
{
    map< InputType, vector< vector< mpq_class > > > input;

    bool   grading_polynomial = false;
    string polynomial;

    if (kwargs != NULL) {
        PyObject* keys = PyDict_Keys(kwargs);
        PyObject* values = PyDict_Values(kwargs);
        const int length = PySequence_Size(keys);
        for (int i = 0; i < length; i++) {
            string type_string =
                PyUnicodeToString(PySequence_GetItem(keys, i));
            if (type_string == "CreateAsLongLong") {
                continue;
            }
            PyObject* current_value = PySequence_GetItem(values, i);
            if (current_value == Py_None)
                continue;
            if (type_string.compare("polynomial") == 0) {
                polynomial = PyUnicodeToString(current_value);
                grading_polynomial = true;
                continue;
            }
            vector< vector< mpq_class > > Mat;
            try {
                PyInputToNmz(Mat, current_value);
            }
            catch (PyNormalizInputException& e) {
                PyErr_SetString(PyNormaliz_cppError,
                                (string("When parsing ") + type_string +
                                 ": " + e.what_message())
                                    .c_str());
                return NULL;
            }
            input[libnormaliz::to_type(type_string)] = Mat;
        }
    }

    Cone< Integer >* C = new Cone< Integer >(input);

    if (grading_polynomial) {
        C->setPolynomial(polynomial);
    }

    PyObject* return_container = pack_cone(C);

    return return_container;
}

#ifdef ENFNORMALIZ
static PyObject* _NmzConeIntern_renf(PyObject* kwargs)
{

    FUNC_BEGIN
    PyObject* number_field_data =
        PyDict_GetItemString(kwargs, "number_field");
    if (number_field_data == NULL) {
        PyErr_SetString(PyNormaliz_cppError, "no number field data given");
        return NULL;
    }
    if (!PySequence_Check(number_field_data)) {
        PyErr_SetString(PyNormaliz_cppError,
                        "number field data must be a list");
        return NULL;
    }
    if (PySequence_Size(number_field_data) != 3) {
        PyErr_SetString(
            PyNormaliz_cppError,
            "number field data must be a list with three entries");
        return NULL;
    }


    renf_class* renf;
    // number_field_data contains 3 entries: poly, var, emb
    // All are strings
    string poly = PyUnicodeToString(PySequence_GetItem(number_field_data, 0));
    string var = PyUnicodeToString(PySequence_GetItem(number_field_data, 1));
    string emb = PyUnicodeToString(PySequence_GetItem(number_field_data, 2));
    renf = new renf_class(poly.c_str(), var.c_str(), emb.c_str());

    map< InputType, vector< vector< renf_elem_class > > > input;


    /* Do not delete entry of kwargs dict, as it might not
       be owned by the cone constructor */
    // PyDict_DelItemString(kwargs,"number_field");
    if (kwargs != NULL) {
        PyObject* keys = PyDict_Keys(kwargs);
        PyObject* values = PyDict_Values(kwargs);
        const int length = PySequence_Size(keys);
        for (int i = 0; i < length; i++) {
            string type_string =
                PyUnicodeToString(PySequence_GetItem(keys, i));
            if (type_string == "number_field")
                continue;
            PyObject* current_value = PySequence_GetItem(values, i);
            if (current_value == Py_None)
                continue;
            vector< vector< renf_elem_class > > Mat;
            try {
                prepare_nf_input(Mat, current_value, renf);
            }
            catch (PyNormalizInputException& e) {
                PyErr_SetString(PyNormaliz_cppError,
                                (string("When parsing ") + type_string +
                                 ": " + e.what_message())
                                    .c_str());
                return NULL;
            }
            input[libnormaliz::to_type(type_string)] = Mat;
        }
    }

    Cone< renf_elem_class >* C = new Cone< renf_elem_class >(input);
    C->setRenf(renf);

    PyObject* return_container = pack_cone(C, renf);

    return return_container;
    FUNC_END
}
#endif

/*
@Name NmzCone
@Arguments <keywords>
@Description
Constructs a normaliz cone object. The keywords must be
Normaliz input types, and the values for the keys matrices
(consisting of either Longs, Floats, or strings for rationals),
lists for single vector input types, or bools for boolean input type.
Special cases are a string describing a polynomial for the polynomial
input type, and the CreateAsLongLong keyword to restrict normaliz computations
to machine integers instead of arbitrary precision numbers.
*/
static PyObject* _NmzCone(PyObject* self, PyObject* args, PyObject* kwargs)
{
    FUNC_BEGIN

    static const char* string_for_long = "CreateAsLongLong";
    PyObject* create_as_long_long = StringToPyUnicode(string_for_long);
#ifdef ENFNORMALIZ
    static const char* string_for_renf = "number_field";
    PyObject*          create_as_renf = StringToPyUnicode(string_for_renf);
#endif

    if (kwargs != NULL && PyDict_Contains(kwargs, create_as_long_long) == 1) {
        create_as_long_long = PyDict_GetItem(kwargs, create_as_long_long);
        if (create_as_long_long == Py_True) {
            return _NmzConeIntern< long long >(kwargs);
        }
    }
#ifdef ENFNORMALIZ
    else if (kwargs != NULL && PyDict_Contains(kwargs, create_as_renf) == 1) {
        return _NmzConeIntern_renf(kwargs);
    }
#endif
    return _NmzConeIntern< mpz_class >(kwargs);
    FUNC_END
}

/*
@Name NmzConeCopy
@Arguments Cone
@Description
Returns a copy of the cone.
*/
static PyObject* _NmzConeCopy(PyObject* self, PyObject* args)
{
    FUNC_BEGIN
    PyObject* cone = PyTuple_GetItem(args, 0);
    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        Cone< mpz_class >* new_cone = new Cone< mpz_class >(*cone_ptr);
        return pack_cone(new_cone);
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        Cone< long long >* new_cone = new Cone< long long >(*cone_ptr);
        return pack_cone(new_cone);
    }
#ifdef ENFNORMALIZ
    else if (is_cone_renf(cone)) {
        Cone< renf_elem_class >* cone_ptr = get_cone_renf(cone);
        Cone< renf_elem_class >* new_cone =
            new Cone< renf_elem_class >(*cone_ptr);
        return pack_cone(new_cone, get_cone_renf_renf(cone));
    }
#endif
    Py_RETURN_NONE;
    FUNC_END
}

/***************************************************************************
 *
 * NmzHilbertSeries
 *
 ***************************************************************************/

/* SUPERFLUOUS
template < typename Integer >
static PyObject* NmzHilbertSeries(Cone< Integer >* C, PyObject* args)
{
    FUNC_BEGIN

    const int arg_len = PyTuple_Size(args);

    if (arg_len == 1) {
        bool is_HSOP = C->isComputed(libnormaliz::ConeProperty::HSOP);
        return NmzHilbertSeriesToPyList(C->getHilbertSeries(), is_HSOP);
    }

    PyObject* is_HSOP = PyTuple_GetItem(args, 1);

    if (is_HSOP == Py_True) {
        if (!C->isComputed(libnormaliz::ConeProperty::HSOP))
            C->compute(libnormaliz::ConeProperty::HSOP);
        return NmzHilbertSeriesToPyList(C->getHilbertSeries(), true);
    }
    else {
        return NmzHilbertSeriesToPyList(C->getHilbertSeries(), false);
    }
    FUNC_END
}


static PyObject* NmzHilbertSeries_Outer(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    TempSignalHandler tmpHandler; // use custom signal handler

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        return NmzHilbertSeries(cone_ptr, args);
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        return NmzHilbertSeries(cone_ptr, args);
    }
    else {
        PyErr_SetString(PyNormaliz_cppError,
                        "Hilbert series not available for renf cone");
        return NULL;
    }
    FUNC_END
}
*/

/***************************************************************************
 *
 * NmzCompute
 *
 ***************************************************************************/


template < typename Integer >
static PyObject* _NmzCompute(Cone< Integer >* C, PyObject* args)
{
    FUNC_BEGIN

    const int arg_len = PyTuple_Size(args);

    PyObject* to_compute;

    if (arg_len == 2) {
        PyObject* first_arg = PyTuple_GetItem(args, 1);
        if (PyList_Check(first_arg) || PyTuple_Check(first_arg)) {
            to_compute = first_arg;
            Py_IncRef(to_compute);
        }
        else {
            to_compute = PyList_New(1);
            int result = PyList_SetItem(to_compute, 0, first_arg);
            if (result != 0) {
                PyErr_SetString(PyNormaliz_cppError,
                                "List could not be created");
                Py_DecRef(to_compute);
                return NULL;
            }
        }
    }
    else {
        to_compute = PyList_New(arg_len - 1);
        for (int i = 1; i < arg_len; i++) {
            PyList_SetItem(to_compute, i-1, PyTuple_GetItem(args, i));
        }
    }

    ConeProperties propsToCompute;
    const int      n = PySequence_Size(to_compute);

    for (int i = 0; i < n; ++i) {
        PyObject* prop = PySequence_GetItem(to_compute, i);
        if (!string_check(prop)) {
            PyErr_SetString(PyNormaliz_cppError,
                            "All elements must be strings");
            Py_DecRef(to_compute);
            return NULL;
        }
        string prop_str(PyUnicodeToString(prop));
        propsToCompute.set(libnormaliz::toConeProperty(prop_str));
    }

    ConeProperties notComputed = C->compute(propsToCompute);

    // Cone.compute returns the not computed properties
    // we return a bool, true when everything requested was computed
    Py_DecRef(to_compute);
    return BoolToPyBool(notComputed.goals().none());
    FUNC_END
}


static PyObject* _NmzCompute_Outer(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);

    PyObject* result = NULL;

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    TempSignalHandler tmpHandler; // use custom signal handler

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        result = _NmzCompute(cone_ptr, args);
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        result = _NmzCompute(cone_ptr, args);
    }
#ifdef ENFNORMALIZ
    else if (is_cone_renf(cone)) {
        Cone< renf_elem_class >* cone_ptr = get_cone_renf(cone);
        result = _NmzCompute(cone_ptr, args);
    }
#endif

    return result;

    FUNC_END
}

/***************************************************************************
 *
 * NmzModify
 *
 ***************************************************************************/

template<typename Integer>
PyObject* _NmzModify(Cone<Integer>* cone, PyObject* args)
{
    string property = PyUnicodeToString( PyTuple_GetItem(args, 1) );
    PyObject* matrix_py = PyTuple_GetItem(args,2);

    vector<vector<Integer>> mat;
    PyInputToNmz( mat,matrix_py );

    cone->modifyCone(libnormaliz::to_type(property),mat);
    Py_RETURN_TRUE;

}

#ifdef ENFNORMALIZ
PyObject* _NmzModify_Renf(Cone<renf_elem_class>* cone, renf_class* nf, PyObject* args)
{
    string property = PyUnicodeToString( PyTuple_GetItem(args, 1) );
    PyObject* matrix_py = PyTuple_GetItem(args,2);

    vector<vector<renf_elem_class>> mat;
    prepare_nf_input( mat,matrix_py,nf );

    cone->modifyCone(libnormaliz::to_type(property),mat);
    Py_RETURN_TRUE;

}
#endif

PyObject* _NmzModify_Outer(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    TempSignalHandler tmpHandler; // use custom signal handler

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        return _NmzModify(cone_ptr, args);
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        return _NmzModify(cone_ptr, args);
    }
#ifdef ENFNORMALIZ
    else if (is_cone_renf(cone)) {
        Cone< renf_elem_class >* cone_ptr = get_cone_renf(cone);
        renf_class* nf = get_cone_renf_renf(cone);
        return _NmzModify_Renf(cone_ptr, nf, args);
    }
#endif

    Py_RETURN_TRUE;

    FUNC_END
}

/***************************************************************************
 *
 * NmzIsComputed
 *
 ***************************************************************************/

/*
@Name NmzIsComputed
@Arguments <cone>, <property_string>
@Desctiption
Returns if the cone property <property_string> is computed in the cone <cone>.
*/
template < typename Integer >
static PyObject* NmzIsComputed(Cone< Integer >* C, PyObject* prop)
{
    FUNC_BEGIN

    libnormaliz::ConeProperty::Enum p =
        libnormaliz::toConeProperty(PyUnicodeToString(prop));

    return BoolToPyBool(C->isComputed(p));

    FUNC_END
}

static PyObject* NmzIsComputed_Outer(PyObject* self, PyObject* args)
{
    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);
    PyObject* to_compute = PyTuple_GetItem(args, 1);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        return NmzIsComputed(cone_ptr, to_compute);
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        return NmzIsComputed(cone_ptr, to_compute);
    }
#ifdef ENFNORMALIZ
    else if (is_cone_renf(cone)) {
        Cone< renf_elem_class >* cone_ptr = get_cone_renf(cone);
        return NmzIsComputed(cone_ptr, to_compute);
    }
#endif
    Py_RETURN_FALSE;

    FUNC_END
}

/***************************************************************************
 *
 * NmzSetGrading
 *
 ***************************************************************************/

template < typename Integer >
static PyObject* NmzSetGrading_inner(Cone< Integer >* cone, PyObject* grad)
{
    vector< Integer > grad_c;
    bool result = PyListToNmz(grad_c, grad);
    if (!result) {
        PyErr_SetString(PyNormaliz_cppError,
                        "grading argument is not an integer list");
        return NULL;
    }
    cone->resetGrading(grad_c);
    Py_RETURN_NONE;
}

#ifdef ENFNORMALIZ
template <>
PyObject* NmzSetGrading_inner(Cone< renf_elem_class >* cone, PyObject* grad)
{
    vector< renf_elem_class > grad_renf;
    vector<vector< renf_elem_class> > grad_mat; // a cheap way to convert vectors
    PyObject*  PyHelpMat = PyList_New(1);     // better: rebuild conversion to renf
    PyList_SetItem(PyHelpMat, 0, grad);    
    prepare_nf_input(grad_mat, PyHelpMat,cone->getRenf());
    grad_renf = grad_mat[0];

    cone->resetGrading(grad_renf);
    Py_RETURN_NONE;
}
#endif

static PyObject* NmzSetGrading(PyObject* self, PyObject* args)
{
    FUNC_BEGIN
    PyObject* cone = PyTuple_GetItem(args, 0);
    PyObject* grading_py = PyTuple_GetItem(args, 1);
    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }
    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        return NmzSetGrading_inner(cone_ptr, grading_py);
    }
     if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        return NmzSetGrading_inner(cone_ptr, grading_py);
    }
#ifdef ENFNORMALIZ
     if (is_cone_renf(cone)) {
        Cone< renf_elem_class >* cone_ptr = get_cone_renf(cone);
        return NmzSetGrading_inner(cone_ptr, grading_py);
    }
#endif
    FUNC_END
    
    Py_RETURN_NONE;
}

/***************************************************************************
 *
 * NmzSetProjectionCoords
 *
 ***************************************************************************/

template < typename Integer >
static PyObject* NmzSetProjectionCoords_inner(Cone< Integer >* cone, PyObject* coords)
{
    vector< Integer > coords_c;
    bool  result = PyListToNmz(coords_c, coords);
    if (!result) {
        PyErr_SetString(PyNormaliz_cppError,
                        " is not an integer list");
        return NULL;
    }
    for(size_t i=0; i< coords_c.size(); ++i){
        if(coords_c[i]!=0 && coords_c[i]!=1)
            PyErr_SetString(PyNormaliz_cppError, "Projection coordinates must be 0 or 1");
    }
            
    cone->resetProjectionCoords(coords_c);
    Py_RETURN_NONE;
}

#ifdef ENFNORMALIZ
template <>
PyObject* NmzSetProjectionCoords_inner(Cone< renf_elem_class >* cone, PyObject* coords)
{
    vector< renf_elem_class > coords_renf;
    vector<vector< renf_elem_class> > coords_mat; // a cheap way to convert vectors
    PyObject*  PyHelpMat = PyList_New(1);     // better: rebuild conversion to renf
    PyList_SetItem(PyHelpMat, 0, coords);    
    prepare_nf_input(coords_mat, PyHelpMat,cone->getRenf());
    coords_renf = coords_mat[0];

    cone->resetGrading(coords_renf);
    Py_RETURN_NONE;
}
#endif


static PyObject* NmzSetProjectionCoords(PyObject* self, PyObject* args)
{
    FUNC_BEGIN
    PyObject* cone = PyTuple_GetItem(args, 0);
    PyObject* coords_py = PyTuple_GetItem(args, 1);
    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }
    if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        return NmzSetProjectionCoords_inner(cone_ptr, coords_py);
    }
    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        return NmzSetProjectionCoords_inner(cone_ptr, coords_py);
    }
#ifdef ENFNORMALIZ
    if (is_cone_renf(cone)) {
        Cone< renf_elem_class >* cone_ptr = get_cone_renf(cone);
        return NmzSetProjectionCoords_inner(cone_ptr, coords_py);
    }
#endif
    FUNC_END
    Py_RETURN_NONE;
}

/***************************************************************************
 *
 * NmzResult
 *
 ***************************************************************************/

/*
@Name NmzResult
@Arguments <cone>,<cone property string>,<keys>
@Description
Returns the cone property belonging to the string <cone property string> of
cone <cone>. Please see the Normaliz manual for details on which cone
properties are available. Here are some special outputs that might differ from
Normaliz:
* HilbertSeries and WeightedEhrhartSeries
  The returned object is a list with three entries: The first one describes
the numerator of the hilbert series, the second one the denominator, and the
last one is the shift. If you pass the HSOP option, output will be done in
HSOP format.
* Grading
  Returns a list with two entries. First is the grading, second one is the
grading denominator.
* Sublattice
  Returns a list with three entries. First is the embedding of the sublattice,
second is the projection third is the annihilator.
* IntegerHull and ProjectCone return new cones.
* StanleyDec
  Returns a list containing the Stanley decomposition. All entries are
2-tuples. First entry in the tuple is the key, second the decomposition data.
*/

template < typename Integer >
static PyObject*
_NmzResultImpl(Cone< Integer >* C, PyObject* prop_obj, void* nf = nullptr)
{

    string prop = PyUnicodeToString(prop_obj);

    libnormaliz::ConeProperty::Enum p = libnormaliz::toConeProperty(prop);

    ConeProperties notComputed;
    {
    TempSignalHandler tmpHandler; // use custom signal handler
    notComputed = C->compute(ConeProperties(p));
    }

    if (notComputed.goals().any()) {
        Py_RETURN_NONE;
    }

    // Handle standard cases
    libnormaliz::OutputType::Enum outputtype = libnormaliz::output_type(p);

    switch (p) {
                  
        case libnormaliz::ConeProperty::Triangulation:{
            return NmzTriangleListToPyList< Integer >(C->getTriangulation());
        }
        
        case libnormaliz::ConeProperty::ConeDecomposition:{
            return NmzTriangleListToPyList< Integer >(C->getConeDecomposition());
        }
            
        case libnormaliz::ConeProperty::AllGeneratorsTriangulation:
            return NmzTriangleListToPyList< Integer >(C->getTriangulation(
                libnormaliz::ConeProperty::AllGeneratorsTriangulation));
    
        case libnormaliz::ConeProperty::LatticePointTriangulation:
            return NmzTriangleListToPyList< Integer >(C->getTriangulation(
               libnormaliz::ConeProperty::LatticePointTriangulation));

        case libnormaliz::ConeProperty::UnimodularTriangulation:
            return NmzTriangleListToPyList< Integer >(C->getTriangulation(
               libnormaliz::ConeProperty::UnimodularTriangulation));

        case libnormaliz::ConeProperty::HilbertSeries: {
            bool is_HSOP = C->isComputed(libnormaliz::ConeProperty::HSOP);
            return NmzHilbertSeriesToPyList(C->getHilbertSeries(), is_HSOP);
        }

        case libnormaliz::ConeProperty::EhrhartSeries: {
            bool is_HSOP = C->isComputed(libnormaliz::ConeProperty::HSOP);
            return NmzHilbertSeriesToPyList(C->getEhrhartSeries(), is_HSOP);
        }

        case libnormaliz::ConeProperty::WeightedEhrhartSeries:
            return NmzWeightedEhrhartSeriesToPyList( C->getWeightedEhrhartSeries());

        // though Grading has the return type vector<Integer> we make it
        // a complex struture within PyNormaliz since we want to combine it
        // with the grading denominator
        case libnormaliz::ConeProperty::Grading: {
            vector< Integer > grad = C->getGrading();
            Integer           denom = C->getGradingDenom();
            PyObject*         return_list = PyList_New(2);
            PyList_SetItem(return_list, 0, NmzVectorToPyList(grad));
            PyList_SetItem(return_list, 1, NmzToPyNumber(denom));
            return return_list;
        }

        case libnormaliz::ConeProperty::StanleyDec:
            return NmzStanleyDecToPyList(C->getStanleyDec());

        case libnormaliz::ConeProperty::InclusionExclusionData:
            return NmzPairVectorToPyList< long >(
                C->getInclusionExclusionData());

        /* returned as a matrix, no need to make it a complex property
        case libnormaliz::ConeProperty::Equations:
            return NmzMatrixToPyList(C->getSublattice().getEquations());

        case libnormaliz::ConeProperty::Congruences:
            return NmzMatrixToPyList(C->getSublattice().getCongruences());
        */

        case libnormaliz::ConeProperty::Sublattice:
            return _NmzBasisChangeIntern(C);

        case libnormaliz::ConeProperty::ExternalIndex:
            return NmzToPyNumber(C->getSublattice().getExternalIndex());

        case libnormaliz::ConeProperty::IntegerHull: {
            Cone< Integer >* hull =
                new Cone< Integer >(C->getIntegerHullCone());
            return pack_cone(hull, nf);
        }

        case libnormaliz::ConeProperty::ProjectCone: {
            Cone< Integer >* projection =
                new Cone< Integer >(C->getProjectCone());
            return pack_cone(projection, nf);
        }

        case libnormaliz::ConeProperty::HilbertQuasiPolynomial:
            return NmzHilbertQuasiPolynomialToPyList< mpz_class >(
                C->getHilbertSeries());

        case libnormaliz::ConeProperty::EhrhartQuasiPolynomial:
            return NmzHilbertQuasiPolynomialToPyList< mpz_class >(
                C->getEhrhartSeries());

        case libnormaliz::ConeProperty::WeightedEhrhartQuasiPolynomial:
            return NmzWeightedEhrhartQuasiPolynomialToPyList< mpz_class >(
                C->getIntData());
            
        case libnormaliz::ConeProperty::ClassGroup:
            return NmzVectorToPyList(C->getClassGroup());

        case libnormaliz::ConeProperty::FVector:
            return NmzVectorToPyList(C->getFVector());
            
        case libnormaliz::ConeProperty::DualFVector:
            return NmzVectorToPyList(C->getDualFVector());

        case libnormaliz::ConeProperty::FaceLattice:
            return NmzFacelatticeToPython(C->getFaceLattice());
            
        case libnormaliz::ConeProperty::DualFaceLattice:
            return NmzFacelatticeToPython(C->getDualFaceLattice());

        case libnormaliz::ConeProperty::Automorphisms:
            return NmzAutomorphismsToPython(C->getAutomorphismGroup(
                libnormaliz::ConeProperty::Automorphisms));

        case libnormaliz::ConeProperty::AmbientAutomorphisms:
            return NmzAutomorphismsToPython(C->getAutomorphismGroup(
                libnormaliz::ConeProperty::AmbientAutomorphisms));

        case libnormaliz::ConeProperty::CombinatorialAutomorphisms:
            return NmzAutomorphismsToPython(C->getAutomorphismGroup(
                libnormaliz::ConeProperty::CombinatorialAutomorphisms));

        case libnormaliz::ConeProperty::RationalAutomorphisms:
            return NmzAutomorphismsToPython(C->getAutomorphismGroup(
                libnormaliz::ConeProperty::RationalAutomorphisms));

        case libnormaliz::ConeProperty::EuclideanAutomorphisms:
            return NmzAutomorphismsToPython(C->getAutomorphismGroup(
                libnormaliz::ConeProperty::EuclideanAutomorphisms));

        case libnormaliz::ConeProperty::Incidence:
            return NmzVectorToPyList(C->getIncidence());

        default: {
            switch (outputtype) {
                case libnormaliz::OutputType::Matrix:
                    return NmzMatrixToPyList(C->getMatrixConeProperty(p));
                case libnormaliz::OutputType::MatrixFloat:
                    return NmzMatrixToPyList(
                        C->getFloatMatrixConeProperty(p));
                case libnormaliz::OutputType::Vector:
                    return NmzVectorToPyList(C->getVectorConeProperty(p));
                case libnormaliz::OutputType::Integer:
                    return NmzToPyNumber(C->getIntegerConeProperty(p));
                case libnormaliz::OutputType::GMPInteger:
                    return NmzToPyNumber(C->getGMPIntegerConeProperty(p));
                case libnormaliz::OutputType::Rational:
                    return NmzToPyNumber(C->getRationalConeProperty(p));
                case libnormaliz::OutputType::FieldElem:
                    return NmzToPyNumber(C->getFieldElemConeProperty(p));
                case libnormaliz::OutputType::Float:
                    return NmzToPyNumber(C->getFloatConeProperty(p));
                case libnormaliz::OutputType::MachineInteger:
                    return NmzToPyNumber(C->getMachineIntegerConeProperty(p));
                case libnormaliz::OutputType::Bool:
                    return BoolToPyBool(C->getBooleanConeProperty(p));
                case libnormaliz::OutputType::Void: {
                    PyErr_SetString(PyNormaliz_cppError,
                                    "ConeProperty is input-only");
                    return NULL;
                }
                case libnormaliz::OutputType::Complex: {
                    PyErr_SetString(PyNormaliz_cppError,
                                    "This should never happen");
                    return NULL;
                }
            }
        }
    }
    Py_RETURN_NONE;
}

static PyObject* _NmzResult(PyObject* self, PyObject* args, PyObject* kwargs)
{

FUNC_BEGIN
    
RationalHandler = NULL;
FloatHandler = NULL;

#ifdef ENFNORMALIZ
NumberfieldElementHandler = NULL;
#endif

VectorHandler = NULL;
MatrixHandler = NULL;
    
    if(PyTuple_Size(args)!=2){
        PyErr_SetString(PyNormaliz_cppError, "Exactly one computation goal required for NmzResult");
        return NULL;        
    }

    PyObject* cone = PyTuple_GetItem(args, 0);
    PyObject* prop = PyTuple_GetItem(args, 1);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    if (!string_check(prop)) {
        PyErr_SetString(PyNormaliz_cppError,
                        "Second argument must be a unicode string");
        return NULL;
    }

    if (kwargs) {
        RationalHandler = PyDict_GetItemString(kwargs, "RationalHandler");
        FloatHandler = PyDict_GetItemString(kwargs, "FloatHandler");
#ifdef ENFNORMALIZ
        NumberfieldElementHandler =
            PyDict_GetItemString(kwargs, "NumberfieldElementHandler");
#endif
        VectorHandler = PyDict_GetItemString(kwargs, "VectorHandler");
        MatrixHandler = PyDict_GetItemString(kwargs, "MatrixHandler");
    }

    PyObject* result = NULL;

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        result = _NmzResultImpl(cone_ptr, prop);
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        result = _NmzResultImpl(cone_ptr, prop);
    }
#ifdef ENFNORMALIZ
    else if (is_cone_renf(cone)) {
        Cone< renf_elem_class >* cone_ptr = get_cone_renf(cone);
        result = _NmzResultImpl(
            cone_ptr, prop,
            reinterpret_cast< void* >(get_cone_renf_renf(cone)));
    }
#endif

    RationalHandler = NULL;
#ifdef ENFNORMALIZ
    NumberfieldElementHandler = NULL;
#endif
    VectorHandler = NULL;
    MatrixHandler = NULL;

    return result;

    FUNC_END
}

/***************************************************************************
 *
 * Python verbosity
 *
 ***************************************************************************/

static PyObject* NmzSetVerboseDefault(PyObject* self, PyObject* args)
{
    FUNC_BEGIN
    PyObject* value = PyTuple_GetItem(args, 0);
    if (value != Py_True && value != Py_False) {
        PyErr_SetString(PyNormaliz_cppError,
                        "Argument must be True or False");
        return NULL;
    }
    return BoolToPyBool(libnormaliz::setVerboseDefault(value == Py_True));
    FUNC_END
}

template < typename Integer >
static PyObject* NmzSetVerbose(Cone< Integer >* C, PyObject* value)
{
    FUNC_BEGIN
    return BoolToPyBool(C->setVerbose(value == Py_True));
    FUNC_END
}

static PyObject* NmzSetVerbose_Outer(PyObject* self, PyObject* args)
{
    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    PyObject* value = PyTuple_GetItem(args, 1);
    if (value != Py_True && value != Py_False) {
        PyErr_SetString(PyNormaliz_cppError,
                        "Second argument must be True or False");
        return NULL;
    }

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        return NmzSetVerbose(cone_ptr, value);
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        return NmzSetVerbose(cone_ptr, value);
    }
#ifdef ENFNORMALIZ
    else if (is_cone_renf(cone)) {
        Cone< renf_elem_class >* cone_ptr = get_cone_renf(cone);
        return NmzSetVerbose(cone_ptr, value);
    }
#endif
    Py_RETURN_NONE;

    FUNC_END
}

/***************************************************************************
 *
 * Get Polynomial
 *
 ***************************************************************************/

static PyObject* NmzGetPolynomial(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    TempSignalHandler tmpHandler; // use custom signal handler

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        return StringToPyUnicode((cone_ptr->getIntData()).getPolynomial());
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        return StringToPyUnicode((cone_ptr->getIntData()).getPolynomial());
    }
    else {
        PyErr_SetString(PyNormaliz_cppError,
                        "Polynomial not available for renf cone");
        return NULL;
    }

    FUNC_END
}

/***************************************************************************
 *
 * NrCoeffQuasiPol
 *
 ***************************************************************************/

static PyObject* NmzSetNrCoeffQuasiPol(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    PyObject* bound_py = PyTuple_GetItem(args, 1);

    TempSignalHandler tmpHandler; // use custom signal handler

    int  overflow;
    long bound = PyLong_AsLongLongAndOverflow(bound_py, &overflow);
    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        cone_ptr->setNrCoeffQuasiPol(bound);
        Py_RETURN_TRUE;
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        cone_ptr->setNrCoeffQuasiPol(bound);
        Py_RETURN_TRUE;
    }
    else {
        PyErr_SetString(PyNormaliz_cppError,
                        "Cannot set quasi polynomial coeffs for renf cone");
        return NULL;
    }

    FUNC_END
}

/***************************************************************************
 *
 * Polynomial
 *
 ***************************************************************************/

static PyObject* NmzSetPolynomial(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }
    
    TempSignalHandler tmpHandler; // use custom signal handler

    PyObject* poly_pi = PyTuple_GetItem(args, 1);
    
    if(!string_check(poly_pi)){
        PyErr_SetString(PyNormaliz_cppError, "Polynomual must be given as a string");
        return NULL;
    }
    TempSignalHandler tmpHandler1; // use custom signal handler
    
    string polynomial = PyUnicodeToString(poly_pi);
    
    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        cone_ptr->setPolynomial(polynomial);
        Py_RETURN_TRUE;
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        cone_ptr->setPolynomial(polynomial);
        Py_RETURN_TRUE;
    }
    else {
        PyErr_SetString(PyNormaliz_cppError,
                        "Polynomial cannot be set for renf cone");
        return NULL;
    }

    FUNC_END
}

/***************************************************************************
 *
 * FaceCodimBound
 *
 ***************************************************************************/

static PyObject* NmzSetFaceCodimBound(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    PyObject* bound_py = PyTuple_GetItem(args, 1);

    TempSignalHandler tmpHandler; // use custom signal handler

    int  overflow;
    long bound = PyLong_AsLongLongAndOverflow(bound_py, &overflow);
    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        cone_ptr->setFaceCodimBound(bound);
        Py_RETURN_TRUE;
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        cone_ptr->setFaceCodimBound(bound);
        Py_RETURN_TRUE;
    }
#ifdef ENFNORMALIZ
    else {
         Cone<renf_elem_class>* cone_ptr = get_cone_renf(cone);
         cone_ptr->setFaceCodimBound(bound);
         Py_RETURN_TRUE;
    }
#endif

    FUNC_END
}

/***************************************************************************
 *
 * Get Symmetrized cone
 *
 ***************************************************************************/

static PyObject* NmzSymmetrizedCone(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    TempSignalHandler tmpHandler; // use custom signal handler

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        Cone< mpz_class >& symm_cone = cone_ptr->getSymmetrizedCone();
        return pack_cone(new Cone< mpz_class >(symm_cone));
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        Cone< long long >& symm_cone = cone_ptr->getSymmetrizedCone();
        return pack_cone(new Cone< long long >(symm_cone));
    }
    else {
        PyErr_SetString(PyNormaliz_cppError,
                        "Symmetrized cone not available for renf cone");
        return NULL;
    }

    FUNC_END
}

/***************************************************************************
 *
 * Get expanded hilbert series
 *
 ***************************************************************************/

static PyObject* NmzGetHilbertSeriesExpansion(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);
    PyObject* py_degree = PyTuple_GetItem(args, 1);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    if (!PyLong_Check(py_degree)) {
        PyErr_SetString(PyNormaliz_cppError,
                        "Second argument must be a long");
        return NULL;
    }

    long                       degree = PyLong_AsLong(py_degree);
    libnormaliz::HilbertSeries HS;
    TempSignalHandler tmpHandler; // use custom signal handler

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        HS = cone_ptr->getHilbertSeries();
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        HS = cone_ptr->getHilbertSeries();
    }
    else {
        PyErr_SetString(
            PyNormaliz_cppError,
            "Hilbert series expansion not available for renf cone");
        return NULL;
    }

    HS.set_expansion_degree(degree);
    return NmzVectorToPyList(HS.getExpansion());

    FUNC_END
}


static PyObject* NmzGetWeightedEhrhartSeriesExpansion(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);
    PyObject* py_degree = PyTuple_GetItem(args, 1);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    if (!PyLong_Check(py_degree)) {
        PyErr_SetString(PyNormaliz_cppError,
                        "Second argument must be a long");
        return NULL;
    }

    long degree = PyLong_AsLong(py_degree);
    pair< libnormaliz::HilbertSeries, mpz_class > ES;
    TempSignalHandler tmpHandler; // use custom signal handler

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        ES = cone_ptr->getWeightedEhrhartSeries();
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        ES = cone_ptr->getWeightedEhrhartSeries();
    }
    else {
        PyErr_SetString(
            PyNormaliz_cppError,
            "Ehrhart series expansion not available for renf cone");
        return NULL;
    }

    ES.first.set_expansion_degree(degree);

    PyObject* return_list = PyList_New(2);
    PyList_SetItem(return_list, 0, NmzVectorToPyList(ES.first.getExpansion()));
        PyList_SetItem(return_list, 1, NmzToPyNumber(ES.second));
    return return_list;
    
    FUNC_END
}

static PyObject* NmzGetEhrhartSeriesExpansion(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* cone = PyTuple_GetItem(args, 0);
    PyObject* py_degree = PyTuple_GetItem(args, 1);

    if (!is_cone(cone)) {
        PyErr_SetString(PyNormaliz_cppError, "First argument must be a cone");
        return NULL;
    }

    if (!PyLong_Check(py_degree)) {
        PyErr_SetString(PyNormaliz_cppError,
                        "Second argument must be a long");
        return NULL;
    }

    long degree = PyLong_AsLong(py_degree);
    libnormaliz::HilbertSeries ES;
    TempSignalHandler tmpHandler; // use custom signal handler

    if (is_cone_mpz(cone)) {
        Cone< mpz_class >* cone_ptr = get_cone_mpz(cone);
        ES = cone_ptr->getEhrhartSeries();
    }
    else if (is_cone_long(cone)) {
        Cone< long long >* cone_ptr = get_cone_long(cone);
        ES = cone_ptr->getEhrhartSeries();
    }
    else {
        PyErr_SetString(
            PyNormaliz_cppError,
            "Ehrhart series expansion not available for renf cone");
        return NULL;
    }

    ES.set_expansion_degree(degree);
    return NmzVectorToPyList(ES.getExpansion());

    FUNC_END
}

/***************************************************************************
 *
 * Set number of threads
 *
 ***************************************************************************/

static PyObject* NmzSetNumberOfNormalizThreads(PyObject* self, PyObject* args)
{

    FUNC_BEGIN

    PyObject* num_threads = PyTuple_GetItem(args, 0);

    long num_threads_long;

    if (PyLong_Check(num_threads)) {
        num_threads_long = PyLong_AsLong(num_threads);
    }
#if PY_MAJOR_VERSION < 3
    else if (PyInt_Check(num_threads)) {
        num_threads_long = PyInt_AsLong(num_threads);
    }
#endif
    else {
        throw PyNormalizInputException("argument must be an integer");
        return NULL;
    }

    num_threads_long = libnormaliz::set_thread_limit(num_threads_long);

    return PyLong_FromLong(num_threads_long);

    FUNC_END
}

/***************************************************************************
 *
 * Check for various features
 *
 ***************************************************************************/

static PyObject* NmzHasEAntic(PyObject* self)
{
#ifdef ENFNORMALIZ
    Py_RETURN_TRUE;
#else
    Py_RETURN_FALSE;
#endif
}

static PyObject* NmzHasNauty(PyObject* self)
{
#ifdef NMZ_NAUTY
    Py_RETURN_TRUE;
#else
    Py_RETURN_FALSE;
#endif
}

static PyObject* NmzHasFlint(PyObject* self)
{
#ifdef NMZ_FLINT
    Py_RETURN_TRUE;
#else
    Py_RETURN_FALSE;
#endif
}

static PyObject* NmzHasCocoa(PyObject* self)
{
#ifdef NMZ_COCOA
    Py_RETURN_TRUE;
#else
    Py_RETURN_FALSE;
#endif
}

/***************************************************************************
 *
 * Write output file
 *
 ***************************************************************************/

static PyObject* NmzWriteOutputFile(PyObject* self, PyObject* args)
{
    FUNC_BEGIN

    if ((!PyTuple_Check(args)) || (PyTuple_Size(args) != 2)) {
        throw PyNormalizInputException(
            "The arguments must be a cone and a string");
        return NULL;
    }

    PyObject* cone_py = PyTuple_GetItem(args, 0);
    PyObject* filename_py = PyTuple_GetItem(args, 1);

    string filename = PyUnicodeToString(filename_py);

    if (is_cone_mpz(cone_py)) {
        Cone< mpz_class >* cone = get_cone_mpz(cone_py);
        cone->write_cone_output(filename);
        Py_RETURN_TRUE;
    }
    if (is_cone_long(cone_py)) {
        Cone< long long >* cone = get_cone_long(cone_py);
        cone->write_cone_output(filename);
        Py_RETURN_TRUE;
    }
#ifdef ENFNORMALIZ
    if (is_cone_renf(cone_py)) {
        Cone< renf_elem_class >* cone = get_cone_renf(cone_py);
        cone->write_cone_output(filename);
        Py_RETURN_TRUE;
    }
#endif
    Py_RETURN_FALSE;

    FUNC_END
}

/***************************************************************************
 *
 * Get renf precision
 *
 ***************************************************************************/

static PyObject* NmzGetRenfInfo(PyObject* self, PyObject* args)
{
    FUNC_BEGIN
#ifdef ENFNORMALIZ
    if( (!PyTuple_Check(args)) || (PyTuple_Size(args) != 1) ){
        throw PyNormalizInputException(
            "Only one argument allowed"
        );
        return NULL;
    }
    PyObject* cone_py = PyTuple_GetItem(args, 0);

    if(!is_cone_renf(cone_py)){
        throw PyNormalizInputException(
            "Only Renf cones allowed"
        );
        return NULL;
    }
    renf_class* renf = get_cone_renf_renf(cone_py);
    std::string minpoly_str;
    minpoly_str = fmpq_poly_get_str_pretty(renf->get_renf()->nf->pol, renf->gen_name.c_str());
    std::string res1 = arb_get_str(renf->get_renf()->emb, 64, 0);
    // long prec = renf->get_renf()->prec;
    return PyTuple_Pack(2, StringToPyUnicode(minpoly_str), StringToPyUnicode(res1));
#else
    return NULL;
#endif

    FUNC_END
}

/***************************************************************************
 *
 * Python init stuff
 *
 ***************************************************************************/

struct module_state {
    PyObject* error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

static PyObject* error_out(PyObject* m)
{
    struct module_state* st = GETSTATE(m);
    PyErr_SetString(st->error, "something bad happened");
    return NULL;
}

static PyMethodDef PyNormaliz_cppMethods[] = {
    {"error_out", (PyCFunction)error_out, METH_NOARGS, NULL},
    {"NmzCone", (PyCFunction)_NmzCone, METH_VARARGS | METH_KEYWORDS,
     "Create a cone"},
    {"NmzConeCopy", (PyCFunction)_NmzConeCopy, METH_VARARGS,
     "Copy an existing cone"},
    {"NmzCompute", (PyCFunction)_NmzCompute_Outer, METH_VARARGS,
     "Compute some stuff"},
    {"NmzIsComputed", (PyCFunction)NmzIsComputed_Outer, METH_VARARGS,
     "Check if property is computed "},
    {"NmzSetGrading", (PyCFunction)NmzSetGrading, METH_VARARGS,
     "Reset the grading of a cone"},
    {"NmzSetProjectionCoords", (PyCFunction)NmzSetProjectionCoords, METH_VARARGS,
     "Reset the projection coordinates"},
    {"NmzResult", (PyCFunction)_NmzResult, METH_VARARGS | METH_KEYWORDS,
     "Return cone property"},
    {"NmzSetVerboseDefault", (PyCFunction)NmzSetVerboseDefault, METH_VARARGS,
     "Set verbosity"},
    {"NmzSetVerbose", (PyCFunction)NmzSetVerbose_Outer, METH_VARARGS,
     "Set verbosity of cone"},
    {"NmzListConeProperties", (PyCFunction)NmzListConeProperties, METH_NOARGS,
     "List all available properties"},
    // {"NmzHilbertSeries", (PyCFunction)NmzHilbertSeries_Outer, METH_VARARGS,
    // "Returns Hilbert series, either HSOP or not"},
    {"NmzGetPolynomial", (PyCFunction)NmzGetPolynomial, METH_VARARGS,
     "Returns grading polynomial"},
    {"NmzSymmetrizedCone", (PyCFunction)NmzSymmetrizedCone, METH_VARARGS,
     "Returns symmetrized cone"},
    {"NmzSetNumberOfNormalizThreads",
     (PyCFunction)NmzSetNumberOfNormalizThreads, METH_VARARGS,
     "Sets the Normaliz thread limit"},
    {"NmzSetNrCoeffQuasiPol", (PyCFunction)NmzSetNrCoeffQuasiPol,
     METH_VARARGS, "Sets the number of computed coefficients for the quasi-polynomial"},
    {"NmzSetPolynomial", (PyCFunction)NmzSetPolynomial,
     METH_VARARGS, "Sets the polynomial for integration and weighted series"},
    {"NmzSetFaceCodimBound", (PyCFunction)NmzSetFaceCodimBound,
     METH_VARARGS, "Sets the maximal codimension for the computed faces"},
    {"NmzGetHilbertSeriesExpansion",
     (PyCFunction)NmzGetHilbertSeriesExpansion, METH_VARARGS,
     "Returns expansion of the hilbert series"},
    {"NmzGetEhrhartSeriesExpansion",
     (PyCFunction)NmzGetEhrhartSeriesExpansion, METH_VARARGS,
     "Returns expansion of the Ehrhart series"},
    {"NmzGetWeightedEhrhartSeriesExpansion",
     (PyCFunction)NmzGetWeightedEhrhartSeriesExpansion, METH_VARARGS,
     "Returns expansion of the weighted Ehrhart series"},

    {"NmzHasEAntic", (PyCFunction)NmzHasEAntic, METH_NOARGS,
     "Returns true if (Py)Normaliz was compiled with e-antic support"},
    {"NmzHasNauty", (PyCFunction)NmzHasNauty, METH_NOARGS,
     "Returns true if (Py)Normaliz was compiled with nauty support"},
    {"NmzHasFlint", (PyCFunction)NmzHasFlint, METH_NOARGS,
     "Returns true if (Py)Normaliz was compiled with Flint support"},
    {"NmzHasCocoa", (PyCFunction)NmzHasCocoa, METH_NOARGS,
     "Returns true if (Py)Normaliz was compiled with CoCoA support"},

    {"NmzWriteOutputFile", (PyCFunction)NmzWriteOutputFile, METH_VARARGS,
     "Prints the Normaliz cone output into a file"},
    {"NmzGetRenfInfo", (PyCFunction)NmzGetRenfInfo, METH_VARARGS,
     "Outputs info of the number field associated to a renf cone"},
    {"NmzModifyCone", (PyCFunction)_NmzModify_Outer, METH_VARARGS,
     "Modifies a given input property of a cone using a new matrix"},
    {
        NULL,
    } /* Sentinel */
};


#if PY_MAJOR_VERSION >= 3

static int PyNormaliz_cpp_traverse(PyObject* m, visitproc visit, void* arg)
{
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int PyNormaliz_cpp_clear(PyObject* m)
{
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}


static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,       "PyNormaliz_cpp",      NULL,
    sizeof(struct module_state), PyNormaliz_cppMethods, NULL,
    PyNormaliz_cpp_traverse,     PyNormaliz_cpp_clear,  NULL};

#define INITERROR return NULL

PyMODINIT_FUNC PyInit_PyNormaliz_cpp(void)

#else
#define INITERROR return

extern "C" void initPyNormaliz_cpp(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
    PyObject* module = PyModule_Create(&moduledef);
#else
    PyObject* module = Py_InitModule("PyNormaliz_cpp", PyNormaliz_cppMethods);
#endif

    if (module == NULL)
        INITERROR;
    struct module_state* st = GETSTATE(module);

    st->error = PyErr_NewException(
        const_cast< char* >("PyNormaliz_cpp.INITError"), NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

    NormalizError = PyErr_NewException(
        const_cast< char* >("PyNormaliz_cpp.NormalizError"), NULL, NULL);
    Py_INCREF(NormalizError);
    PyNormaliz_cppError = PyErr_NewException(
        const_cast< char* >("PyNormaliz_cpp.NormalizInterfaceError"), NULL,
        NULL);
    Py_INCREF(PyNormaliz_cppError);

    PyModule_AddObject(module, "normaliz_error", NormalizError);
    PyModule_AddObject(module, "pynormaliz_error", PyNormaliz_cppError);

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}
