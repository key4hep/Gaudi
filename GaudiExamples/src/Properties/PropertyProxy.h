#ifndef GAUDIEXAMPLE_PropertyProxy_H
#define GAUDIEXAMPLE_PropertyProxy_H

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

/** @class PropertyProxy
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class PropertyProxy : public Algorithm {
public:
  /// Constructor of this form must be provided
  PropertyProxy( const std::string& name, ISvcLocator* pSvcLocator );

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  IProperty* m_remAlg;
};

#endif // GAUDIEXAMPLE_PropertyProxy_H
