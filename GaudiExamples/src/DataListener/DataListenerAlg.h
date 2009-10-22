// $Id: DataListenerAlg.h,v 1.1 2006/09/04 08:22:30 hmd Exp $
#ifndef DATALISTENER_DATALISTENER_H 
#define DATALISTENER_DATALISTENER_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
//#include "GaudiSvc/v14r6/src/DataListenerSvc/DataListenerSvc.h"

/** @class DataListenerAlg DataListenerAlg.h DataListener/DataListenerAlg.h
 *  
 *
 *  @author Ben King
 *  @date   2006-08-28
 */
class DataListenerAlg : public GaudiAlgorithm {
public: 
  /// Standard constructor
  DataListenerAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~DataListenerAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:
  int m_fibo;
  int m_geo;
  
  int m_arith;
  double m_expo;
  
  int m_counter;
  

  IMonitorSvc *m_DataListenerSvc;
  

//   StatusCode Fibonacci(int& startPoint1, int startPoint2, unsigned int numSteps);
//   StatusCode Arith(int& startPoint, int difference, unsigned int numSteps);
//   StatusCode Geo(int& startPoint, int ratio, unsigned int numSteps);
//   StatusCode Expo(double& startPoint, unsigned int numSteps);


  StatusCode Fibonacci(int startPoint2, unsigned int numSteps);
  StatusCode Arith( int difference, unsigned int numSteps);
  StatusCode Geo( int ratio, unsigned int numSteps);
  StatusCode Expo( unsigned int numSteps);
  
  

};
#endif // DATALISTENER_DATALISTENER_H
