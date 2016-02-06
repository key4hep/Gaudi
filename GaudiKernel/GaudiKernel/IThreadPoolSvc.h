#ifndef GAUDIKERNEL_ITHREADPOOLSVC_H
#define GAUDIKERNEL_ITHREADPOOLSVC_H

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IThreadInitTool.h"
#include <vector>

//---------------------------------------------------------------------------

/** IThreadPoolSvc GaudiKernel/IThreadPoolSvc.h
 *
 * Abstract interface to initialize thread pool, and retrieve thread local
 * initialization tools.
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class GAUDI_API  IThreadPoolSvc : virtual public IInterface {
 public:

  /// InterfaceID
  DeclareInterfaceID(IThreadPoolSvc,1,0);

  virtual StatusCode initPool(const int& poolSize) = 0;

  virtual int poolSize() const = 0;

  virtual std::vector<IThreadInitTool*> getThreadInitTools() const = 0;

};

#endif
