// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IPersistencySvc.h,v 1.3 2003/08/05 13:49:16 mato Exp $
#ifndef KERNEL_IPERSISTENCYSVC_H
#define KERNEL_IPERSISTENCYSVC_H

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IInterface.h"

// Forward declarations
// Generic interface to data object class
class IConversionSvc;

/** @class IPersistencySvc IPersistencySvc.h GaudiKernel/IPersistencySvc.h

    Data persistency service interface.

    @author Markus Frank
    @version 1.0
*/
class GAUDI_API IPersistencySvc: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IPersistencySvc,2,0);

  /** Set default service type.
      @return     Status code indicating success or failure.
      @param      type Storage technology type.
  */
  virtual StatusCode setDefaultCnvService(long type) = 0;

  /** Add new Conversion Service.
      @return     Status code indicating success or failure.
      @param      service Pointer to conversion service interface.
  */
  virtual StatusCode addCnvService(IConversionSvc* service) = 0;

  /** Remove Conversion Service.
      @return     Status code indicating success or failure.
      @param      type Storage technology type.
  */
  virtual StatusCode removeCnvService(long type) = 0;

  /** Retrieve conversion service identified by technology
      @return     Status code indicating success or failure.
      @param      service_type Storage technology type.
      @param      refpSvc      Reference to pointer to interface of required service.
  */
  virtual StatusCode getService(long service_type, IConversionSvc*& refpSvc) = 0;
  /** Retrieve conversion service identified by technology
      @return     Status code indicating success or failure.
      @param      service_type Storage technology type (ROOT, MS Access, ....)
      @param      refpSvc      Reference to pointer to interface of required service.
  */
  virtual StatusCode getService(const std::string& service_type, IConversionSvc*& refpSvc) = 0;
};

#endif // KERNELGAUDI_IPERSISTENCYSVC_H
