// $Id: PPSLHCb.h,v 1.1 2008/05/29 18:12:56 marcocle Exp $
#ifndef PARTPROP_PPSLHCB_H 
#define PARTPROP_PPSLHCB_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"


/** @class PPSLHCb PPSLHCb.h PartProp/PPSLHCb.h
 *  
 *
 *  @author Marco CLEMENCIC
 *  @date   2008-05-23
 */
class PPSLHCb : public GaudiAlgorithm {
public: 
  /// Standard constructor
  PPSLHCb( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~PPSLHCb( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:

};
#endif // PARTPROP_PPSLHCB_H
