#ifndef GAUDIEXAMPLE_HELLOWORLD_H
#define GAUDIEXAMPLE_HELLOWORLD_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

/** @class HelloWorld
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class HelloWorld : public Algorithm
{
public:
  /// Constructor of this form must be provided
  HelloWorld( const std::string& name, ISvcLocator* pSvcLocator );

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  bool m_initialized;
  /// These data members are used in the execution of this algorithm
  /// They are set in the initialisation phase by the job options service
  Gaudi::Property<int> m_int;
  Gaudi::Property<double> m_double;
  Gaudi::Property<std::string> m_string;
  int t_int;
  double t_double;
  std::string t_string;
};

#endif // GAUDIEXAMPLE_HELLOWORLD_H
