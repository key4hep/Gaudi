///////////////////////// -*- C++ -*- /////////////////////////////
// IoComponentMgr.h 
// Header file for class IoComponentMgr
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef GAUDIMP_IOCOMPONENTMGR_H
#define GAUDIMP_IOCOMPONENTMGR_H 1

// Python includes
#include <Python.h>

// STL includes
#include <string>
#include <map>
#include <list>

// FrameWork includes
#include "GaudiKernel/Service.h"

// GaudiKernel
#include "GaudiKernel/IIoComponent.h"
#include "GaudiKernel/IIoComponentMgr.h"

// Forward declaration
class ISvcLocator;
template <class TYPE> class SvcFactory;


class IoComponentMgr: public extends1<Service, IIoComponentMgr> {

  friend class SvcFactory<IoComponentMgr>;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  IoComponentMgr( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~IoComponentMgr(); 

  // Assignment operator: 
  //IoComponentMgr &operator=(const IoComponentMgr &alg); 

  /// Gaudi Service Implementation
  //@{
  virtual StatusCode initialize();
  virtual StatusCode finalize();

  //@}

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /** @brief: check if the registry contains a given @c IIoComponent
   */
  virtual
  bool io_hasitem (IIoComponent* iocomponent) const;

  /** @brief: check if the registry contains a given @c IIoComponent and
   *          that component had @param `fname` as a filename
   */
  virtual
  bool io_contains (IIoComponent* iocomponent,
		    const std::string& fname) const;

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  virtual
  StatusCode io_register (IIoComponent* iocomponent);

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  virtual
  StatusCode io_register (IIoComponent* iocomponent,
			  IIoComponentMgr::IoMode::Type iomode,
			  const std::string& fname);

  /** @brief: retrieve the new filename for a given @c IIoComponent and
   *          @param `fname` filename
   */
  virtual
  StatusCode io_retrieve (IIoComponent* iocomponent,
			  std::string& fname);

  /** @brief: reinitialize the I/O subsystem.
   *  This effectively calls @c IIoComponent::io_reinit on all the registered
   *  @c IIoComponent.
   */
  virtual
  StatusCode io_reinitialize ();

  /** @brief: finalize the I/O subsystem.
   *  Hook to allow to e.g. give a chance to I/O subsystems to merge output
   *  files. Not sure how to do this correctly though...
   */
  virtual
  StatusCode io_finalize ();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  IoComponentMgr();

  mutable MsgStream m_log;

  typedef std::map<std::string, IIoComponent*> IoRegistry_t;
  /// Registry of @c IIoComponents
  IoRegistry_t m_ioregistry;

  typedef std::list<IIoComponent*> IoStack_t;
  /// Stack of @c IIoComponents to properly handle order of registration
  IoStack_t m_iostack;

  /** pointer to a python dictionary holding the associations:
   *    { 'iocomp-name' : { 'oldfname' : [ 'iomode', 'newfname' ] } }
   */
  PyObject *m_dict;

  /// location of the python dictionary
  std::string m_dict_location;

}; 


#endif //> !GAUDIMP_IOCOMPONENTMGR_H
