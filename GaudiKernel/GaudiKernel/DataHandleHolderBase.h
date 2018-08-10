#ifndef GAUDIKERNEL_DATAHANDLEHOLDERBASE
#define GAUDIKERNEL_DATAHANDLEHOLDERBASE 1

#include <algorithm>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include <boost/optional.hpp>

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/StateMachine.h"

/// Common infrastructure for classes that manage data handles and other
/// Scheduler-known data dependencies
///
/// This template is intended to be used as a common base class of Algorithms
/// and AlgTools. It factors out the bookkeeping associated with DataHandles and
/// other explicitly declared data dependencies.
///
template <typename BASE>
class GAUDI_API DataHandleHolderBase : public extends<BASE, IDataHandleHolder>
{
  using Super = extends<BASE, IDataHandleHolder>;

public:
  /// NOTE: Cannot use "using Super::Super;" due to a GCC 6 bug
  template <typename... Args>
  DataHandleHolderBase( Args&&... args ) : Super( std::forward<Args>( args )... )
  {
  }

  /// Register a data handle as an event data input of the algorithm
  ///
  /// You should not need to call this method manually, as it is
  /// automatically called by the DataHandle constructor.
  ///
  void registerInput( Gaudi::v2::DataHandle& handle ) final override
  {
    if ( m_explicitDepsCollected ) {
      throw GaudiException( "Cannot register input after data dependency collection", this->name(), StatusCode::FAILURE );
    }
    m_inputHandles.push_back( &handle );
  }

  /// Register a data handle as an event data output of the algorithm
  ///
  /// You should not need to call this method manually, as it is
  /// automatically called by the DataHandle constructor.
  ///
  void registerOutput( Gaudi::v2::DataHandle& handle ) final override
  {
    if ( m_explicitDepsCollected ) {
      throw GaudiException( "Cannot register output after data dependency collection", this->name(), StatusCode::FAILURE );
    }
    m_outputHandles.push_back( &handle );
  }

  /// Add an event data input dynamically at run time
  ///
  /// DataHandles are the preferred way to declare statically known data
  /// dependencies. However, there are cases in which an Algorithm's data
  /// dependencies are only known at run time, typically when data is loaded
  /// on demand from a file or database. In this case, this method should be
  /// used to declare those dynamic data dependencies.
  ///
  void addDynamicInput( const DataObjID& key ) final override { m_inputKeys.insert( key ); }

  /// Add an event data output dynamically at run time
  ///
  /// See addDynamicInput() for more details.
  ///
  void addDynamicOutput( const DataObjID& key ) final override { m_outputKeys.insert( key ); }

  /// Tell which event store keys the algorithm will be reading from
  ///
  /// This function will only yield the full dependency list after it has
  /// been collected, which happens during initialization.
  ///
  const DataObjIDColl& inputKeys() const final override
  {
    if ( !m_explicitDepsCollected ) {
      throw GaudiException( "Cannot read inputs before data dependency collection", this->name(), StatusCode::FAILURE );
    }
    return m_inputKeys;
  }

  /// Tell which event store keys the algorithm will be writing to
  ///
  /// The interface caveats described in inputKeys() also apply here.
  ///
  const DataObjIDColl& outputKeys() const final override
  {
    if ( !m_explicitDepsCollected ) {
      throw GaudiException( "Cannot read outputs before data dependency collection", this->name(), StatusCode::FAILURE );
    }
    return m_outputKeys;
  }

  /// Declare ownership of a legacy DataHandle
  void declare( Gaudi::v1::DataHandle& handle ) override
  {
    if ( m_explicitDepsCollected ) {
      throw GaudiException( "Cannot register legacy handle after data dependency collection", this->name(), StatusCode::FAILURE );
    }

    if ( !handle.owner() ) {
      handle.setOwner( this );
    } else if ( handle.owner() != this ) {
      throw GaudiException( "Attempt to declare foreign handle with algorithm!", this->name(), StatusCode::FAILURE );
    }

    m_legacyHandles.insert( &handle );
  }

  /// Discard ownership of a legacy DataHandle
  void renounce( Gaudi::v1::DataHandle& handle ) override
  {
    // In general, deleting a DataHandle after the data dependencies have been
    // collected is bogus, because this change will not propagate to the data
    // dependency list, and to our clients that have queried and potentially
    // made copies of this list, such as the Scheduler.
    //
    // Nevertheless, we must tolerate this operation after an Algorithm or
    // AlgTool has been finalized, because its inner DataHandles will then be
    // deleted in a harmless and non-observable fashion, right before
    // termination of the host Gaudi process.
    //
    if ( m_explicitDepsCollected && dynamic_cast<IStateful*>(this)->FSMState() >= Gaudi::StateMachine::INITIALIZED ) {
      throw GaudiException( "Cannot discard legacy handle after data dependency collection", this->name(), StatusCode::FAILURE );
    }

    if ( handle.owner() != this ) {
      throw GaudiException( "Attempt to renounce foreign handle with algorithm!", this->name(), StatusCode::FAILURE );
    }

    m_legacyHandles.erase( &handle );
  }

protected:
  /// DataObjID mapping function, with optimized identity mapping case
  ///
  /// This construct is used when updating the DataObjIDs associated with
  /// data dependencies, which is needed for some legacy workarounds.
  ///
  /// The mapping function shall return boost::none when no change is
  /// required, and the modified DataObjID otherwise. This design
  /// optimizes the "no change" case, which is expected to be the norm.
  ///
  using DataObjIDMapping = std::function<boost::optional<DataObjID>( const DataObjID& )>;

  /// Update the key of each registered event data dependency, using a
  /// user-defined mapping from the old to the new key.
  ///
  /// This method must be called at a framework initialization stage
  /// where the DataObjIDs associated with data dependencies are
  /// guaranteed not to change anymore. Otherwise the DataObjIDs changes
  /// brought by this update will be overwritten.
  ///
  /// It should also be called before the explicit dependencies are
  /// collected by collectExplicitDeps(), as otherwise the DataObjIDs
  /// modifications will be carried out in two different places, resulting
  /// in worse performance.
  ///
  void updateEventKeys( const DataObjIDMapping& keyMap )
  {
    auto legacyInputs  = legacyInputHandles();
    auto legacyOutputs = legacyOutputHandles();
    updateKeys( legacyInputs, keyMap );
    updateKeys( legacyOutputs, keyMap );
    updateKeys( m_inputKeys, keyMap );
    updateKeys( m_outputKeys, keyMap );
    updateKeys( m_inputHandles, keyMap );
    updateKeys( m_outputHandles, keyMap );
    updateKeys( m_extraInputs.value(), keyMap );
    updateKeys( m_extraOutputs.value(), keyMap );
  }

  /// Collect all explicit data dependencies in a single place
  ///
  /// Like updateEventKeys(), this method must be called at a framework
  /// initialization stage where the DataObjIDs associated with data
  /// dependencies are set in stone and will not change anymore.
  ///
  /// For optimal performance, it should also be called after any call
  /// to updateEventKeys (see that method's documentation for more info).
  ///
  void collectExplicitDeps()
  {
    // Considering this function's interface contract, calling it multiple
    // times is very likely to be the result of a usage error.
    if ( m_explicitDepsCollected ) {
      throw GaudiException( "Explicit dependencies may only be collected once", this->name(), StatusCode::FAILURE );
    }

    // Collect all input dependencies in a single place
    extractInputs( m_inputHandles );
    extractInputs( legacyInputHandles() );
    extractInputs( m_extraInputs.value() );

    // Collect all output dependencies in a single place
    extractOutputs( m_outputHandles );
    extractOutputs( legacyOutputHandles() );
    extractOutputs( m_extraOutputs.value() );

    // Take note that the explicit dependencies have been collected
    m_explicitDepsCollected = true;
  }

  /// A circular dependency handling action
  ///
  /// A Gaudi Algorithm with circular dependencies is not schedulable: the
  /// Scheduler will not run it until all of its data dependencies are
  /// present, but it must be run for at least one of its dependencies to
  /// be created. As a result of this deadlock, the Scheduler will stall.
  ///
  /// Most circular dependencies are the result of a bug, in which case
  /// aborting framework initialization is the right course of action. But
  /// some circular dependencies accidentally emerge as a result of Algs
  /// having inner structure (e.g. calling multiple AlgTools/SubAlgs which
  /// generate each other's inputs). In this latter case, silently
  /// breaking up the circularity by deleting the conflicting input
  /// dependency may be the right course of action.
  ///
  /// The user-specified circular dependency handler will notify the
  /// DataHandleHolder of its decision with this enum.
  ///
  enum class CircularDepAction {
    /// The framework cannot recover from this circular dependency and will
    /// shut down, iteration should be terminated.
    Abort,

    /// The circular dependency is assumed to be a false alarm (e.g. data
    /// dependencies from inner AlgTools) and the corresponding input
    /// dependency can be deleted to avoid scheduler stalls.
    Ignore
  };

  /// A circular dependency handler, which can take any user-specified
  /// step (e.g. logging), then produces a dependency handling action.
  ///
  /// See CircularDepAction documentation for more information.
  ///
  using CircularDepHandler = std::function<CircularDepAction( const DataObjID& )>;

  /// Look for circular dependencies and let a user-specified handler deal
  /// with each of them.
  ///
  /// This method must be called after explicit dependencies have been
  /// collected, as otherwise it may miss some circular dependencies.
  ///
  StatusCode handleCircularDeps( CircularDepHandler&& circularDepHandler )
  {
    // Make sure that explicit dependencies have been collected beforehand
    if ( !m_explicitDepsCollected ) {
      throw GaudiException( "Explicit dependencies must be collected first", this->name(), StatusCode::FAILURE );
    }

    // Go through the output dependencies, looking for circular input
    // dependencies
    //
    // We must use this inefficient iteration pattern because circular
    // dependency handling may mutate the input dependency list,
    // invalidating any iterator to that list.
    //
    for ( const auto& outputKey : m_outputKeys ) {
      auto inputPos = m_inputKeys.find( outputKey );
      if ( inputPos != m_inputKeys.end() ) {
        // Call the user circular dependency handler on each dependency
        switch ( circularDepHandler( *inputPos ) ) {
        // Abort iteration if asked to do so
        case CircularDepAction::Abort:
          return StatusCode::FAILURE;

        // Break the circular dependency otherwise
        case CircularDepAction::Ignore:
          m_inputKeys.erase( inputPos );
        }
      }
    }

    // If control reached this point, we are fine
    return StatusCode::SUCCESS;
  }

  /// Add the dependencies of another DataHandleHolder to our dependency list
  ///
  /// This method is used to collect the implicit data dependencies that
  /// appear when an Algorithm holds inner Tools and SubAlgs, which have
  /// data dependencies themselves. We need this so that the Scheduler
  /// can see those "inner" dependencies.
  ///
  /// This method must be called after the dependencies of the child
  /// Tool/Algorithm have been collected. This includes implicit
  /// dependencies, if applicable.
  ///
  void addImplicitDeps( const IDataHandleHolder* child )
  {
    if ( !child ) return;
    extractInputs( child->inputKeys() );
    extractOutputs( child->outputKeys() );
  }

  /// Tell which input dependencies have been ignored due to an empty key
  ///
  /// The interface caveats described in inputKeys() also apply here.
  ///
  const DataObjIDColl& ignoredInputs() const
  {
    if ( !m_explicitDepsCollected ) {
      throw GaudiException( "Cannot read ignored inputs before data dependency collection", this->name(), StatusCode::FAILURE );
    }
    return m_ignoredInputs;
  }

  /// Tell which output dependencies have been ignored due to an empty key
  ///
  /// The interface caveats described in inputKeys() also apply here.
  ///
  const DataObjIDColl& ignoredOutputs() const
  {
    if ( !m_explicitDepsCollected ) {
      throw GaudiException( "Cannot read ignored outputs before data dependency collection", this->name(), StatusCode::FAILURE );
    }
    return m_ignoredOutputs;
  }

  /// Initialize the DataHandles
  ///
  /// This operation accesses framework services and must therefore be run
  /// after the host Algorithm or AlgTool has been sysInitialized.
  ///
  void initDataHandleHolder()
  {
    for ( auto h : m_legacyHandles ) h->init();
    initializeHandles( m_inputHandles );
    initializeHandles( m_outputHandles );
  }

private:
  // Data handles associated with input and output event data
  using DataHandleList = std::vector<Gaudi::v2::DataHandle*>;
  DataHandleList m_inputHandles, m_outputHandles;

  // Legacy DataHandles
  std::unordered_set<Gaudi::v1::DataHandle*> m_legacyHandles;

  // Properties allowing extra input and output dependencies to be
  // declared at configuration time
  Gaudi::Property<DataObjIDColl> m_extraInputs{this, "ExtraInputs", DataObjIDColl{}};
  Gaudi::Property<DataObjIDColl> m_extraOutputs{this, "ExtraOutputs", DataObjIDColl{}};

  // Location where all event store dependencies will be eventually collected
  DataObjIDColl m_inputKeys, m_outputKeys;

  /// Truth that all explicit data dependencies have already been
  /// collected in a single list.
  bool m_explicitDepsCollected = false;

  // Places where we keep track of the inputs and outputs that were
  // ignored because they had an empty key
  DataObjIDColl m_ignoredInputs, m_ignoredOutputs;

  /// Initialize a set of handles
  void initializeHandles( DataHandleList& handles )
  {
    for ( auto handlePtr : handles ) {
      handlePtr->initialize( *this );
    }
  }

  /// Access the key of a DataHandle
  static const DataObjID& accessKey( const Gaudi::v2::DataHandle* handlePtr ) { return handlePtr->targetKey(); }

  /// Access the key of a DataObjID (identity function)
  static const DataObjID& accessKey( const DataObjID& key ) { return key; }

  /// Access the key of a legacy (non-reentrant) DataHandle
  static const DataObjID& accessKey( const Gaudi::v1::DataHandle* handlePtr ) { return handlePtr->fullKey(); }

  /// Extract non-empty DataObjID keys from a collection of key holders
  ///
  /// Given an output key collection and a collection of objects that
  /// have an associated DataObjID key which can be accessed via an
  /// overload of accessKey(), extract the keys, filter out empty ones,
  /// and store the rest in the output collection.
  ///
  template <typename KeyHolderColl>
  static void extractKeys( DataObjIDColl& validOutput, DataObjIDColl& ignoredOutput, const KeyHolderColl& keyHolders )
  {
    validOutput.reserve( keyHolders.size() );
    for ( const auto& holder : keyHolders ) {
      const DataObjID& key = accessKey( holder );
      if ( !key.empty() ) {
        validOutput.insert( key );
      } else {
        ignoredOutput.insert( key );
      }
    }
  }

  /// Specialization of extractKeys for event data inputs
  template <typename KeyHolderColl>
  void extractInputs( const KeyHolderColl& keyHolders )
  {
    extractKeys( m_inputKeys, m_ignoredInputs, keyHolders );
  }

  /// Specialization of extractKeys for event data outputs
  template <typename KeyHolderColl>
  void extractOutputs( const KeyHolderColl& keyHolders )
  {
    extractKeys( m_outputKeys, m_ignoredOutputs, keyHolders );
  }

  /// Update the key of a DataHandle
  static void updateKey( Gaudi::v2::DataHandle* target, DataObjID&& key ) { target->setTargetKey( std::move( key ) ); }

  /// Update the key of a DataObjID
  static void updateKey( DataObjID& target, DataObjID&& key ) { target = std::move( key ); }

  /// Update the key of a legacy (non-reentrant) DataHandle
  static void updateKey( Gaudi::v1::DataHandle* target, DataObjID&& key ) { target->setKey( std::move( key ) ); }

  /// Update the DataObjID keys of a collection of key holders
  ///
  /// Access the key from each key holder using accessKey, pass it
  /// through a user-defined transform, and if the result is different
  /// from the original key update the key using updateKey.
  ///
  template <typename KeyHolderColl>
  static void updateKeys( KeyHolderColl& keyHolders, const DataObjIDMapping& keyMap )
  {
    for ( auto& holder : keyHolders ) {
      const DataObjID& oldKey    = accessKey( holder );
      auto             mappedKey = keyMap( oldKey );
      if ( mappedKey ) {
        updateKey( holder, *std::move( mappedKey ) );
      }
    }
  }

  /// Specialization of the above algorithm for DataObjIDColls, where
  /// updating the keys in place is not possible.
  static void updateKeys( DataObjIDColl& keys, const DataObjIDMapping& keyMap )
  {
    DataObjIDColl result;
    result.reserve( keys.size() );
    for ( const auto& key : keys ) {
      result.emplace( keyMap( key ).value_or( key ) );
    }
    keys = std::move( result );
  }

  /// Select the legacy data handles by operating mode (input/output)
  std::vector<Gaudi::v1::DataHandle*> selectLegacyHandles( Gaudi::v1::DataHandle::Mode mode ) const
  {
    std::vector<Gaudi::v1::DataHandle*> result;
    std::copy_if( std::begin( m_legacyHandles ), std::end( m_legacyHandles ), std::back_inserter( result ),
                  [&]( const Gaudi::v1::DataHandle* hndl ) -> bool { return hndl->mode() & mode; } );
    return result;
  }

  /// Enumerate legacy input handles
  std::vector<Gaudi::v1::DataHandle*> legacyInputHandles() const
  {
    return selectLegacyHandles( Gaudi::v1::DataHandle::Reader );
  }

  /// Enumerate legacy output handles
  std::vector<Gaudi::v1::DataHandle*> legacyOutputHandles() const
  {
    return selectLegacyHandles( Gaudi::v1::DataHandle::Writer );
  }
};

#endif // !GAUDIKERNEL_DATAHANDLEHOLDERBASE
