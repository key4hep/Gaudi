#ifndef GaudiKernel_IFileAccess_H
#define GaudiKernel_IFileAccess_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include <istream>
#include <memory>
#include <string>
#include <vector>

/** @class IFileAccess IFileAccess.h GaudiKernel/IFileAccess.h
 *
 * Abstract interface for a service or tool implementing a read access to files.
 *
 * @author Marco Clemencic
 * @date   2008-01-18
 */
class GAUDI_API IFileAccess : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IFileAccess, 3, 0 );

  /// Find the URL and returns a unique_ptr to an input stream interface of an
  /// object that can be used to read from the file the URL is pointing to.
  /// Returns an empty pointer if the URL cannot be resolved.
  virtual std::unique_ptr<std::istream> open( const std::string& url ) = 0;

  /// Protocols supported by the instance.
  virtual const std::vector<std::string>& protocols() const = 0;
};

#endif // GaudiKernel_IFileAccess_H
