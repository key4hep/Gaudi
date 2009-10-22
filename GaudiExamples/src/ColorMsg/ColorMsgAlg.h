#ifndef COLORMSG_COLORMSGALG_H
# define COLORMSG_COLORMSGALG_H

//<<<<<< INCLUDES                                                       >>>>>>
#include "GaudiKernel/Algorithm.h"
//<<<<<< CLASS DECLARATIONS                                             >>>>>>
/** @class ColorMgs
 * @brief  an algorithm to test the color setting options of MsgService
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 *$Id: ColorMsgAlg.h,v 1.1 2003/02/28 01:21:22 calaf Exp $
 */



class ColorMsgAlg : public Algorithm {

public:
  ColorMsgAlg (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  
private:

};



//<<<<<< INLINE PUBLIC FUNCTIONS                                        >>>>>>
//<<<<<< INLINE MEMBER FUNCTIONS                                        >>>>>>

#endif // COLORMSG_COLORMSGALG_H
