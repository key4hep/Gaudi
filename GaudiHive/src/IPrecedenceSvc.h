#ifndef GAUDIHIVE_IPRECEDENCESVC_H_
#define GAUDIHIVE_IPRECEDENCESVC_H_

#include "GaudiKernel/IInterface.h"

#include <string>

class EventSlot;
struct Cause;

//-----------------------------------------------------------------------------

/** @class IPrecedenceSvc GaudiHive/IPrecedenceSvc.h
 *
 *  @brief Abstract interface for a service that manages tasks' precedence.
 *
 *  @author Illya Shapoval
 *  @created Aug 7, 2017
 */

//-----------------------------------------------------------------------------


class GAUDI_API  IPrecedenceSvc : virtual public IInterface {

public:
  
  /// InterfaceID
  DeclareInterfaceID(IPrecedenceSvc,1,0);

  /// Infer the precedence effect caused by an execution flow event
  virtual StatusCode iterate(EventSlot&, const Cause&) = 0;

  /// Simulate execution flow
  virtual StatusCode simulate(EventSlot&) const = 0;

  /// Check if control flow rules are resolved
  virtual bool CFRulesResolved(EventSlot&) const = 0;

  /// Get task priority
  virtual uint getPriority(const std::string&) const = 0;

  /// Check if a task is CPU-blocking
  virtual bool isBlocking(const std::string&) const = 0;

  /// Dump precedence rules
  virtual void dumpControlFlow() const = 0;
  virtual void dumpDataFlow() const = 0;
  virtual const std::string printState(EventSlot&) const = 0;

  /// Dump precedence trace
  virtual void dumpPrecedenceTrace(EventSlot&) const = 0;

};

#endif /* GAUDIHIVE_IPRECEDENCESVC_H_ */
