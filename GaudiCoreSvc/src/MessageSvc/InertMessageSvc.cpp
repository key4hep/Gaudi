// Include files

// From Gaudi
#include "GaudiKernel/SvcFactory.h"
// local
#include "InertMessageSvc.h"

// ----------------------------------------------------------------------------
// Implementation file for class: InertMessageSvc
//
// 12/02/2013: Danilo Piparo
// ----------------------------------------------------------------------------
DECLARE_SERVICE_FACTORY(InertMessageSvc);

//---------------------------------------------------------------------------

InertMessageSvc::InertMessageSvc(const std::string& name, ISvcLocator* pSvcLocator)
  : MessageSvc(name, pSvcLocator),
    m_isActive(false){
}

//---------------------------------------------------------------------------

InertMessageSvc::~InertMessageSvc() {
}

//---------------------------------------------------------------------------

StatusCode InertMessageSvc::initialize() {
  StatusCode sc = MessageSvc::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc; // error printed already by MessageSvc

  info() << "Activating in a separate thread" << endmsg;
  m_thread = std::thread (std::bind(&InertMessageSvc::m_activate,
                                    this));   

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode InertMessageSvc::InertMessageSvc::finalize() {
  
  m_deactivate();
  
  m_thread.join();
  
  return MessageSvc::finalize(); // must be called after all other actions
  
}

//---------------------------------------------------------------------------

void InertMessageSvc::m_activate(){
  m_isActive=true;
  messageActionPtr thisMessageAction;
  while (m_isActive or not m_messageActionsQueue.empty()){    
    m_messageActionsQueue.pop(thisMessageAction);
    (*thisMessageAction)();   
  }
}

//---------------------------------------------------------------------------

void InertMessageSvc::m_deactivate(){
  
  if (m_isActive){    
      // This would be the last action
      m_messageActionsQueue.push(messageActionPtr(new messageAction([this]() {m_isActive=false;})));
    }    
}

//---------------------------------------------------------------------------
/**
 * The message action is created and pushed to the message queue.
 * The message is captured by value since the one referenced by msg can 
 * (and basically always will given the asynchronous nature of the printing) 
 * go out of scope before the differed print.
 */
void InertMessageSvc::reportMessage(const Message& msg, int outputLevel) {
  m_messageActionsQueue.push(
    messageActionPtr(new messageAction([this, msg,outputLevel] () 
                                       {this->i_reportMessage(msg, outputLevel);})));
}

//---------------------------------------------------------------------------

void InertMessageSvc::reportMessage(const Message& msg) {
  m_messageActionsQueue.push(
    messageActionPtr(new messageAction([this,msg] () 
                                       {this->i_reportMessage(msg, this->outputLevel(msg.getSource()));})));
}

//---------------------------------------------------------------------------

void InertMessageSvc::reportMessage(const StatusCode& code, const std::string& source) {
  m_messageActionsQueue.push(
    messageActionPtr(new messageAction([this,code,source] () 
                                       {this->i_reportMessage(code, source);})));
}

//---------------------------------------------------------------------------
