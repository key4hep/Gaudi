///////////////////////// -*- C++ -*- /////////////////////////////
// IIoComponentMgr.h 
// Header file for class IIoComponentMgr
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef GAUDIKERNEL_IIOCOMPONENTMGR_H 
#define GAUDIKERNEL_IIOCOMPONENTMGR_H 1 

/** @class IIoComponentMgr
 */

// STL includes
#include <string>

// GaudiKernel includes
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/StatusCode.h"

// forward declare
class IIoComponent;

class GAUDI_API IIoComponentMgr : virtual public INamedInterface
{ 
 public:
  DeclareInterfaceID(IIoComponentMgr,1,0);

  /////////////////////////////////////////////////////////////////// 
  // Public enums: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  struct IoMode {
    enum Type {
      Input  = 0,
      Output
    };
  };

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Destructor: 
   */
  virtual ~IIoComponentMgr();

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  virtual
  StatusCode io_register (IIoComponent* iocomponent) = 0;

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  virtual
  StatusCode io_register (IIoComponent* iocomponent,
			  IIoComponentMgr::IoMode::Type iomode,
			  const std::string& fname) = 0;

  /** @brief: check if the registry contains a given @c IIoComponent
   */
  virtual
  bool io_hasitem (IIoComponent* iocomponent) const = 0;

  /** @brief: check if the registry contains a given @c IIoComponent and
   *          that component had @param `fname` as a filename
   */
  virtual
  bool io_contains (IIoComponent* iocomponent,
		    const std::string& fname) const = 0;

  /** @brief: retrieve the new filename for a given @c IIoComponent and
   *          @param `fname` filename
   */
  virtual
  StatusCode io_retrieve (IIoComponent* iocomponent,
			  std::string& fname) = 0;

  /** @brief: reinitialize the I/O subsystem.
   *  This effectively calls @c IIoComponent::io_reinit on all the registered
   *  @c IIoComponent.
   */
  virtual
  StatusCode io_reinitialize () = 0;

  /** @brief: finalize the I/O subsystem.
   *  Hook to allow to e.g. give a chance to I/O subsystems to merge output
   *  files. Not sure how to do this correctly though...
   */
  virtual
  StatusCode io_finalize () = 0;

}; 


#endif //> !GAUDIKERNEL_IIOCOMPONENTMGR_H
