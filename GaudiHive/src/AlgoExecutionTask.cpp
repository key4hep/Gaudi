#include "AlgoExecutionTask.h"
#include "GaudiKernel/Algorithm.h"
#include <functional>

tbb::task* AlgoExecutionTask::execute() {

  Algorithm* this_algo = dynamic_cast<Algorithm*>(m_algorithm.get());  
   if (!this_algo){
     throw GaudiException ("Cast to Algorithm failed!","AlgoExecutionTask",StatusCode::FAILURE);
   }

  EventContext* eventContext = this_algo->getContext();

  eventContext->m_thread_id = pthread_self();

  m_algorithm->sysExecute();

  // TODO: Here the code from the algo is to be analysed for the Control Flow

  // TODO: call a scheduler function..
  // Push in the scheduler queue an action to be performed 
  auto action_promote2Executed = std::bind(&ForwardSchedulerSvc::m_promoteToExecuted, 
                                           m_schedSvc, 
                                           m_algoIndex, 
                                           eventContext->m_evt_slot,
                                           m_algorithm);

  m_schedSvc->m_actionsQueue.push(action_promote2Executed);    

  return nullptr;
}
