// $Id: HelloWorld.h,v 1.5 2005/01/17 17:27:25 mato Exp $
#ifndef GAUDIEXAMPLE_HELLOWORLD_H
#define GAUDIEXAMPLE_HELLOWORLD_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

/** @class HelloWorld
    Trivial Algorithm for tutotial purposes
    
    @author nobody
*/
class HelloWorld : public Algorithm {
public:
  /// Constructor of this form must be provided
  HelloWorld(const std::string& name, ISvcLocator* pSvcLocator); 

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  StatusCode beginRun();
  StatusCode endRun();
private:
  bool m_initialized;
  /// These data members are used in the execution of this algorithm
  /// They are set in the initialisation phase by the job options service
  IntegerProperty m_int;
  DoubleProperty  m_double;
  StringProperty  m_string;
  int t_int;
  double  t_double;
  std::string  t_string;
};

#endif    // GAUDIEXAMPLE_HELLOWORLD_H
