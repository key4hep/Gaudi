// $Id: IJobOptionsSvc.h,v 1.6 2006/07/31 08:25:29 mato Exp $
#ifndef KERNEL_IJOBOPTIONSSVC_H
#define KERNEL_IJOBOPTIONSSVC_H

// Include files
#include "GaudiKernel/IInterface.h"
#include <string>
#include <vector>

// Forward declaration
class StatusCode;
class IProperty;
class Property;

/** @class IJobOptionsSvc IJobOptionsSvc.h GaudiKernel/IJobOptionsSvc.h

    Main interface for the JobOptions service

    @author Pere Mato
*/
class GAUDI_API IJobOptionsSvc: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IJobOptionsSvc,4,0);

  /** Override default properties of the calling client
      @param client Name of the client algorithm or service
      @param me Address of the interface IProperty of the client
  */
  virtual StatusCode setMyProperties
  ( const std::string& client, IProperty* me ) = 0;

  /// Add a property into the JobOptions catalog
  virtual StatusCode addPropertyToCatalogue( const std::string& client,
                                             const Property& property ) = 0;
  /// Remove a property from the JobOptions catalog
  virtual StatusCode removePropertyFromCatalogue( const std::string& client,
                                                  const std::string& name ) = 0;
  /// Get the properties associated to a given client
  virtual const std::vector<const Property*>*
  getProperties( const std::string& client) const = 0;

  /// Get the list of clients
  virtual std::vector<std::string> getClients() const = 0;

  /** look for file 'File' into search path 'Path'
   *  and read it to update existing JobOptionsCatalogue
   *  @param File file   name
   *  @param Path search path
   *  @return status code
   */
  virtual StatusCode readOptions
  ( const std::string& file      ,
    const std::string& path = "" ) = 0 ;

};

#endif  // KERNEL_IJOBOPTIONSSVC_H
