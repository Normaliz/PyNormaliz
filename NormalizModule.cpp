#include <Python.h>
using namespace std;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <libnormaliz/cone.h>
#include <libnormaliz/map_operations.h>

using libnormaliz::Cone;
//using libnormaliz::ConeProperty;
using libnormaliz::ConeProperties;
using libnormaliz::Sublattice_Representation;
using libnormaliz::Type::InputType;

#include <vector>
using std::map;
using std::vector;
using std::pair;

// Macros for try catch
#define FUNC_BEGIN try {

#define FUNC_END \
    } catch (libnormaliz::NormalizException& e) { \
        PyErr_SetString( NormalizError, e.what() ); \
        return NULL; \
    } catch( ... ) { \
        PyErr_SetString( PyNormaliz_cppError, "unknown exception" ); \
        return NULL; \
    }

static PyObject * NormalizError;
static PyObject * PyNormaliz_cppError;
static const char* cone_name = "Cone";
static const char* cone_name_long = "Cone<long long>";
static string cone_name_str( cone_name );
static string cone_name_str_long( cone_name_long );

#if PY_MAJOR_VERSION >= 3
#define string_check PyUnicode_Check
#else
#define string_check PyString_Check
#endif

// Hacky 64-bit check. Works for windows and gcc, probably not clang.
// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__ || __aarch64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#ifndef NMZ_RELEASE
    static_assert(false,
       "Your Normaliz version (unknown) is to old! Update to 3.0.0 or newer.");
#endif
#if NMZ_RELEASE < 30000
    static_assert(false, "Your Normaliz version is to old! Update to 3.0.0 or newer.");
#endif


typedef int py_size_t;

string PyUnicodeToString( PyObject* in ){
#if PY_MAJOR_VERSION >= 3
  string out = "";
  int length = PyUnicode_GET_SIZE( in );
  for( int i = 0; i < length; i++ ){
      out += PyUnicode_READ_CHAR( in, i );
  }
  return out;
#else
  char* out = PyString_AsString( in );
  return string(out);
#endif
}

// Boolean conversion

inline PyObject* BoolToPyBool( bool in ){
  return in ? Py_True : Py_False;
}

// Converting MPZ's to PyLong and back via strings. Worst possible solution ever.

bool PyLongToNmz( PyObject * in, mpz_class& out ){
  PyObject * in_as_string = PyObject_Str( in );
  const char* in_as_c_string = PyUnicodeToString( in_as_string ).c_str();
  out.set_str( in_as_c_string, 10 );
  return true;
}

PyObject* NmzToPyLong( mpz_class in ){
  string mpz_as_string = in.get_str();
  char* mpz_as_c_string = const_cast<char*>(mpz_as_string.c_str());
  char * pend;
  PyObject* ret_val = PyLong_FromString( mpz_as_c_string, &pend, 10 );
  return ret_val;
}

PyObject* NmzToPyList( mpq_class in ){
  PyObject* out_list = PyList_New( 2 );
  PyList_SetItem( out_list, 0, NmzToPyLong( in.get_num() ) );
  PyList_SetItem( out_list, 1, NmzToPyLong( in.get_den() ) );
  return out_list;
}

bool PyLongToNmz( PyObject* in, long long & out ){
  
  int overflow;
  out = PyLong_AsLongLongAndOverflow( in, &overflow );
  if( overflow == -1 )
    return false;
  return true;
  
}

PyObject* NmzToPyLong( long long in ){
  
  return PyLong_FromLongLong( in );
  
}

PyObject* NmzToPyLong( libnormaliz::key_t in ){
  
  return PyLong_FromLong( in );
  
}

#ifdef ENVIRONMENT64
PyObject* NmzToPyLong( size_t in ){
  
  return PyLong_FromLong( in );
  
}
#endif

PyObject* NmzToPyLong( long in ){
  
  return PyLong_FromLong( in );
  
}

template<typename Integer>
bool PyLongToNmz(Integer& x, Integer &out){
  
  return Integer::unimplemented_function;
  
}

template<typename Integer>
PyObject* NmzToPyLong(Integer &in){
  
  return Integer::unimplemented_function;
  
}

template<typename Integer>
static bool PyListToNmz( vector<Integer>& out, PyObject* in ){
  if (!PyList_Check(in))
        return false;
    const int n = PyList_Size(in);
    out.resize(n);
    for (int i = 0; i < n; ++i) {
        PyObject* tmp = PyList_GetItem(in, i);
        if (!PyLongToNmz(tmp, out[i]))
            return false;
    }
    return true;
}

template<typename Integer>
static bool PyIntMatrixToNmz( vector<vector<Integer> >& out, PyObject* in ){
  if (!PyList_Check( in ) )
        return false;
    const int nr = PyList_Size( in );
    out.resize(nr);
    for (int i = 0; i < nr; ++i) {
        bool okay = PyListToNmz(out[i], PyList_GetItem(in, i));
        if (!okay)
            return false;
    }
    return true;
}

template<typename Integer>
PyObject* NmzVectorToPyList(const vector<Integer>& in)
{
    PyObject* vector;
    const size_t n = in.size();
    vector = PyList_New(n);
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(vector, i, NmzToPyLong(in[i]));
    }
    return vector;
}

PyObject* NmzBoolVectorToPyList(const vector<bool>& in)
{
    PyObject* vector;
    const size_t n = in.size();
    vector = PyList_New(n);
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(vector, i, BoolToPyBool(in[i]));
    }
    return vector;
}

PyObject* NmzBoolMatrixToPyList(const vector< vector<bool> >& in)
{
    PyObject* matrix;
    const size_t n = in.size();
    matrix = PyList_New( n );
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(matrix, i, NmzBoolVectorToPyList(in[i]));
    }
    return matrix;
}

template<typename Integer>
PyObject* NmzMatrixToPyList(const vector< vector<Integer> >& in)
{
    PyObject* matrix;
    const size_t n = in.size();
    matrix = PyList_New( n );
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(matrix, i, NmzVectorToPyList(in[i]));
    }
    return matrix;
}

PyObject* NmzHilbertSeriesToPyList(const libnormaliz::HilbertSeries& HS, bool is_HSOP)
{   
    PyObject* return_list = PyList_New( 3 );
    if(is_HSOP){
        PyList_SetItem(return_list, 0, NmzVectorToPyList(HS.getHSOPNum()));
        PyList_SetItem(return_list, 1, NmzVectorToPyList(libnormaliz::to_vector(HS.getHSOPDenom())));
        PyList_SetItem(return_list, 2, NmzToPyLong(HS.getShift()));
    }else{
        PyList_SetItem(return_list, 0, NmzVectorToPyList(HS.getNum()));
        PyList_SetItem(return_list, 1, NmzVectorToPyList(libnormaliz::to_vector(HS.getDenom())));
        PyList_SetItem(return_list, 2, NmzToPyLong(HS.getShift()));
    }
    return return_list;
}

template<typename Integer>
PyObject* NmzHilbertQuasiPolynomialToPyList(const libnormaliz::HilbertSeries& HS)
{
    vector< vector<Integer> > HQ = HS.getHilbertQuasiPolynomial();
    const size_t n = HS.getPeriod();
    PyObject* return_list = PyList_New(n+1);
    for (size_t i = 0; i < n; ++i) {
        PyList_SetItem(return_list, i, NmzVectorToPyList(HQ[i]));
    }
    PyList_SetItem(return_list, n, NmzToPyLong(HS.getHilbertQuasiPolynomialDenom()));
    return return_list;
}

template<typename Integer>
PyObject* NmzTriangleListToPyList(const vector< pair<vector<libnormaliz::key_t>, Integer> >& in)
{
    const size_t n = in.size();
    PyObject* M = PyList_New( n );
    for (size_t i = 0; i < n; ++i) {
        // convert the pair
        PyObject* pair = PyList_New(2);
        PyList_SetItem(pair, 0, NmzVectorToPyList<libnormaliz::key_t>(in[i].first));
        PyList_SetItem(pair, 1, NmzToPyLong(in[i].second));
        PyList_SetItem(M, i, pair);
    }
    return M;
}

template<typename Integer>
PyObject* NmzStanleyDataToPyList(const libnormaliz::STANLEYDATA<Integer>& StanleyData)
{
    PyObject* pair = PyList_New(2);
    PyList_SetItem(pair, 0, NmzVectorToPyList<libnormaliz::key_t>(StanleyData.key));
    PyList_SetItem(pair, 1, NmzMatrixToPyList(StanleyData.offsets.get_elements()));
    return pair;
}

template<typename Integer>
PyObject* NmzStanleyDecToPyList(const list<libnormaliz::STANLEYDATA<Integer> >& StanleyDec)
{
    const size_t n = StanleyDec.size();
    PyObject* M = PyList_New( n );
    typename list<libnormaliz::STANLEYDATA<Integer> >::const_iterator S = StanleyDec.begin();
    for (size_t i = 0; i < n; ++i) {        
        PyList_SetItem(M, i,NmzStanleyDataToPyList(*S) );
        ++S;
    }
    return M;
}


void delete_cone_mpz( PyObject* cone ){
  Cone<mpz_class> * cone_ptr = reinterpret_cast<Cone<mpz_class>* >( PyCapsule_GetPointer( cone, cone_name ) );
  delete cone_ptr;
}

void delete_cone_long( PyObject* cone ){
  Cone<long long> * cone_ptr = reinterpret_cast<Cone<long long>* >( PyCapsule_GetPointer( cone, cone_name_long ) );
  delete cone_ptr;
}

Cone<long long>* get_cone_long( PyObject* cone ){
  return reinterpret_cast<Cone<long long>*>( PyCapsule_GetPointer( cone, cone_name_long ) );
}

Cone<mpz_class>* get_cone_mpz( PyObject* cone ){
  return reinterpret_cast<Cone<mpz_class>*>( PyCapsule_GetPointer( cone, cone_name ) );
}

// template<typename Integer>
// Cone<Integer>* get_cone( PyObject* cone ){
//   return reinterpret_cast<Cone<Integer>*>( PyCapsule_GetPointer( cone, cone_name ) );
// }

PyObject* pack_cone( Cone<mpz_class>* C ){
  return PyCapsule_New( reinterpret_cast<void*>( C ), cone_name, &delete_cone_mpz );
}

PyObject* pack_cone( Cone<long long>* C ){
  return PyCapsule_New( reinterpret_cast<void*>( C ), cone_name_long, &delete_cone_long );
}

// template<typename Integer>
// PyObject* pack_cone( Cone<Integer>* C ){
//   return PyCapsule_New( reinterpret_cast<void*>( C ), cone_name, &delete_cone<Integer> );
// }

bool is_cone( PyObject* cone ){
  if( PyCapsule_CheckExact( cone ) ){
    // compare as string
    return cone_name_str == string(PyCapsule_GetName( cone )) || cone_name_str_long == string(PyCapsule_GetName( cone ));
  }
  return false;
}

template<typename Integer>
static PyObject* _NmzConeIntern(PyObject * args)
{
    map <InputType, vector< vector<Integer> > > input;
    
    PyObject* input_list;
    
    if( PyTuple_Size(args)==1 ){
        input_list = PyTuple_GetItem( args, 0 );
        if( ! PyList_Check( input_list ) ){
            PyErr_SetString( PyNormaliz_cppError, "Single argument must be a list" );
            return NULL;
        }
        input_list = PyList_AsTuple( input_list );
    }else{
        input_list = args;
    }
    
    const int n = PyTuple_Size(input_list);
    if (n&1) {
        PyErr_SetString( PyNormaliz_cppError, "Number of arguments must be even" );
        return NULL;
    }
    for (int i = 0; i < n; i += 2) {
        PyObject* type = PyTuple_GetItem(input_list, i);
        if (!string_check(type)) {
            PyErr_SetString( PyNormaliz_cppError, "Odd entries must be strings" );
            return NULL;
        }
        
        string type_str = PyUnicodeToString( type );
        
        PyObject* M = PyTuple_GetItem(input_list, i+1);
        vector<vector<Integer> > Mat;
        bool okay = PyIntMatrixToNmz(Mat, M);
        if (!okay) {
            PyErr_SetString( PyNormaliz_cppError, "Even entries must be matrices" );
            return NULL;
        }

        input[libnormaliz::to_type(type_str)] = Mat;
    }

    Cone<Integer>* C = new Cone<Integer>(input);
    
    PyObject* return_container = pack_cone( C );
    
    return return_container;
}

PyObject* _NmzCone(PyObject* self, PyObject* args, PyObject* keywds)
{
    FUNC_BEGIN
    
    static const char* string_for_keyword_argument = "CreateAsLongLong";
    PyObject* create_as_long_long;
    
#if PY_MAJOR_VERSION >= 3
    PyObject* key = PyUnicode_FromString( string_for_keyword_argument );
#else
    PyObject* key = PyString_FromString( const_cast<char*>(string_for_keyword_argument) );
#endif
    
    if( keywds != NULL && PyDict_Contains( keywds, key ) == 1 ){
        create_as_long_long = PyDict_GetItem( keywds, key );
    }else{
        create_as_long_long = Py_False;
    }
    
    if( create_as_long_long!=Py_True ){
        return _NmzConeIntern<mpz_class>(args);
    }else{
        return _NmzConeIntern<long long>(args);
    }

    FUNC_END
}

template<typename Integer>
PyObject* _NmzCompute(Cone<Integer>* C, PyObject* args)
{
    FUNC_BEGIN
    
    const int arg_len = PyTuple_Size(args);
    
    PyObject* to_compute;
    
    if(arg_len==2){
        PyObject* first_arg = PyTuple_GetItem(args,1);
        if(PyList_CheckExact( first_arg )){
            to_compute = first_arg;
        }else{
            to_compute = PyList_New( 1 );
            int result = PyList_SetItem( to_compute, 0, first_arg );
            if(result!=0){
                PyErr_SetString( PyNormaliz_cppError, "List could not be created" );
                return NULL;
            }
        }
    }else{
        to_compute = PyList_New( arg_len - 1 );
        for( int i = 1;i<arg_len;i++){
            PyList_SetItem( to_compute, i, PyTuple_GetItem( args, i ) );
        }
    }

    ConeProperties propsToCompute;
    const int n = PyList_Size(to_compute);
    
    for (int i = 0; i < n; ++i) {
        PyObject* prop = PyList_GetItem(to_compute, i);
        if (!string_check(prop)) {
            PyErr_SetString( PyNormaliz_cppError, "All elements must be strings" );
            return NULL;
        }
        string prop_str(PyUnicodeToString(prop));
        propsToCompute.set( libnormaliz::toConeProperty(prop_str) );
    }
    
    ConeProperties notComputed = C->compute(propsToCompute);
    
    // Cone.compute returns the not computed properties
    // we return a bool, true when everything requested was computed
    return notComputed.none() ? Py_True : Py_False;
    FUNC_END
}


PyObject* _NmzCompute_Outer(PyObject* self, PyObject* args){
  
  FUNC_BEGIN
  
  PyObject* cone = PyTuple_GetItem( args, 0 );
  
  if( !is_cone(cone) ){
      PyErr_SetString( PyNormaliz_cppError, "First argument must be a cone" );
      return NULL;
  }
  
  if( cone_name_str == string(PyCapsule_GetName(cone)) ){
      Cone<mpz_class>* cone_ptr = get_cone_mpz(cone);
      return _NmzCompute(cone_ptr, args);
  }else{
      Cone<long long>* cone_ptr = get_cone_long(cone);
      return _NmzCompute(cone_ptr,args);
  }
  
  FUNC_END
  
}

template<typename Integer>
PyObject* NmzIsComputed(Cone<Integer>* C, PyObject* prop)
{
    FUNC_BEGIN
    
    libnormaliz::ConeProperty::Enum p = libnormaliz::toConeProperty(PyUnicodeToString( prop ) );
  
    return C->isComputed(p) ? Py_True : Py_False;

    FUNC_END
}

PyObject* NmzIsComputed_Outer(PyObject* self, PyObject* args)
{
    FUNC_BEGIN
    
    PyObject* cone = PyTuple_GetItem( args, 0 );
    PyObject* to_compute = PyTuple_GetItem( args, 1 );
    
    if( !is_cone(cone) ){
        PyErr_SetString( PyNormaliz_cppError, "First argument must be a cone" );
        return NULL;
    }
    
    if( cone_name_str == string(PyCapsule_GetName(cone)) ){
        Cone<mpz_class>* cone_ptr = get_cone_mpz(cone);
        return NmzIsComputed(cone_ptr, to_compute);
    }else{
        Cone<long long>* cone_ptr = get_cone_long(cone);
        return NmzIsComputed(cone_ptr,to_compute);
    }
    
    FUNC_END
}

template<typename Integer>
static PyObject* _NmzBasisChangeIntern(Cone<Integer>* C)
{
    Sublattice_Representation<Integer> bc = C->getSublattice();

    PyObject* res = PyList_New( 3 );
    PyList_SetItem(res, 0, NmzMatrixToPyList(bc.getEmbedding()));
    PyList_SetItem(res, 1, NmzMatrixToPyList(bc.getProjection()));
    PyList_SetItem(res, 2, NmzToPyLong(bc.getAnnihilator()));
    // Dim, Rank, Equations and Congruences are already covered by special functions
    // ditto ExternalIndex
    return res;
}

template<typename Integer>
PyObject* _NmzResultImpl(Cone<Integer>* C, PyObject* prop_obj)
{
    
    string prop = PyUnicodeToString( prop_obj );

    libnormaliz::ConeProperty::Enum p = libnormaliz::toConeProperty(prop);

    ConeProperties notComputed = C->compute(ConeProperties(p));
    if (notComputed.any()) {
        return Py_None;
    }

    switch (p) {
    case libnormaliz::ConeProperty::Generators:
        return NmzMatrixToPyList(C->getGenerators());

    case libnormaliz::ConeProperty::ExtremeRays:
        return NmzMatrixToPyList(C->getExtremeRays());

    case libnormaliz::ConeProperty::VerticesOfPolyhedron:
        return NmzMatrixToPyList(C->getVerticesOfPolyhedron());

    case libnormaliz::ConeProperty::SupportHyperplanes:
        return NmzMatrixToPyList(C->getSupportHyperplanes());

    case libnormaliz::ConeProperty::TriangulationSize:
        return NmzToPyLong(C->getTriangulationSize());

    case libnormaliz::ConeProperty::TriangulationDetSum:
        return NmzToPyLong(C->getTriangulationDetSum());

    case libnormaliz::ConeProperty::Triangulation:
        return NmzTriangleListToPyList<Integer>(C->getTriangulation());

    case libnormaliz::ConeProperty::Multiplicity:
        return NmzToPyList(C->getMultiplicity());

    case libnormaliz::ConeProperty::RecessionRank:
        return NmzToPyLong(C->getRecessionRank());

    case libnormaliz::ConeProperty::AffineDim:
        return NmzToPyLong(C->getAffineDim());

    case libnormaliz::ConeProperty::ModuleRank:
        return NmzToPyLong(C->getModuleRank());

    case libnormaliz::ConeProperty::HilbertBasis:
        return NmzMatrixToPyList(C->getHilbertBasis());

    case libnormaliz::ConeProperty::MaximalSubspace:
        return NmzMatrixToPyList(C->getMaximalSubspace());

    case libnormaliz::ConeProperty::ModuleGenerators:
        return NmzMatrixToPyList(C->getModuleGenerators());

    case libnormaliz::ConeProperty::Deg1Elements:
        return NmzMatrixToPyList(C->getDeg1Elements());

    case libnormaliz::ConeProperty::HilbertSeries:
        {
        bool is_HSOP = C->isComputed(libnormaliz::ConeProperty::HSOP);
        return NmzHilbertSeriesToPyList(C->getHilbertSeries(),is_HSOP);
        }

    case libnormaliz::ConeProperty::Grading:
        {
        vector<Integer> grad = C->getGrading();
        grad.push_back(C->getGradingDenom());
        return NmzVectorToPyList(grad);
        }

    case libnormaliz::ConeProperty::IsPointed:
        return BoolToPyBool(C->isPointed());

    case libnormaliz::ConeProperty::IsDeg1ExtremeRays:
        return BoolToPyBool(C->isDeg1ExtremeRays());

    case libnormaliz::ConeProperty::IsDeg1HilbertBasis:
        return BoolToPyBool(C->isDeg1HilbertBasis());

    case libnormaliz::ConeProperty::IsIntegrallyClosed:
        return BoolToPyBool(C->isIntegrallyClosed());

    case libnormaliz::ConeProperty::OriginalMonoidGenerators:
        return NmzMatrixToPyList(C->getOriginalMonoidGenerators());

    case libnormaliz::ConeProperty::IsReesPrimary:
        return BoolToPyBool(C->isReesPrimary());

    case libnormaliz::ConeProperty::ReesPrimaryMultiplicity:
        return NmzToPyLong(C->getReesPrimaryMultiplicity());

    case libnormaliz::ConeProperty::StanleyDec:
        return NmzStanleyDecToPyList(C->getStanleyDec());

    case libnormaliz::ConeProperty::ExcludedFaces:
        return NmzMatrixToPyList(C->getExcludedFaces());

    case libnormaliz::ConeProperty::Dehomogenization:
        return NmzVectorToPyList(C->getDehomogenization());

    case libnormaliz::ConeProperty::InclusionExclusionData:
        return NmzTriangleListToPyList<long>(C->getInclusionExclusionData());

    case libnormaliz::ConeProperty::ClassGroup:
        return NmzVectorToPyList(C->getClassGroup());
    
    case libnormaliz::ConeProperty::IsInhomogeneous:
        return BoolToPyBool(C->isInhomogeneous());
    
    /* Sublattice properties */
    
    case libnormaliz::ConeProperty::Equations:
        return NmzMatrixToPyList(C->getSublattice().getEquations());
    
    case libnormaliz::ConeProperty::Congruences:
        return NmzMatrixToPyList(C->getSublattice().getCongruences());
    
    case libnormaliz::ConeProperty::EmbeddingDim:
        return NmzToPyLong(C->getEmbeddingDim());
    
    case libnormaliz::ConeProperty::Rank:
        return NmzToPyLong(C->getRank());
    
    case libnormaliz::ConeProperty::Sublattice:
        return _NmzBasisChangeIntern(C);
    
    case libnormaliz::ConeProperty::ExternalIndex:
        return NmzToPyLong(C->getSublattice().getExternalIndex());
    
    case libnormaliz::ConeProperty::InternalIndex:
        return NmzToPyLong(C->getIndex());
    
    case libnormaliz::ConeProperty::WitnessNotIntegrallyClosed:
        return NmzVectorToPyList(C->getWitnessNotIntegrallyClosed());
    
    
    /* New stuff */
    
    case libnormaliz::ConeProperty::GradingDenom:
        return NmzToPyLong(C->getGradingDenom());
    
    case libnormaliz::ConeProperty::UnitGroupIndex:
        return NmzToPyLong(C->getUnitGroupIndex());
    
    case libnormaliz::ConeProperty::ModuleGeneratorsOverOriginalMonoid:
        return NmzMatrixToPyList(C->getModuleGeneratorsOverOriginalMonoid());
    
    case libnormaliz::ConeProperty::IntegerHull:
    {   
        Cone<Integer>* hull = new Cone<Integer>( C->getIntegerHullCone() );
        return pack_cone( hull ); 
    }
    
    case libnormaliz::ConeProperty::HilbertQuasiPolynomial:
        return NmzHilbertQuasiPolynomialToPyList<mpz_class>(C->getHilbertSeries()); //FIXME: Why is this return value not parametrized, but mpz_class only?
        
    case libnormaliz::ConeProperty::IsTriangulationNested:
        return BoolToPyBool(C->isTriangulationNested());
        
    case libnormaliz::ConeProperty::IsTriangulationPartial:
        return BoolToPyBool(C->isTriangulationPartial());
        
    case libnormaliz::ConeProperty::ConeDecomposition:
        return NmzBoolMatrixToPyList(C->getOpenFacets());

//  the following properties are compute options and do not return anything
    case libnormaliz::ConeProperty::DualMode:
    case libnormaliz::ConeProperty::DefaultMode:
    case libnormaliz::ConeProperty::Approximate:
    case libnormaliz::ConeProperty::BottomDecomposition:
    case libnormaliz::ConeProperty::KeepOrder:
    case libnormaliz::ConeProperty::NoBottomDec:
    case libnormaliz::ConeProperty::PrimalMode:
    case libnormaliz::ConeProperty::Symmetrize:
    case libnormaliz::ConeProperty::NoSymmetrization:
    case libnormaliz::ConeProperty::BigInt:
    case libnormaliz::ConeProperty::NoNestedTri:
    case libnormaliz::ConeProperty::HSOP:
        PyErr_SetString( PyNormaliz_cppError, "ConeProperty is input-only" );
        return NULL;
#if NMZ_RELEASE >= 30200
    case libnormaliz::ConeProperty::NoSubdivision:
        PyErr_SetString( PyNormaliz_cppError, "ConeProperty is input-only" );
        return NULL;
#endif
    default:
        PyErr_SetString( PyNormaliz_cppError, "Unknown cone property" );
        return NULL;
        break;
    }

    return Py_None;
}

PyObject* _NmzResult( PyObject* self, PyObject* args ){
  
  FUNC_BEGIN
  PyObject* cone = PyTuple_GetItem( args, 0 );
  PyObject* prop = PyTuple_GetItem( args, 1 );
  
  if( !is_cone( cone ) ){
    PyErr_SetString( PyNormaliz_cppError, "First argument must be a cone" );
    return NULL;
  }
  
  if( !string_check( prop ) ){
    PyErr_SetString( PyNormaliz_cppError, "Second argument must be a unicode string" );
    return NULL;
  }
  
  if( cone_name_str == string(PyCapsule_GetName(cone)) ){
    Cone<mpz_class>* cone_ptr = get_cone_mpz(cone);
    return _NmzResultImpl(cone_ptr, prop);
  }else{
    Cone<long long>* cone_ptr = get_cone_long(cone);
    return _NmzResultImpl(cone_ptr, prop);
  }
  
  FUNC_END
}

PyObject* NmzSetVerboseDefault( PyObject* self, PyObject* args)
{
    FUNC_BEGIN
    PyObject * value = PyTuple_GetItem( args, 0 );
    if (value != Py_True && value != Py_False){
        PyErr_SetString( PyNormaliz_cppError, "Argument must be True or False" );
        return NULL;
    }
    return BoolToPyBool(libnormaliz::setVerboseDefault(value == Py_True));
    FUNC_END
}

template<typename Integer>
PyObject* NmzSetVerbose(Cone<Integer>* C, PyObject* value)
{
    FUNC_BEGIN
    bool old_value;
    old_value = C->setVerbose(value == Py_True);
    return BoolToPyBool(old_value);
    FUNC_END
}

PyObject* NmzSetVerbose_Outer(PyObject* self, PyObject* args)
{
    FUNC_BEGIN
    
    PyObject* cone = PyTuple_GetItem( args, 0 );
    
    if( !is_cone( cone ) ){
        PyErr_SetString( PyNormaliz_cppError, "First argument must be a cone" );
        return NULL;
    }
    
    PyObject* value = PyTuple_GetItem( args, 1 );
    if (value != Py_True && value != Py_False){
        PyErr_SetString( PyNormaliz_cppError, "Second argument must be True or False" );
        return NULL;
    }
    
    if( cone_name_str == string(PyCapsule_GetName(cone)) ){
        Cone<mpz_class>* cone_ptr = get_cone_mpz(cone);
        return NmzSetVerbose(cone_ptr, value);
    }else{
        Cone<long long>* cone_ptr = get_cone_long(cone);
        return NmzSetVerbose(cone_ptr, value);
    }
    
    FUNC_END
    
}
/*
 * Python mixed init stuff
 */

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

static PyObject * error_out(PyObject *m) {
    struct module_state *st = GETSTATE(m);
    PyErr_SetString(st->error, "something bad happened");
    return NULL;
}

static PyMethodDef PyNormaliz_cppMethods[] = {
    {"error_out", (PyCFunction)error_out, METH_NOARGS, NULL},
    {"NmzCone",  (PyCFunction)_NmzCone, METH_VARARGS|METH_KEYWORDS,
     "Create a cone"},
    {"NmzCompute", (PyCFunction)_NmzCompute_Outer, METH_VARARGS,
     "Compute some stuff"},
    {"NmzIsComputed", (PyCFunction)NmzIsComputed_Outer, METH_VARARGS,
     "Check if property is computed "},
    {"NmzResult", (PyCFunction)_NmzResult, METH_VARARGS,
      "Return cone property" },
    { "NmzSetVerboseDefault", (PyCFunction)NmzSetVerboseDefault, METH_VARARGS,
      "Set verbosity" },
    { "NmzSetVerbose", (PyCFunction)NmzSetVerbose_Outer, METH_VARARGS,
      "Set verbosity of cone" },
    {NULL, }        /* Sentinel */
};


#if PY_MAJOR_VERSION >= 3

static int PyNormaliz_cpp_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int PyNormaliz_cpp_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}


static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "PyNormaliz_cpp",
        NULL,
        sizeof(struct module_state),
        PyNormaliz_cppMethods,
        NULL,
        PyNormaliz_cpp_traverse,
        PyNormaliz_cpp_clear,
        NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC PyInit_PyNormaliz_cpp(void)

#else
#define INITERROR return

extern "C" void initPyNormaliz_cpp(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
    PyObject *module = PyModule_Create(&moduledef);
#else
    PyObject *module = Py_InitModule("PyNormaliz_cpp", PyNormaliz_cppMethods);
#endif

    if (module == NULL)
        INITERROR;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException(const_cast<char*>("PyNormaliz_cpp.INITError"), NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }
    
    NormalizError = PyErr_NewException(const_cast<char*>("Normaliz.error"), NULL, NULL );
    Py_INCREF( NormalizError );
    PyNormaliz_cppError = PyErr_NewException(const_cast<char*>("Normaliz.interface_error"), NULL, NULL );
    Py_INCREF( PyNormaliz_cppError );
    
    PyModule_AddObject( module, "error", NormalizError );
    PyModule_AddObject( module, "error", PyNormaliz_cppError );

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}

