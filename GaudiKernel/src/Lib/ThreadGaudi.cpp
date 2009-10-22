// -*-C++-*-
// $Id: ThreadGaudi.cpp,v 1.4 2006/04/20 16:42:58 hmd Exp $
// $Name:  $

#include "GaudiKernel/ThreadGaudi.h"

#include <sstream>

#include <iostream>

const std::string threadSeparator = "__" ;
const std::string algToolSeparator = "." ;

//
//   getGaudiThreadIDfromName
//   ------------------------

std::string getGaudiThreadIDfromID(int iCopy) {
  std::ostringstream ost;
  ost << threadSeparator << iCopy << std::ends;
  return ost.str();
}

//
//   getGaudiThreadIDfromName
//   ------------------------

std::string getGaudiThreadIDfromName(const std::string& name) {
  std::string threadAppendix = "" ;

  // find parent if name of an AlgTool
  std::string parent_name = name ;
  std::string tool_name  = "" ;
  size_t pp = name.find(".") ;
  if ( (pp > 0) && (pp <= name.length()) ) {
    parent_name = name.substr(0,name.find(".")-1) ;
    tool_name = name.substr(name.find("."));
  }

  // get from (parent_)name thread ID
  pp = parent_name.find(threadSeparator) ;
  if ( (pp > 0) && (pp <= parent_name.length()) ) {
    threadAppendix = parent_name.substr(parent_name.find(threadSeparator)) ;
  }
  return threadAppendix ;
}

//
//   getGaudiThreadGenericName
//   -------------------------

std::string getGaudiThreadGenericName(const std::string& name) {
  std::string genericName = name ;

  // find parent if name of an AlgTool
  std::string parent_name = name ;
  std::string tool_name  = "" ;
  size_t pp = name.find(".") ;
  if ( (pp > 0) && (pp <= name.length()) ) {
    parent_name = name.substr(0,name.find(".")-1) ;
    tool_name = name.substr(name.find("."));
  }

  // construct gneric name
  pp = parent_name.find(threadSeparator) ;
  if ( (pp > 0) && (pp <= parent_name.length()) ) {
    genericName = parent_name.substr(0,parent_name.find(threadSeparator))+tool_name ;
  }
  return genericName ;
}

//
//  isGaudiThreaded
//
bool isGaudiThreaded(const std::string& name) {
  return  (!(getGaudiThreadIDfromName(name).empty()));
}

//
//   ThreadGaudi
//   -----------

ThreadGaudi* ThreadGaudiInstance = 0 ;

ThreadGaudi::ThreadGaudi() {
  m_threadMap = new ThreadMap();
  (*m_threadMap)[0] = "" ;
}

ThreadGaudi::~ThreadGaudi() {
  delete m_threadMap;
}

ThreadGaudi* ThreadGaudi::instance() {
  if ( 0 == ThreadGaudiInstance ) {
    ThreadGaudiInstance = new ThreadGaudi();
  }
  return ThreadGaudiInstance ;
}

void ThreadGaudi::setThreadID(const std::string& threadID) {
  ThreadMap* p_threadMap = getThreadMap() ;
  // get from name thread ID
  System::ThreadHandle s_pid = System::threadSelf() ;
  if (p_threadMap->count(s_pid) == 0 ) {
    (*p_threadMap)[s_pid] = threadID ;
#ifdef THREAD_GAUDI__DEBUG
    std::cout << " *** ThreadGaudi setThreadID *** value set for *** " << threadID << " thread ID : " << s_pid << std::endl ;
#endif    
  }
}

ThreadGaudi::ThreadMap* ThreadGaudi::getThreadMap() {
  return ThreadGaudi::instance()->m_threadMap;
}   

const std::string& ThreadGaudi::getThreadID() {
  ThreadMap* p_threadMap = getThreadMap() ;
  System::ThreadHandle s_pid = System::threadSelf() ;
  if ( p_threadMap->find(s_pid) != p_threadMap->end() ) {
    return (*p_threadMap->find(s_pid)).second ;
  } else {
    return (*p_threadMap->find(0)).second ; ;
  }
}
