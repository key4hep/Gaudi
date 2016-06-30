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
  using GaudiAlgorithm::GaudiAlgorithm;

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;
private:
  PropertyWithValue<T>  m_t{this, "TProperty", {}};
  PropertyWithValue<R>  m_r{this, "RProperty", {}};
};

#endif    // GAUDIEXAMPLE_TEMPLATEDALG_H
