#ifndef AIDATUPLES_AIDATupleAlgorithmRead_H
#define AIDATUPLES_AIDATupleAlgorithmRead_H 1

// Include files
#include "AIDA/ITuple.h"
#include "GaudiKernel/Algorithm.h"

using namespace AIDA;

// Forward declarations
class AIDATupleSvc;

class AIDATupleAlgorithmRead : public Algorithm {

public:
  /// Constructor of this form must be provided
  AIDATupleAlgorithmRead( const std::string& name, ISvcLocator* pSvcLocator );

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  AIDA::ITuple* tuple;
};

#endif // AIDATUPLES_AIDATupleAlgorithmRead_H
