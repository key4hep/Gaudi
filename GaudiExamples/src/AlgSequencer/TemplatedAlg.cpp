// $Id: TemplatedAlg.cpp,v 1.1 2008/10/09 21:00:37 marcocle Exp $

// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IEventProcessor.h"

#include "TemplatedAlg.h"
#include <vector>

// Static Factory declaration
typedef TemplatedAlg<int,std::vector<std::string> > t1;
typedef TemplatedAlg<double, bool > t2;

DECLARE_ALGORITHM_FACTORY(t1)
DECLARE_NAMED_ALGORITHM_FACTORY(t1,TAlgIS)
DECLARE_ALGORITHM_FACTORY(t2)
DECLARE_NAMED_ALGORITHM_FACTORY(t2,TAlgDB)


template <typename T, typename R>
TemplatedAlg<T,R>::TemplatedAlg(const std::string& name, ISvcLocator* ploc)
          : GaudiAlgorithm(name, ploc), m_t(), m_r() {
//------------------------------------------------------------------------------
  declareProperty("TProperty", m_t);
  declareProperty("RProperty", m_r);
}

//------------------------------------------------------------------------------
template <typename T, typename R>
StatusCode TemplatedAlg<T,R>::initialize() {
//------------------------------------------------------------------------------
  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "Initializing TemplatedAlg instance " << name() << " of type " << typeid(*this).name() << endmsg;
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
