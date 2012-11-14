#ifndef GAUDIKERNEL_EVENTCONTEXT_H
#define GAUDIKERNEL_EVENTCONTEXT_H

#include <memory>

/** @class EventContext EventContext.h GaudiKernel/EventContext.h
 *
 * This class represents an entry point to all the event specific data.
 * It is needed to make the algorithm "aware" of the event it is operating on.
 * This was not needed in the serial version of Gaudi where the assumption
 * of 1-event-at-the-time processing was implicit.
 *
 * This class has nothing to do with the AlgContextSvc!
 *
 * @author Danilo Piparo
 * @date 2012
 **/

class EventContext;

//fwd declaration
namespace DataSvcHelpers {
class RegistryEntry;
}

// fast implementation, to be better organised.
class EventContext{
public:
	long int m_evt_num;
	unsigned long int m_thread_id;
  DataSvcHelpers::RegistryEntry* m_registry;
	// type is vague on purpose.
	void* m_event_data_provider_svc;
	void* m_event_data_conversion_svc;
	void* m_detector_data_provider_svc;
	void* m_detector_data_conversion_svc;
	void* m_histogram_svc; // might not be needed if we schedule correctly. It depends on DQM design
	void* m_chrono_stat_svc; // can we make it ts?
	void* m_exception_svc; // can we make it ts?
	void* m_auditor_svc; // can we make it ts?
	void* m_tools_svc; // hic sunt leones
	void* m_pMonitor_svc; // hic sunt leones
};

#endif //GAUDIKERNEL_EVENTCONTEXT_H
