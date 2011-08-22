///////////////////////// -*- C++ -*- /////////////////////////////
// IoComponentMgr.cxx
// Implementation file for class IoComponentMgr
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////

// python includes
#include <Python.h>

// GaudiMP includes
#include "IoComponentMgr.h"


// STL includes

// FrameWork includes
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/Property.h"

DECLARE_SERVICE_FACTORY(IoComponentMgr)


///////////////////////////////////////////////////////////////////
// Public methods:
///////////////////////////////////////////////////////////////////

// Constructors
////////////////
IoComponentMgr::IoComponentMgr( const std::string& name,
				ISvcLocator* svc )
: base_class(name,svc), m_log(msgSvc(), name ),
    m_dict   ( 0 )
{
  //
  // Property declaration
  //
  //declareProperty( "Property", m_nProperty );

//   declareProperty ("Registry",
// 		   m_dict_location = "GaudiMP.IoRegistry.registry",
// 		   "Location of the python dictionary holding the "
// 		   "associations: \n"
// 		   " {component-name:{ 'old-fname' : ['io','new-fname'] }}\n"
// 		   "\nSyntax: <python-module>.<python-module>.<fct-name> \n"
// 		   " where fct-name is a function returning the wanted "
// 		   " dictionary.");
}

// Destructor
///////////////
IoComponentMgr::~IoComponentMgr()
{
  Py_XDECREF (m_dict);
}

// Service's Hooks
////////////////////////////
StatusCode IoComponentMgr::initialize()
{
  m_log << MSG::INFO << "Initializing " << name() << "..." << endmsg;

  if ( Service::initialize().isFailure() ) {
    m_log << MSG::ERROR << "Unable to initialize Service base class" << endmsg;
    return StatusCode::FAILURE;
  }
  m_log.setLevel( m_outputLevel.value() );

  if ( ! Py_IsInitialized() ) {
    if (m_log.level() <= MSG::DEBUG) {
      m_log << MSG::DEBUG << "Initializing Python" << endmsg;
    }
    PyEval_InitThreads();
    Py_Initialize();

    if ( ! Py_IsInitialized() ) {
      m_log << MSG::ERROR << "Unable to initialize Python" << endmsg;
      return StatusCode::FAILURE;
    }
  }


  // retrieve the python dictionary holding the I/O registry
  const std::string py_module_name = "GaudiMP.IoRegistry";
  m_log << MSG::DEBUG << "importing module [" << py_module_name << "]..."
	<< endmsg;
  PyObject *module = PyImport_ImportModule ((char*)py_module_name.c_str());
  if ( !module || !PyModule_Check (module) ) {
    m_log << MSG::ERROR << "Could not import [" << py_module_name << "] !"
	  << endmsg;
    Py_XDECREF (module);
    return StatusCode::FAILURE;
  }

  const std::string py_class_name = "IoRegistry";
  PyObject *pyclass = PyDict_GetItemString (PyModule_GetDict(module),
					    (char*)py_class_name.c_str());
  // borrowed ref.
  Py_XINCREF (pyclass);
  if ( !pyclass ) {
    m_log << MSG::ERROR << "Could not import ["
	  << py_class_name << "] from module ["
	  << py_module_name << "] !"
	  << endmsg ;
    Py_XDECREF (pyclass);
    Py_DECREF  (module);
    return StatusCode::FAILURE;
  }

  m_dict = PyObject_GetAttrString (pyclass, (char*)"instances");
  if ( !m_dict || !PyDict_Check (m_dict) ) {
    m_log << MSG::ERROR
	  << "could not retrieve attribute [instances] from class ["
	  << py_module_name << "." << py_class_name << "] !" << endmsg;
    Py_DECREF (pyclass);
    Py_DECREF (module);
    return StatusCode::FAILURE;
  }

  m_log << MSG::INFO <<  "python I/O registry retrieved [ok]" << endmsg;
  if ( m_log.level() <=  MSG::DEBUG ) {
    std::string repr = "";
    // PyObject_Repr returns a new ref.
    PyObject* py_repr = PyObject_Repr (m_dict);
    if ( py_repr && PyString_Check(py_repr) ) {
      repr = PyString_AsString(py_repr);
    }
    Py_XDECREF( py_repr );
    m_log << MSG::DEBUG << "content: " << repr << endmsg;
  }

  return StatusCode::SUCCESS;
}

StatusCode IoComponentMgr::finalize()
{
  m_log << MSG::INFO << "Finalizing " << name() << "..." << endmsg;


  if (m_log.level() <= MSG::DEBUG) {
    m_log << MSG::DEBUG << "Listing all monitored entries: " << std::endl;

    std::string repr = "";
    // PyObject_Repr returns a new ref.
    PyObject* py_repr = PyObject_Repr (m_dict);
    if ( py_repr && PyString_Check(py_repr) ) {
      repr = PyString_AsString(py_repr);
    }
    Py_XDECREF( py_repr );
    m_log << MSG::DEBUG << "content: " << repr << endmsg;
  }

  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Const methods:
///////////////////////////////////////////////////////////////////

  /** @brief: check if the registry contains a given @c IIoComponent
   */
bool
IoComponentMgr::io_hasitem (IIoComponent* iocomponent) const
{
  if ( 0 == iocomponent ) {
    return false;
  }
  const std::string& ioname = iocomponent->name();
  IoRegistry_t::const_iterator io = m_ioregistry.find (ioname);
  return io != m_ioregistry.end();
}

/** @brief: check if the registry contains a given @c IIoComponent and
 *          that component had @param `fname` as a filename
 */
bool
IoComponentMgr::io_contains (IIoComponent* iocomponent,
			     const std::string& fname) const
{
  if ( 0 == iocomponent ) {
    return false;
  }
  const std::string& ioname = iocomponent->name();

  // m_dict is a python dictionary like so:
  //  { 'iocomp-name' : { 'oldfname' : [ 'iomode', 'newfname' ] } }

  // -> check there is an 'iocomp-name' entry
  // -> retrieve that entry
  PyObject *o = PyDict_GetItemString (m_dict, (char*)ioname.c_str());
  Py_XINCREF (o);

  // -> check it is a dictionary
  if ( NULL==o || !PyDict_Check (o) ) {
    Py_XDECREF (o);

    return false;
  }

  // -> check 'oldfname' exists
  PyObject *item = PyDict_GetItemString (o, (char*)fname.c_str());

  const bool contains = (item != 0);
  if ( contains == false ) {
    std::string repr = "";
    // PyObject_Repr returns a new ref.
    PyObject* py_repr = PyObject_Repr (o);
    if ( py_repr && PyString_Check(py_repr) ) {
      repr = PyString_AsString(py_repr);
    }
    Py_XDECREF( py_repr );
    m_log << MSG::ERROR << "content: " << repr << endmsg;
  }
  Py_DECREF (o);
  Py_XDECREF(item);
  return contains;
}

///////////////////////////////////////////////////////////////////
// Non-const methods:
///////////////////////////////////////////////////////////////////

/** @brief: allow a @c IIoComponent to register itself with this
 *          manager so appropriate actions can be taken when e.g.
 *          a @c fork(2) has been issued (this is usually handled
 *          by calling @c IIoComponent::io_reinit on every registered
 *          component)
 */
StatusCode
IoComponentMgr::io_register (IIoComponent* iocomponent)
{
  if ( 0 == iocomponent ) {
    m_log << MSG::ERROR
	  << "io_register (component) received a NULL pointer !" << endmsg;
    return StatusCode::FAILURE;
  }
  const std::string& ioname = iocomponent->name();
  IoRegistry_t::iterator itr = m_ioregistry.find (ioname);
  if ( itr == m_ioregistry.end() ) {
    iocomponent->addRef(); // ownership...
    m_ioregistry[ioname] = iocomponent;
    m_iostack.push_back (iocomponent);
  } else {
    m_log << MSG::INFO << "IoComponent[" << iocomponent->name()
	  <<"] already registered @" << (void*)itr->second << endmsg;
  }
  return StatusCode::SUCCESS;
}

/** @brief: allow a @c IIoComponent to register itself with this
 *          manager so appropriate actions can be taken when e.g.
 *          a @c fork(2) has been issued (this is usually handled
 *          by calling @c IIoComponent::io_reinit on every registered
 *          component)
 */
StatusCode
IoComponentMgr::io_register (IIoComponent* iocomponent,
			     IIoComponentMgr::IoMode::Type iomode,
			     const std::string& fname)
{
  if ( 0 == iocomponent ) {
    return StatusCode::FAILURE;
  }
  const std::string& ioname = iocomponent->name();

  if ( !io_hasitem (iocomponent) ) {
    if ( !io_register (iocomponent).isSuccess() ) {
      m_log << MSG::ERROR
	    << "could not register component [" << iocomponent->name() << "] "
	    << "with the I/O component manager !"
	    << endmsg;
      return StatusCode::FAILURE;
    }
  }

  // m_dict is a python dictionary like so:
  //  { 'iocomp-name' : { 'oldfname' : [ 'iomode', 'newfname' ] } }

  // -> check there is an 'iocomp-name' entry
  // -> retrieve that entry
  PyObject *o = PyDict_GetItemString (m_dict, (char*)ioname.c_str());

  // -> check it is a dictionary
  if ( NULL==o ) {
    o = PyDict_New();
    if (NULL == o) {
      m_log << MSG::ERROR << "could not create an I/O entry for ["
	    << ioname << "] "
	    << "in the I/O registry !" << endmsg;
      return StatusCode::FAILURE;
    }
    if ( 0 != PyDict_SetItemString (m_dict, (char*)ioname.c_str(), o) ) {
      Py_DECREF (o);
      m_log << MSG::ERROR << "could not create an I/O entry for ["
	    << ioname << "] " << "in the I/O registry !" << endmsg;
      return StatusCode::FAILURE;
    }
  } else if ( !PyDict_Check (o) ) {
    m_log << MSG::ERROR
	  << "internal consistency error (expected a dictionary !)"
	  << endmsg;
    return StatusCode::FAILURE;
  } else {
    // borrowed ref.
    Py_INCREF (o);
  }

  // -> check if 'fname' has already been registered
  std::string mode;
  switch (iomode) {
  case IIoComponentMgr::IoMode::Input: mode ="<input>"; break;
  case IIoComponentMgr::IoMode::Output:mode ="<output>";break;
  default:
    m_log << MSG::ERROR << "unknown value for iomode: [" << iomode << "] !"
	  << endmsg;
    Py_DECREF (o);
    return StatusCode::FAILURE;
  }

  PyObject *val = PyDict_GetItemString (o, (char*)fname.c_str());
  if ( 0 != val ) {
    Py_DECREF (o);
    return StatusCode::SUCCESS;
  }

  val = PyList_New(2);
  if ( 0 == val ) {
    m_log << MSG::ERROR << "could not allocate a python-list !" << endmsg;
    Py_DECREF (o);
    return StatusCode::FAILURE;
  }

  int err = PyList_SetItem (val,
			    0, PyString_FromString ((char*)mode.c_str()));
  if (err) {
    Py_DECREF (val);
    Py_DECREF (o);
    m_log << MSG::ERROR << "could not set py-iomode !" << endmsg;
    return StatusCode::FAILURE;
  }
  Py_INCREF (Py_None);
  // PyList_SetItem steals the ref...
  err = PyList_SetItem (val, 1, Py_None);
  if (err) {
    Py_DECREF (val);
    Py_DECREF (o);
    m_log << MSG::ERROR << "could not properly fill python-list !" << endmsg;
    return StatusCode::FAILURE;
  }
  err = PyDict_SetItemString (o, (char*)fname.c_str(), val);
  if (err) {
    m_log << MSG::ERROR << "could not properly fill registry w/ python-list !"
	  << endmsg;
    Py_DECREF (val);
    Py_DECREF (o);
    return StatusCode::FAILURE;
  }
  Py_DECREF (o);
  return StatusCode::SUCCESS;
}

/** @brief: retrieve the new filename for a given @c IIoComponent and
 *          @param `oldfname` filename
 */
StatusCode
IoComponentMgr::io_retrieve (IIoComponent* iocomponent,
			     std::string& fname)
{
  if ( 0 == iocomponent ) {
    return StatusCode::FAILURE;
  }

  const std::string& ioname = iocomponent->name();

  // m_dict is a python dictionary like so:
  //  { 'iocomp-name' : { 'oldfname' : [ 'iomode', 'newfname' ] } }

  // -> check there is an 'iocomp-name' entry
  // -> retrieve that entry
  PyObject *o = PyDict_GetItemString (m_dict, (char*)ioname.c_str());
  Py_XINCREF (o);

  // -> check it is a dictionary
  if ( NULL==o || !PyDict_Check (o) ) {
    Py_XDECREF (o);
    return StatusCode::FAILURE;
  }

  // -> check 'oldfname' exists
  PyObject *pylist = PyDict_GetItemString (o, (char*)fname.c_str());
  Py_XINCREF (pylist);

  if ( NULL==pylist || !PyList_Check (pylist) ) {
    Py_XDECREF(pylist);
    Py_DECREF (o);
    return StatusCode::FAILURE;
  }

  const std::size_t sz = PyList_Size (pylist);
  if ( 2 != sz ) {
    m_log << MSG::ERROR << "[" << ioname << "][" << fname << "] list has size ["
	  << sz << " ! (expected sz==2)"
	  << endmsg;
    Py_DECREF (o);
    Py_DECREF (pylist);
    return StatusCode::FAILURE;
  }

  fname = PyString_AsString ( PyList_GetItem (pylist, 1) );
  Py_DECREF (o);
  Py_DECREF (pylist);
  return StatusCode::SUCCESS;
}

/** @brief: reinitialize the I/O subsystem.
 *  This effectively calls @c IIoComponent::io_reinit on all the registered
 *  @c IIoComponent.
 */
StatusCode
IoComponentMgr::io_reinitialize ()
{
  m_log << MSG::DEBUG << "reinitializing I/O subsystem..." << endmsg;
  bool allgood = true;
  for ( IoStack_t::iterator io = m_iostack.begin(), ioEnd = m_iostack.end();
	io != ioEnd;
	++io ) {
    m_log << MSG::DEBUG << " [" << (*io)->name() << "]->io_reinit()..."
	  << endmsg;
    if ( !(*io)->io_reinit().isSuccess() ) {
      allgood = false;
      m_log << MSG::ERROR << "problem in [" << (*io)->name()
	    << "]->io_reinit() !" << endmsg;
    }
    // we are done with this guy... release it
    (*io)->release();
  }

  // we are done.
  // FIXME: shall we allow for multiple io_reinitialize ?
  m_iostack.clear();
  m_ioregistry.clear();

  return allgood
    ? StatusCode::SUCCESS
    : StatusCode::FAILURE;
}

/** @brief: finalize the I/O subsystem.
 *  Hook to allow to e.g. give a chance to I/O subsystems to merge output
 *  files. Not sure how to do this correctly though...
 */
StatusCode
IoComponentMgr::io_finalize ()
{
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Protected methods:
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// Const methods:
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// Non-const methods:
///////////////////////////////////////////////////////////////////


