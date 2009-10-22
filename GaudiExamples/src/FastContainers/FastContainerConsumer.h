// $Id: FastContainerConsumer.h,v 1.2 2006/11/29 18:47:33 hmd Exp $
#ifndef FASTCONTAINERCONSUMER_H 
#define FASTCONTAINERCONSUMER_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

#include "GaudiKernel/TransientFastContainer.h"

#include "FastContainerProducer.h"

/** @class FastContainerConsumer FastContainerConsumer.h
 *  
 *
 *  @author Marco Clemencic
 *  @date   2006-05-03
 */
class FastContainerConsumer : public GaudiAlgorithm {
public: 

  typedef FastContainerProducer::internal_type internal_type;
  typedef FastContainerProducer::container_type container_type;

  /// Standard constructor
  FastContainerConsumer( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~FastContainerConsumer( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

  std::string m_containerLocation;

  container_type *m_container;

};
#endif // FASTCONTAINERCONSUMER_H
