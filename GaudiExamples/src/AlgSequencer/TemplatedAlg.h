// $Id: TemplatedAlg.h,v 1.1 2008/10/09 21:00:37 marcocle Exp $
#ifndef GAUDIEXAMPLE_TEMPLATEALG_H
#define GAUDIEXAMPLE_TEMPLATEALG_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

/** @class TemplateAlg
    Trivial Algorithm for tutotial purposes
    
    @author nobody
*/
template <typename T, typename R> class TemplatedAlg : public GaudiAlgorithm {
public:
  /// Constructor of this form must be provided
  TemplatedAlg(const std::string& name, ISvcLocator* pSvcLocator); 

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
private:
  T  m_t;
  R  m_r;
};

#endif    // GAUDIEXAMPLE_TEMPLATEDALG_H
