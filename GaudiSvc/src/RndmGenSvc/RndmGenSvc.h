//====================================================================
//	Random Generator service definition
//--------------------------------------------------------------------
//
//	Package    : Gaudi/RndmGen ( The LHCb Offline System)
//	Author     : M.Frank
//====================================================================
#ifndef GAUDI_RANDOMGENSVC_RNDMGENSVC_H
#define GAUDI_RANDOMGENSVC_RNDMGENSVC_H 1

// STL include files
#include <vector>

// Framework include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/ISerialize.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmEngine.h"
#include "GaudiKernel/IRndmGenSvc.h"

// Forward declarations
class IRndmGenFactory;
class IMessageSvc;

/** Random Generator service definition

    Description:
    Definition of a interface for a service to access
    random generators according to predefined distributions.
    For more detailed explanations please see the interface definition.

    Dependencies:
    <UL>
    <LI> Base class: Gaudi/RndmGenSvc/ISerialize.h
    <LI> Base class: Gaudi/RndmGenSvc/IRandomGenSvc.h
    </UL>

    <P> History    :
    <PRE>
    +---------+----------------------------------------------+--------+
    |    Date |                 Comment                      | Who    |
    +---------+----------------------------------------------+--------+
    | 21/04/99| Initial version.                             | MF     |
    +---------+----------------------------------------------+--------+
    </PRE>
    Author:  M.Frank
    Version: 1.0
*/
class RndmGenSvc : public extends3<Service, IRndmGenSvc, IRndmEngine, ISerialize> {
private:
  /// Random number engine
  mutable SmartIF<IRndmEngine> m_engine;
  /// Serialization interface of random number engine
  mutable SmartIF<ISerialize>  m_serialize;
  /// Engine name
  std::string          m_engineName;
public:
  /// Standard Service constructor
  RndmGenSvc(const std::string& name, ISvcLocator* svc);
  /// Standard Service destructor
  ~RndmGenSvc() override = default;
public:
  /// Service override: initialization
  StatusCode initialize() override;
  /// Service override: finalization
  StatusCode finalize() override;
  /** IRndmGenSvc interface implementation  */
  /// Input serialization from stream buffer. Restores the status of the generator engine.
  StreamBuffer& serialize(StreamBuffer& str) override;
  /// Output serialization to stream buffer. Saves the status of the generator engine.
  StreamBuffer& serialize(StreamBuffer& str) const override;
  /// Retrieve engine
  IRndmEngine* engine() override;
  /// Retrieve a valid generator from the service.
  StatusCode generator(const IRndmGen::Param& par, IRndmGen*& refpGen) override;
  /// Single shot returning single random number
  double rndm() const override;
  /** Multiple shots returning vector with flat random numbers.
      @param  array    Array containing random numbers
      @param  howmany  fill 'howmany' random numbers into array
      @param  start    ... starting at position start
      @return StatusCode indicating failure or success.
  */
  StatusCode rndmArray( std::vector<double>& array, long howmany, long start = 0) const override;
  /// Allow to set new seeds
  StatusCode setSeeds(const std::vector<long>& seeds) override;
  /// Allow to get seeds
  StatusCode seeds(std::vector<long>& seeds)  const override;
};

#endif // GAUDI_RANDOMGENSVC_RNDMGENSVC_H
