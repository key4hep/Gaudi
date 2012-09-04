//
//  HiveTestAlgorithm.h
//  
//
//  Created by Benedikt Hegner on 7/21/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"

class GAUDI_API HiveTestAlgorithm: public Algorithm {
 public:

  /**
   ** Constructor(s)
   **/
  HiveTestAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  /**
   ** Destructor
   **/
  virtual ~HiveTestAlgorithm( );

  /*****************************
   ** Public Function Members **
   *****************************/

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  // BH: concurrency additions
  virtual const std::vector<std::string> get_inputs();
  virtual const std::vector<std::string> get_outputs();

 private:

  /**************************
   ** Private Data Members **
   **************************/

  /**
   ** The total events seen.
   **/
  int m_total;

  std::vector<std::string> m_inputs;
  std::vector<std::string> m_outputs;

};

