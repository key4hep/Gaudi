/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef COLORMSG_COLORMSGALG_H
#define COLORMSG_COLORMSGALG_H

//<<<<<< INCLUDES                                                       >>>>>>
#include <GaudiKernel/Algorithm.h>
//<<<<<< CLASS DECLARATIONS                                             >>>>>>
/** @class ColorMgs
 * @brief  an algorithm to test the color setting options of MsgService
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */

class ColorMsgAlg : public Algorithm {

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
