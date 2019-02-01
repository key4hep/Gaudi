#ifndef GAUDIKERNEL_IALGORITHM_H
#define GAUDIKERNEL_IALGORITHM_H

// Include files
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IStateful.h"
#include <string>

class IAlgTool;
class AlgResourcePool;
class AlgExecState;
class EventContext;

namespace Gaudi {
  class StringKey;
}

/** @class IAlgorithm IAlgorithm.h GaudiKernel/IAlgorithm.h

    The IAlgorithm is the interface implemented by the Algorithm base class.
    Concrete algorithms, derived from the Algorithm base class are controlled
    via this interface.

    @author Paul Maley
    @author D.Quarrie
    @author Marco Clemencic
*/
class GAUDI_API IAlgorithm : virtual public extend_interfaces<INamedInterface, IStateful> {
public:
  friend AlgResourcePool;

  /// InterfaceID
  DeclareInterfaceID( IAlgorithm, 7, 0 );

  /** The version of the algorithm
   */
  virtual const std::string& version() const = 0;

  /** The type of the algorithm
   */
  virtual const std::string& type() const                  = 0;
  virtual void               setType( const std::string& ) = 0;

  /** StringKey rep of name
   */
  virtual const Gaudi::StringKey& nameKey() const = 0;

  /** The index of the algorithm
   */
  virtual unsigned int index() const = 0;

  /** Specify if the algorithm is clonable
   */
  virtual bool isClonable() const { return false; }

  /** Cardinality (Maximum number of clones that can exist)
   *  special value 0 means that algorithm is reentrant
   */
  virtual unsigned int cardinality() const = 0;

  /** Named, non thread-safe resources used during event processing
   */
  virtual const std::vector<std::string>& neededResources() const = 0;

  /** The action to be performed by the algorithm on an event. This method is
      invoked once per event for top level algorithms by the application manager.
  */
  virtual StatusCode execute( const EventContext& ) const = 0;

  /// check if the algorithm is initialized properly
  virtual bool isInitialized() const = 0;
  /// check if the algorithm is finalized properly
  virtual bool isFinalized() const = 0;

  /** Initialization method invoked by the framework. This method is responsible
      for any bookkeeping of initialization required by the framework itself.
      It will in turn invoke the initialize() method of the derived algorithm,
      and of any sub-algorithms which it creates.
  */
  virtual StatusCode sysInitialize() = 0;

  /** Startup method invoked by the framework. This method is responsible
      for any bookkeeping of initialization required by the framework itself.
      It will in turn invoke the start() method of the derived algorithm,
      and of any sub-algorithms which it creates.
  */
  virtual StatusCode sysStart() = 0;

  /** Re-initialization method invoked by the framework. This method is responsible
      for any re-initialization required by the framework itself.
      It will in turn invoke the reinitialize() method of the derived algorithm,
      and of any sub-algorithms which it creates.
  */
  virtual StatusCode sysReinitialize() = 0;

  /** Re-start method invoked by the framework. This method is responsible
      for any re-start required by the framework itself.
      It will in turn invoke the restart() method of the derived algorithm,
      and of any sub-algorithms which it creates.
  */
  virtual StatusCode sysRestart() = 0;

/// sysExecute changed to accept the EventContext as argument
#define GAUDI_SYSEXECUTE_WITHCONTEXT 1

  /// System execution. This method invokes the execute() method of a concrete algorithm
  virtual StatusCode sysExecute( const EventContext& ) = 0;

  /** System stop. This method invokes the stop() method of a concrete
      algorithm and the stop() methods of all of that algorithm's sub algorithms.
  */
  virtual StatusCode sysStop() = 0;

  /** System finalization. This method invokes the finalize() method of a concrete
      algorithm and the finalize() methods of all of that algorithm's sub algorithms.
  */
  virtual StatusCode sysFinalize() = 0;

  /** beginRun method invoked by the framework. This method is responsible
      for any beginRun actions required by the framework itself.
      It will in turn invoke the beginRun() method of the derived algorithm,
      and of any sub-algorithms which it creates.
  */
  virtual StatusCode sysBeginRun() = 0;

  /** endRun method invoked by the framework. This method is responsible
      for any endRun actions required by the framework itself.
      It will in turn invoke the endRun() method of the derived algorithm,
      and of any sub-algorithms which it creates.
  */
  virtual StatusCode sysEndRun() = 0;

  /** Algorithm begin run. This method is called at the beginning of the event loop.
   */
  virtual StatusCode beginRun() = 0;
  /** Algorithm end run. This method is called at the end of the event loop.
   */
  virtual StatusCode endRun() = 0;

  /// reference to AlgExecState of Alg
  virtual AlgExecState& execState( const EventContext& ctx ) const = 0;

  /// Is this algorithm enabled or disabled?
  virtual bool isEnabled() const = 0;

  /// Are we a Sequence?
  virtual bool isSequence() const = 0;

  /// Produce string represention of the control flow expression.
  virtual std::ostream& toControlFlowExpression( std::ostream& os ) const = 0;

  /// Set instantiation index of Alg
  virtual void setIndex( const unsigned int& idx ) = 0;

  virtual bool isReEntrant() const = 0;
};

#endif // GAUDIKERNEL_IALGORITHM_H
