#ifndef GAUDIKERNEL_IDATASTOREAGENT_H
#define GAUDIKERNEL_IDATASTOREAGENT_H

// Framework include files
#include "GaudiKernel/Kernel.h"
#include <functional>

// Forward declarations:
class IRegistry;

/** @class IDataStoreAgent IDataStoreAgent.h GaudiKernel/IDataStoreAgent.h

    Generic data agent interface

    @author Markus Frank
*/
class GAUDI_API IDataStoreAgent {
public:
  /// destructor
  virtual ~IDataStoreAgent() = default;

  /** Analyse the data object.
  @return Boolean indicating wether the tree below should be analysed
  */
  virtual bool analyse( IRegistry* pObject, int level ) = 0;
};

namespace details {
  template <typename F>
  class GenericDataStoreAgent final : public IDataStoreAgent {
    F f;

  public:
    template <typename G>
    GenericDataStoreAgent( G&& g ) : f{std::forward<G>( g )} {}

    bool analyse( IRegistry* pObj, int level ) override { return std::invoke( f, pObj, level ); }
  };
} // namespace details

template <typename F>
::details::GenericDataStoreAgent<F> makeDataStoreAgent( F&& f ) {
  return {std::forward<F>( f )};
}

#endif // GAUDIKERNEL_IDATASTOREAGENT_H
