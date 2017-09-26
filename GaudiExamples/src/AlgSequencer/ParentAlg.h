#ifndef GAUDIEXAMPLE_PARENTALG_H
#define GAUDIEXAMPLE_PARENTALG_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

/** @class ParentAlg
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class ParentAlg : public GaudiAlgorithm
{
public:
  /// Constructor of this form must be provided
  ParentAlg( const std::string& name, ISvcLocator* pSvcLocator );

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  Algorithm* m_subalg1;
  Algorithm* m_subalg2;
};

#endif // GAUDIEXAMPLE_PARENTALG_H
