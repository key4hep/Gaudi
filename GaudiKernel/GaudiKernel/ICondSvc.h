#ifndef GAUDIKERNEL_ICONDSVC_H
#define GAUDIKERNEL_ICONDSVC_H 1

#include "GaudiKernel/IService.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/EventContext.h"

#include <string>
#include <sstream>
#include <set>

/**@class ICondSvc ICondSvc.h GaudiKernel/ICondSvc.h
 *
 *  Interface for the Condition Service. It allows Algorithms to register
 *  their ConditionsHanaldes with the Service, and clients to query the
 *  Service to determine which objects are valid/invalid for a particualar
 *  EventContext.
 * 
 *  @author  Charles Leggett
 *  @version 2.0
 */

class ConditionSlotFuture;
class IConditionIOSvc;

class GAUDI_API ICondSvc: virtual public IService {
public:
  DeclareInterfaceID(ICondSvc, 1, 0);

  virtual ~ICondSvc() = default;

  /// register dependency between an Algorithm and the Condition Handle it creates
  /// dbKey is an optional key to further identify the object in the backend dB
  virtual StatusCode regHandle(IAlgorithm* alg, const Gaudi::DataHandle& id) = 0;

  /// check to see if a specific condition object ID is valid
  virtual bool isValidID(const EventContext& ctx, const DataObjID& id) const = 0;

  /// get list of all registered condition Algorithms
  virtual const std::set<IAlgorithm*>& condAlgs() const = 0;

  /// query if a specific Algorithm is a condition Algorithm
  virtual bool isRegistered(IAlgorithm* alg) const = 0;

  /// query if a condition Object ID is registered
  virtual bool isRegistered(const DataObjID& id) const = 0;

  /// get collection of all registered condition Object IDs
  virtual const DataObjIDColl& conditionIDs() const = 0;

  /// dump the condition store
  virtual void dump(std::ostringstream& ost) const = 0;


  /// Asynchronously setup conditions
  virtual ConditionSlotFuture* startConditionSetup(const EventContext& ctx) = 0;

  /// register an IConditionIOSvc (alternative to Algorithm processing of 
  /// Conditions)
  virtual StatusCode registerConditionIOSvc(IConditionIOSvc *ioService) = 0;


};

#endif
