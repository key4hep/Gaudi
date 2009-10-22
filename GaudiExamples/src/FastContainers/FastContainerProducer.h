// $Id: FastContainerProducer.h,v 1.2 2006/11/29 18:47:34 hmd Exp $
#ifndef FASTCONTAINERPRODUCER_H 
#define FASTCONTAINERPRODUCER_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

#include "GaudiKernel/TransientFastContainer.h" 

/** @class FastContainerProducer FastContainerProducer.h
 *  
 *
 *  @author Marco Clemencic
 *  @date   2006-05-03
 */
class FastContainerProducer : public GaudiAlgorithm {
public: 

  typedef double internal_type;
  typedef TransientFastContainer<internal_type> container_type;

  /// Standard constructor
  FastContainerProducer( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~FastContainerProducer( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

  std::string m_containerLocation;

  container_type *m_container;
  
};
#endif // FASTCONTAINERPRODUCER_H
