// $Id: HistoTimingAlg.h,v 1.1 2005/10/03 14:15:30 hmd Exp $
#ifndef HISTOGRAMS_HistoTimingAlg_H 
#define HISTOGRAMS_HistoTimingAlg_H 1

#include <map>
#include <sstream>

// GaudiKernel
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"

// from Gaudi
#include "GaudiAlg/GaudiHistoAlg.h"

/** @class HistoTimingAlg HistoTimingAlg.h Histograms/HistoTimingAlg.h
 *  
 *  Timing tests for various histogramming methods
 *
 *  @author Chris Jones
 *  @date   2005-08-12
 */

class HistoTimingAlg : public GaudiHistoAlg 
{

public: 

  /// Standard constructor
  HistoTimingAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~HistoTimingAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution

private:

  Rndm::Numbers m_rand;

  typedef std::map<IHistogram1D*,std::string> HistoMap;

  HistoMap m_histos;

  unsigned int m_nHistos;

  unsigned int m_nTracks;

  bool m_useGaudiAlg;

};

#endif // HISTOGRAMS_HistoTimingAlg_H
