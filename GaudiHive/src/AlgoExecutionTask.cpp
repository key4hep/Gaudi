#include "AlgoExecutionTask.h"
#include "GaudiKernel/Algorithm.h"
#include <functional>

tbb::task* AlgoExecutionTask::execute() {
  Algorithm* this_algo = dynamic_cast<Algorithm*>(m_algorithm);
  EventContext* eventContext = this_algo->getContext();
  
  eventContext->m_thread_id = pthread_self();
  
  m_algorithm->sysExecute();

  // TODO: Here the code from the algo is to be analysed for the Control Flow
  
  // Push in the scheduler queue an action to be performed 
  m_schedSvc->m_algosInFlight--; //This is atomic, no worries.
  
  auto promote2Finished = std::bind(&SchedulerSvc::m_promoteToFinished, 
                                    m_schedSvc, 
                                    m_algoIndex, 
                                    eventContext->m_evt_slot);  
  
  m_schedSvc->m_actions_queue.push(promote2Finished); // This container is ts, no worries.
  
  return nullptr;
}
