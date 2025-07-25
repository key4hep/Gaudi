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
#include "ColorMsgAlg.h"
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>

// Static Factory declaration

DECLARE_COMPONENT( ColorMsgAlg )

///////////////////////////////////////////////////////////////////////////

ColorMsgAlg::ColorMsgAlg( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator ) {}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ColorMsgAlg::initialize() { return StatusCode::SUCCESS; }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ColorMsgAlg::execute() {

  auto& log = msgStream();

  log << MSG::FATAL << "THIS IS A FATAL MESSAGE" << endmsg;
  log << MSG::ERROR << "THIS IS AN ERROR MESSAGE" << endmsg;
  log << MSG::WARNING << "THIS IS A WARNING MESSAGE" << endmsg;
  log << MSG::INFO << "THIS IS A INFO MESSAGE" << endmsg;
  log << MSG::DEBUG << "THIS IS A DEBUG MESSAGE" << endmsg;
  log << MSG::VERBOSE << "THIS IS A VERBOSE MESSAGE" << endmsg;

  log << MSG::ERROR << "this is another";
  log.setColor( MSG::GREEN );
  log << " error ";
  log.resetColor();
  log << "message" << endmsg;

  log << MSG::WARNING << "This is another";
  log.setColor( MSG::PURPLE );
  log << " warning ";
  log.resetColor();
  log << "message" << endmsg;

  log << MSG::INFO << "testing colour" << endmsg;

  log << MSG::INFO << "setting....";
  log.setColor( MSG::YELLOW );
  log << "this should be in yellow";
  log.setColor( MSG::RED );
  log << " and red";
  //  log.setColor(MSG::U1);
  //  log << "/n this should be in unknown";
  log << endmsg;

  log << MSG::INFO << "multicolor: ";
  log.setColor( MSG::BLUE, MSG::GREEN );
  log << "this is blue on green";
  log.setColor( MSG::PURPLE, MSG::WHITE );
  log << " and purple on white" << endmsg;

  log.setColor( MSG::GREEN );
  log << MSG::INFO << "This is green. ";
  log.setColor( MSG::YELLOW, MSG::BLUE );
  log << "This is in yellow on blue. ";
  log.resetColor();
  log << "This is back to normal" << endmsg;

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode ColorMsgAlg::finalize() { return StatusCode::SUCCESS; }
