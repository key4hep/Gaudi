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
// Include files
#include "QotdAlg.h"

#include <GaudiKernel/MsgStream.h>

using namespace GaudiEx;

DECLARE_COMPONENT( QotdAlg )

//------------------------------------------------------------------------------
QotdAlg::QotdAlg( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator )
    , m_evtCnt( 0 )
//------------------------------------------------------------------------------
{}

//------------------------------------------------------------------------------
StatusCode QotdAlg::initialize()
//------------------------------------------------------------------------------
{
  info() << "Initializing " << name() << "..." << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode QotdAlg::execute()
//------------------------------------------------------------------------------
{
  info() << "Event #" << m_evtCnt++ << "\n"
         << " --- famous quotes ---\n"
         << " - God does not play dice with the Universe.\n"
         << " - 640K of memory should be enough for anybody.\n"
         << " - Always code as if the guy maintaining your code would be a violent psychopath knowing where you live.\n"
         << " - In a few minutes a computer can make a mistake so great that it would have taken many men many months "
            "to equal it.\n"
         << " - GIGO is not a design pattern.\n"
         << " --- famous quotes --- [DONE]" << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode QotdAlg::finalize()
//------------------------------------------------------------------------------
{
  info() << "Finalizing..." << endmsg;
  return StatusCode::SUCCESS;
}
