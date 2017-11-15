#define GAUDIKERNEL_STATUSCODE_CPP 1

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IStatusCodeSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>

bool StatusCode::s_checking( false );

void StatusCode::enableChecking() { s_checking = true; }

void StatusCode::disableChecking() { s_checking = false; }

bool StatusCode::checkingEnabled() { return s_checking; }

void StatusCode::check()
{

  if ( !m_checked && !GaudiException::s_proc && !std::uncaught_exception() ) {

    auto msg = Gaudi::svcLocator()->as<IMessageSvc>();
    auto scs = Gaudi::svcLocator()->service<IStatusCodeSvc>( "StatusCodeSvc" );

    const size_t depth = 21;
    void* addresses[depth];

    std::string lib, fnc;
    void* addr = nullptr;
    /// @FIXME : (MCl) use backTrace(std::string&, const int, const int) instead
    if ( System::backTrace( addresses, depth ) ) {

      for ( size_t idx : {2, 3} ) {
        if ( System::getStackLevel( addresses[idx], addr, fnc, lib ) && fnc != "StatusCode::~StatusCode()" ) {

          if ( scs ) {
            scs->regFnc( fnc, lib );
          } else {
            MsgStream log( msg, "StatusCode" );
            log << MSG::WARNING << "Unchecked in " << fnc << " (" << lib << ")" << endmsg;
          }
          break;
        }
      }
    }
  }
}
