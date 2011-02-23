// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiExamples/src/Properties/PropertyProxy.h,v 1.1 2001/06/26 17:05:47 mato Exp $
#ifndef GAUDIEXAMPLE_PropertyProxy_H
#define GAUDIEXAMPLE_PropertyProxy_H

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

/** @class PropertyProxy
    Trivial Algorithm for tutotial purposes
    
    @author nobody
*/
class PropertyProxy : public Algorithm {
public:
  /// Constructor of this form must be provided
  PropertyProxy(const std::string& name, ISvcLocator* pSvcLocator); 

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  IProperty* m_remAlg;
};

#endif    // GAUDIEXAMPLE_PropertyProxy_H
