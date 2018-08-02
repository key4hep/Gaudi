#ifndef GAUDIKERNEL_IDATABROKER_H
#define GAUDIKERNEL_IDATABROKER_H

// Framework includes
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/IInterface.h"

// C++ includes
#include <string>
#include <vector>

// Forward class declaration
class Algorithm;

/** @class IAlgResourcePool IAlgResourcePool.h GaudiKernel/IAlgResourcePool.h

    The IDataBroker interface provides an ordered list of algorithms that
    are to be executed in order for the provided DataObjID to be produced.
    Note that this list is both neccessary and sufficient, in the sense that
    in the absence of errors encountered during the execution, there will be
    an item at DataObjID after executing this sequence.

    The actual creation and deletion of algorithm instances implied by this
    interface is assumed to implemented through the IAlgManager service.

    @author Gerhard Raven
    @version 1.0
*/
struct GAUDI_API IDataBroker : extend_interfaces<IInterface> {
  /// InterfaceID
  DeclareInterfaceID( IDataBroker, 1, 0 );

  /// Get the (ordered!) list of algorithms required to provide a given DataObjIDColl
  virtual std::vector<Algorithm*> algorithmsRequiredFor( const DataObjIDColl& requested ) const = 0;

  /// Get the (ordered!) list of algorithms required to succesfully execute (as far as data dependencies go)
  /// the specified algorithm
  virtual std::vector<Algorithm*> algorithmsRequiredFor( const Gaudi::Utils::TypeNameString& alg ) const = 0;
};

#endif // GAUDIKERNEL_IDATABROKER
