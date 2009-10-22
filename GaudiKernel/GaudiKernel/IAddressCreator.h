// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IAddressCreator.h,v 1.10 2003/11/27 10:20:57 mato Exp $
#ifndef GAUDIKERNEL_IADDRESSCREATOR_H
#define GAUDIKERNEL_IADDRESSCREATOR_H

// Framework include files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ClassID.h"

// C++ include files
#include <string>

// Forward declarations
class IOpaqueAddress;

/** @class IAddressCreator IAddressCreator.h GaudiKernel/IAddressCreator.h

    IAddressCreator interface definition.
    The address creator allows to:
    <UL>
    <LI> Create persistent address representations for a given
         conversion service using this service itself. The aim is to
         allow address creation without coupling directly to the service.
    </UL>


    @author Markus Frank
    @version 1.0
*/
class GAUDI_API IAddressCreator: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IAddressCreator,3,0);

  /** Create a Generic address using explicit arguments to
      identify a single object.
      @param      svc_type    Technology identifier encapsulated
                              in this address.
      @param      clid        Class identifier of the DataObject
                              represented by the opaque address
      @param      par         Array of strings needed to
                              construct the opaque address.
      @param      ipar        Array of integers needed to
                              construct the opaque address.
      @param      refpAddress Reference to pointer to the address where the created
                              Address should be stored.
      @return     Status code indicating success or failure.
  */
  virtual StatusCode createAddress( long svc_type,
                                    const CLID& clid,
                                    const std::string* par,
                                    const unsigned long* ipar,
                                    IOpaqueAddress*& refpAddress) = 0;

  /** Convert an address to string form
      @param      pAddress    Input address.
      @param      refAddress  Output address in string form.
      @return     Status code indicating success or failure.
  */
  virtual StatusCode convertAddress( const IOpaqueAddress* pAddress,
                                     std::string& refAddress) = 0;

  /** Creates an address in string form to object form
      @param      svc_type    Technology identifier encapsulated
                              in this address.
      @param      clid        Class identifier of the DataObject
                              represented by the opaque address
      @param      address     Input address.
      @param      refpAddress Output address in string form.
      @return     Status code indicating success or failure.
  */
  virtual StatusCode createAddress( long svc_type,
                                    const CLID& clid,
                                    const std::string& refAddress,
                                    IOpaqueAddress*& refpAddress) = 0;
};

#endif // GAUDIKERNEL_IADDRESSCREATOR_H
