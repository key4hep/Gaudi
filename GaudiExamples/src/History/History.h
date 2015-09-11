#ifndef GAUDIEXAMPLES_HISTORY_H
# define GAUDIEXAMPLES_HISTORY_H

//<<<<<< INCLUDES                                                       >>>>>>
#include "GaudiKernel/Algorithm.h"
//<<<<<< CLASS DECLARATIONS                                             >>>>>>
/** @class ColorMgs
 * @brief  an algorithm to test the color setting options of MsgService
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */

class IHistorySvc;

class History : public Algorithm {

public:
  History (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  
private:

  IHistorySvc* m_histSvc;

};



//<<<<<< INLINE PUBLIC FUNCTIONS                                        >>>>>>
//<<<<<< INLINE MEMBER FUNCTIONS                                        >>>>>>

#endif // GAUDIEXAMPLES_HISTORY_H
