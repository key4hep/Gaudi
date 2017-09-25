#ifndef COLORMSG_COLORMSGALG_H
#define COLORMSG_COLORMSGALG_H

//<<<<<< INCLUDES                                                       >>>>>>
#include "GaudiKernel/Algorithm.h"
//<<<<<< CLASS DECLARATIONS                                             >>>>>>
/** @class ColorMgs
 * @brief  an algorithm to test the color setting options of MsgService
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */

class ColorMsgAlg : public Algorithm
{

public:
  ColorMsgAlg( const std::string& name, ISvcLocator* pSvcLocator );
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
};

//<<<<<< INLINE PUBLIC FUNCTIONS                                        >>>>>>
//<<<<<< INLINE MEMBER FUNCTIONS                                        >>>>>>

#endif // COLORMSG_COLORMSGALG_H
