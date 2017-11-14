#ifndef GAUDIPYTHON_HELPERS_H
#define GAUDIPYTHON_HELPERS_H

#include "Python.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"

// FIXME: (MCl) workaround for ROOT-5847
#include "GaudiKernel/PropertyFwd.h"

// FIXME: (MCl) workaround for ROOT-5850
namespace AIDA
{
  class IHistogram1D;
  class IHistogram2D;
  class IHistogram3D;
  class IProfile1D;
  class IProfile2D;
}

#if PY_VERSION_HEX < 0x02050000
// Note (MCl):
// In Python 2.5, all the functions working with lenghts use the type PySsize_t
// instead of int. The also changed the name of the typedefs for those functions.
// Here we use:
//  intargfunc -> ssizeargfunc
//  inquiry    -> lenfunc
//
/// For compatibility with Python 2.4 and 2.5
typedef int Py_ssize_t;
#endif

// the following is done instead of including Python.h becuase there is
// a clash with codecs.h defined in Python and the one in VC++ 7.1
// struct _object;
// typedef _object PyObject;
// extern "C" {
//  PyObject* PyBuffer_FromMemory( void *ptr, int size);
//}

/** Namespace for all classes interfacing Gaudi to Python.
 */

namespace GaudiPython
{

  struct Helper {
    // This is a number of static functions to overcome the current problem with PyLCGDict that
    // does not allow to return instances of complex objects by calling arguments
    // (reference to pointers)
    Helper() {}
    // Provided for backward compatibility
    static IService* service( ISvcLocator* svcloc, const std::string& name, bool createif = false )
    {
      return svcloc->service( name, createif ).get();
    }
    // Provided for backward compatibility
    static IAlgorithm* algorithm( IAlgManager* algmgr, const std::string& name, const bool createIf = false )
    {
      return algmgr->algorithm( name, createIf ).get();
    }
    // ==========================================================================
    static DataObject* dataobject( IDataProviderSvc* dpsvc, const std::string& path )
    {
      DataObject* o;
      return dpsvc->retrieveObject( path, o ).isSuccess() ? o : nullptr;
    }
    // ==========================================================================
    static StatusCode registerObject( IDataProviderSvc* dpsvc, const std::string& path, DataObject* pObject )
    {
      return dpsvc->registerObject( path, pObject );
    }
    // ==========================================================================
    static StatusCode unregisterObject( IDataProviderSvc* dpsvc, const std::string& path )
    {
      return dpsvc->unregisterObject( path );
    }
    // ==========================================================================
    /** simple wrapper for IDataProviderSvc::findObject
     *  The methdod does NOT trigger the loading the object from tape or Data-On-Demand action
     *  @see IDataProviderSvc
     *  @see IDataProviderSvc::findObject
     *  @param psvc (INPUT) pointer to Data ProviderService
     *  @param path the full path in TES
     *  @return the object
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-09
     */
    static GAUDI_API DataObject* findobject( IDataProviderSvc* dpsvc, const std::string& path );
    // ==========================================================================
    /** the generic function to get object from TES
     *  @see IDataProviderSvc
     *  @see IDataProviderSvc::findObject
     *  @see IDataProviderSvc::retriveObject
     *  @param psvc       (INPUT) pointer to Data ProviderService
     *  @param path the full path in TES
     *  @param rerieve    (INPUT) retrieve or find object ?
     *  @param disableDoD (INPUT) disable 'on-demand-actions' ?
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-09
     */
    static GAUDI_API DataObject* getobject( IDataProviderSvc* dpsvc, const std::string& path,
                                            const bool retrieve = true, const bool disableDoD = false );
    // ==========================================================================
    static IAlgTool* tool( IToolSvc* toolsvc, const std::string& type, const std::string& name, IInterface* p,
                           bool cif )
    {
      IAlgTool* o;
      return toolsvc->retrieve( type, name, IAlgTool::interfaceID(), o, p, cif ).isSuccess() ? o : nullptr;
    }
    static long loadDynamicLib( const std::string& name )
    {
      void* h;
      return System::loadDynamicLib( name, &h );
    }
    static IHistogram1D* histo1D( IHistogramSvc* hsvc, const std::string& path )
    {
      IHistogram1D* h;
      return ( hsvc->findObject( path, h ).isSuccess() ) ? h : nullptr;
    }
    static IHistogram2D* histo2D( IHistogramSvc* hsvc, const std::string& path )
    {
      IHistogram2D* h;
      return ( hsvc->findObject( path, h ).isSuccess() ) ? h : nullptr;
    }
    static IHistogram3D* histo3D( IHistogramSvc* hsvc, const std::string& path )
    {
      IHistogram3D* h;
      return ( hsvc->findObject( path, h ).isSuccess() ) ? h : nullptr;
    }
    static IProfile1D* profile1D( IHistogramSvc* hsvc, const std::string& path )
    {
      IProfile1D* h = 0;
      return ( hsvc && hsvc->findObject( path, h ).isSuccess() ) ? h : nullptr;
    }
    static IProfile2D* profile2D( IHistogramSvc* hsvc, const std::string& path )
    {
      IProfile2D* h = 0;
      return ( hsvc && hsvc->findObject( path, h ).isSuccess() ) ? h : nullptr;
    }

    // Array support
  private:
    template <class T>
    static Py_ssize_t Array_length( PyObject* self )
    {
#if PY_VERSION_HEX < 0x02050000
      const
#endif
          char* buf   = 0;
      Py_ssize_t size = ( *( self->ob_type->tp_as_buffer->bf_getcharbuffer ) )( self, 0, &buf );
      return size / sizeof( T );
    }

    template <class T>
    static PyObject* toPython( T* /*o*/ )
    {
      return 0;
    }
    static PyObject* toPython( int* o ) { return PyInt_FromLong( (long)*o ); }
    static PyObject* toPython( short* o ) { return PyInt_FromLong( (long)*o ); }
    static PyObject* toPython( char* o ) { return PyInt_FromLong( (long)*o ); }
    static PyObject* toPython( long* o ) { return PyInt_FromLong( *o ); }
    static PyObject* toPython( float* o ) { return PyFloat_FromDouble( (double)*o ); }
    static PyObject* toPython( double* o ) { return PyFloat_FromDouble( *o ); }

    template <class T>
    static PyObject* Array_item( PyObject* self, Py_ssize_t idx )
    {
#if PY_VERSION_HEX < 0x02050000
      const
#endif
          char* buf   = nullptr;
      Py_ssize_t size = ( *( self->ob_type->tp_as_buffer->bf_getcharbuffer ) )( self, 0, &buf );
      if ( idx < 0 || idx >= size / int( sizeof( T ) ) ) {
        PyErr_SetString( PyExc_IndexError, "buffer index out of range" );
        return nullptr;
      }
      return toPython( (T*)buf + idx );
    }

  public:
    template <class T>
    static PyObject* toArray( T* ptr, Py_ssize_t size )
    {
      static PyTypeObject type      = PyBuffer_Type;
      static PySequenceMethods meth = *( PyBuffer_Type.tp_as_sequence );
#if PY_VERSION_HEX < 0x02050000
      meth.sq_item   = (intargfunc)&Array_item<T>;
      meth.sq_length = (inquiry)&Array_length<T>;
#else
      meth.sq_item   = (ssizeargfunc)&Array_item<T>;
      meth.sq_length = (lenfunc)&Array_length<T>;
#endif
      type.tp_as_sequence = &meth;
      PyObject* buf       = PyBuffer_FromReadWriteMemory( ptr, size * sizeof( T ) );
      buf->ob_type        = &type;
      Py_INCREF( buf->ob_type );
      return buf;
    }
    static PyObject* toIntArray( void* ptr, Py_ssize_t size ) { return toArray( (int*)ptr, size ); }
    static PyObject* toShortArray( void* ptr, Py_ssize_t size ) { return toArray( (short*)ptr, size ); }
    static PyObject* toFloatArray( void* ptr, Py_ssize_t size ) { return toArray( (float*)ptr, size ); }
    static PyObject* toDoubleArray( void* ptr, Py_ssize_t size ) { return toArray( (double*)ptr, size ); }

    template <class T>
    static T* toAddress( std::vector<T>& v )
    {
      return v.data();
    }
    template <class T>
    static T* toAddress( void* a )
    {
      return (T*)a;
    }

    // FIXME: (MCl) workaround for ROOT-6028, ROOT-6054, ROOT-6073
    static StatusCode setPropertyFromString( Gaudi::Details::PropertyBase& p, const std::string& s )
    {
      return p.fromString( s );
    }
  };

  template PyObject* Helper::toArray( int*, Py_ssize_t );
  template PyObject* Helper::toArray( char*, Py_ssize_t );
  template PyObject* Helper::toArray( short*, Py_ssize_t );
  template PyObject* Helper::toArray( float*, Py_ssize_t );
  template PyObject* Helper::toArray( double*, Py_ssize_t );
  template int* Helper::toAddress( std::vector<int>& );
  template float* Helper::toAddress( std::vector<float>& );
  template double* Helper::toAddress( std::vector<double>& );
  template int* Helper::toAddress<int>( void* );
  template float* Helper::toAddress<float>( void* );
  template double* Helper::toAddress<double>( void* );

} // namespace GaudiPython

#endif // !GAUDIPYTHON_HELPERS_H
