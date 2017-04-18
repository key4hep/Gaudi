#ifndef GAUDIKERNEL_ICONDSVC_H
#define GAUDIKERNEL_ICONDSVC_H 1

#include "GaudiKernel/IService.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/EventIDRange.h"

#include <string>
#include <set>


class GAUDI_API ICondSvc: virtual public IService {
public:
  DeclareInterfaceID(ICondSvc, 1, 0);

  typedef float dbData_t;

  virtual ~ICondSvc() = default;

  virtual StatusCode regHandle(IAlgorithm* alg, const Gaudi::DataHandle& id, 
                               const std::string& key) = 0;

  virtual bool getInvalidIDs(const EventContext& ctx, DataObjIDColl& ids) = 0;
  virtual bool getValidIDs(const EventContext& ctx, DataObjIDColl& ids) = 0;
  virtual bool getIDValidity(const EventContext& ctx, DataObjIDColl& validIDs,
                             DataObjIDColl& invalidIDs) = 0;

  virtual bool isValidID(const EventContext& ctx, const DataObjID& id) const = 0;

  // virtual std::set<std::string> getUnchangedAlgs( const DataObjIDColl& ids) = 0;
  // virtual StatusCode getRange(const std::string&, const EventContext& ctx, 
  //                             EventIDRange& rng, dbData_t&) const = 0;

  virtual const std::set<IAlgorithm*>& condAlgs() const = 0;

  virtual bool isRegistered(const DataObjID& id) const = 0;
  virtual const DataObjIDColl& conditionIDs() const = 0;

  virtual void dump() const = 0;

};

#endif
