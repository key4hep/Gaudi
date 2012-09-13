/**
 * @file GaudiMP/src/Lib/PyROOTPickle.cpp
 * @author Wim Lavrijsen
 * @date Apr 2008
 * @brief Port pickling functionality while awaiting newer release.
 */

#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT
#pragma warning(disable:2259)
#endif

#include "GaudiMP/PyROOTPickle.h"
#include "TClass.h"
#include "TClassRef.h"
#include "TBufferFile.h"
#include "TPython.h"
#include "RVersion.h"

//- data _______________________________________________________________________
#if ROOT_VERSION_CODE < ROOT_VERSION(5,19,0)
static PyObject* gExpand = 0;
#endif

namespace GaudiMP {

#if ROOT_VERSION_CODE < ROOT_VERSION(5,19,0)

/**
 * @brief PyROOT object proxy pickle support
 * @param self object proxy instance to be pickled
 */
PyObject* ObjectProxyReduce( PyObject* self )
{
  // Turn the object proxy instance into a character stream and return for
  // pickle, together with the callable object that can restore the stream
  // into the object proxy instance.

  void* vself = TPython::ObjectProxy_AsVoidPtr( self );    // checks type
  if ( ! vself ) {
     PyErr_SetString( PyExc_TypeError,
       "__reduce__ requires an object proxy instance as first argument" );
     return 0;
  }

  PyObject* nattr = PyObject_GetAttrString( (PyObject*)self->ob_type, (char*)"__name__" );
  PyObject* pyname = PyObject_Str( nattr );
  Py_DECREF( nattr );

  static TClass* bufferclass =  TClass::GetClass("TBufferFile");
  TClass* klass = TClass::GetClass( PyString_AS_STRING( pyname ) );

  // no cast is needed, but WriteObject taking a TClass argument is protected,
  // so use WriteObjectAny()
  TBufferFile* buf = 0;
  if ( klass == bufferclass ) {
     buf = (TBufferFile*)vself;
  }
  else {
     static TBufferFile buffer( TBuffer::kWrite );
     buffer.Reset();
     if ( buffer.WriteObjectAny( vself, klass ) != 1 ) {
        PyErr_Format( PyExc_IOError,
           "could not stream object of type %s", PyString_AS_STRING( pyname ) );
        Py_DECREF( pyname );
        return 0;
     }
     buf = &buffer;
  }

  // use a string for the serialized result, as a python buffer will not copy
  // the buffer contents; use a string for the class name, used when casting
  // on reading back in
  PyObject* res2 = PyTuple_New( 2 );
  PyTuple_SET_ITEM( res2, 0, PyString_FromStringAndSize( buf->Buffer(), buf->Length() ) );
  PyTuple_SET_ITEM( res2, 1, pyname );

  PyObject* result = PyTuple_New( 2 );
  Py_INCREF( gExpand );
  PyTuple_SET_ITEM( result, 0, gExpand );
  PyTuple_SET_ITEM( result, 1, res2 );

  return result;
}


class ObjectProxy {
   public:
      enum EFlags { kNone = 0x0, kIsOwner = 0x0001, kIsReference = 0x0002 };

   public:
      void HoldOn() { fFlags |= kIsOwner; }
      void Release() { fFlags &= ~kIsOwner; }
   public:               // public, as the python C-API works with C structs
      PyObject_HEAD
      void*     fObject;
      TClassRef fClass;
      int       fFlags;
   private:              // private, as the python C-API will handle creation
      ObjectProxy() {}
};


/**
 * @brief Helper for (un)pickling of ObjectProxy's
 * @param args The Python arguments.
 */
PyObject* ObjectProxyExpand( PyObject*, PyObject* args )
{
  // This method is a helper for (un)pickling of ObjectProxy instances.
  PyObject* pybuf = 0;
  const char* clname = 0;
  if ( ! PyArg_ParseTuple( args, const_cast< char* >( "O!s:__expand__" ),
           &PyString_Type, &pybuf, &clname ) )
    return 0;

  // use the PyString macro's to by-pass error checking; do not adopt the buffer,
  // as the local TBufferFile can go out of scope (there is no copying)
  void* result;
  if( strcmp(clname, "TBufferFile") == 0) {
    TBufferFile* buf = new TBufferFile( TBuffer::kWrite);
    buf->WriteFastArray( PyString_AS_STRING(pybuf), PyString_GET_SIZE( pybuf ));
    result = buf;
  }
  else {
    TBufferFile buf( TBuffer::kRead,
       PyString_GET_SIZE( pybuf ), PyString_AS_STRING( pybuf ), kFALSE );
    result = buf.ReadObjectAny( 0 );
  }
  PyObject* pobj =  TPython::ObjectProxy_FromVoidPtr( result, clname );
  // set Ownership of the returned object
  ObjectProxy* obj = (ObjectProxy*)pobj;
  obj->HoldOn();
  return pobj;
}


/**
 * @brief Install the pickling of ObjectProxy's functionality.
 * @param libpyroot_pymodule The libPyROOT python module
 * @param objectproxy_pytype The ObjectProxy python type
 */
void PyROOTPickle::Initialize( PyObject* libpyroot_pymodule, PyObject* objectproxy_pytype )
{
  Py_INCREF( libpyroot_pymodule );
  PyTypeObject* pytype = (PyTypeObject*)objectproxy_pytype;

  static PyMethodDef s_pdefExp = { (char*)"_ObjectProxy__expand__",
            (PyCFunction)ObjectProxyExpand, METH_VARARGS, (char*)"internal function" };

  PyObject* pymname = PyString_FromString( PyModule_GetName( libpyroot_pymodule ) );
  gExpand = PyCFunction_NewEx( &s_pdefExp, NULL, pymname );
  Py_DECREF( pymname );
  Bool_t isOk = PyObject_SetAttrString( libpyroot_pymodule, s_pdefExp.ml_name, gExpand ) == 0;
  Py_DECREF( gExpand );      // is moderately risky, but Weakref not allowed (?)

  if ( ! isOk ) {
    Py_DECREF( libpyroot_pymodule );
    PyErr_SetString( PyExc_TypeError, "could not add expand function to libPyROOT" );
    return;
  }

  static PyMethodDef s_pdefRed = { (char*)"__reduce__",
            (PyCFunction)ObjectProxyReduce, METH_NOARGS, (char*)"internal function" };

  PyObject* descr = PyDescr_NewMethod( pytype, &s_pdefRed );
  isOk = PyDict_SetItemString( pytype->tp_dict, s_pdefRed.ml_name, descr) == 0;
  Py_DECREF( descr );
  if ( ! isOk ) {
    Py_DECREF( libpyroot_pymodule );
    PyErr_SetString( PyExc_TypeError, "could not add __reduce__ function to ObjectProxy" );
    return;
  }

  Py_DECREF( libpyroot_pymodule );
}

#else //  ROOT_VERSION_CODE < ROOT_VERSION(5,19,0)

void PyROOTPickle::Initialize( PyObject*, PyObject* )
{
  /* dummy. It is not needed for this version of ROOT */
}

#endif //  ROOT_VERSION_CODE < ROOT_VERSION(5,19,0)

} // namespace GaudiMP
