#include "ThreadInitTask.h"
#include <thread>
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IThreadInitTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IMessageSvc.h"

std::atomic<bool> ThreadInitTask::m_execFailed(false);

tbb::task*
ThreadInitTask::execute() {

  SmartIF<IMessageSvc> messageSvc (m_serviceLocator);
  MsgStream log(messageSvc, "ThreadInitTask");

  log << MSG::DEBUG << "execute in thread 0x" << std::hex << pthread_self()
      << " at " << this
      << std::dec << endmsg;

  // if (m_tools.retrieve().isFailure()) {
  //   log << MSG::ERROR << "unable to retrieve ToolHandleArray " << m_tools
  // 	<< endmsg;
  //   m_execFailed = true;
  // } else {

  //   if (m_tools.size() == 0) {
  //     log << MSG::INFO << "no entries in Tool Array"
  // 	<< endmsg;
  //   } else {

  //     log << MSG::DEBUG << "executing in thread 0x" << std::hex << pthread_self()
  // 	  << std::dec << endmsg;      

  //   for ( auto t : m_tools ) {

  // 	try {
    
  // 	log << MSG::DEBUG << "calling IThreadInitTool " << t << endmsg;
  // 	if (!m_terminate) {
  // 	  t->initThread();
  //     } else {
  // 	  t->terminateThread();
  //     }

  // 	} catch( const GaudiException& exc ) {
  // 	  log << MSG::ERROR << "ThreadInitTool " << t << " in thread 0x"
  // 	      << std::hex << pthread_self() << std::dec << " threw GaudiException: " 
  // 	      << exc << endmsg;
  // 	  m_execFailed = true;
  // 	} catch (const std::exception& exc ) {
  // 	  log << MSG::ERROR << "ThreadInitTool " << t << " in thread 0x"
  // 	      << std::hex << pthread_self() << std::dec << " threw std::exception: " 
  // 	      << exc.what() << endmsg;
  // 	  m_execFailed = true;
  // 	} catch(...) {
  // 	  log << MSG::ERROR << "ThreadInitTool " << t << " in thread 0x"
  // 	      << std::hex << pthread_self() << std::dec << " threw unknown exception" 
  // 	      << endmsg;
  // 	  m_execFailed = true;
  // 	}

  //   }

  //     m_tools.release().ignore();            
    
  //   }
  // }


  if (m_barrier != 0) {
    log << MSG::INFO << "waiting at barrier in thread 0x" << std::hex << pthread_self()
	<< std::dec << endmsg;
    m_barrier->wait();
  }

  return nullptr;

}
