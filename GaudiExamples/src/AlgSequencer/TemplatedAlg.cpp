// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IEventProcessor.h"

#include "TemplatedAlg.h"
#include <vector>

// Static Factory declaration
typedef TemplatedAlg<int,std::vector<std::string> > t1;
typedef TemplatedAlg<double, bool > t2;

DECLARE_COMPONENT(t1)
DECLARE_COMPONENT_WITH_ID(t1,"TAlgIS")
DECLARE_COMPONENT(t2)
DECLARE_COMPONENT_WITH_ID(t2,"TAlgDB")

//------------------------------------------------------------------------------
template <typename T, typename R>
StatusCode TemplatedAlg<T,R>::initialize() {
//------------------------------------------------------------------------------
  info() << "Initializing TemplatedAlg instance " << name() << " of type " << typeid(*this).name() << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
template <typename T, typename R>
StatusCode TemplatedAlg<T,R>::execute() {
//------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
template <typename T, typename R>
StatusCode TemplatedAlg<T,R>::finalize() {
//------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}
