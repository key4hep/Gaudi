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
  using AccessMode = Gaudi::v2::DataHandle::AccessMode;

public:
  /// NOTE: Cannot use "using Super::Super;" due to a GCC 6 bug
  template <typename... Args>
  DataHandleHolderBase( Args&&... args ) : Super( std::forward<Args>( args )... )
  {
  }

  /// Register a data handle of this algorithm/tool
  ///
  /// You should not need to call this method manually, as it is
  /// automatically called by the DataHandle constructor.
  ///
  void registerDataHandle( Gaudi::v2::DataHandle& handle ) final override
  {
    if ( m_explicitDepsCollected && isNewDataDependency( handle ) ) {
      throw GaudiException( "Cannot register a new data handle after data dependency collection", this->name(), StatusCode::FAILURE );
    }
    auto index = accessToIndex( handle.access() );
    m_dataHandles[index].push_back( &handle );
  }

  /// Add a data dependency, even after initialization
  ///
  /// DataHandles are the preferred way to declare statically known data
  /// dependencies. However, there are cases in which an Algorithm's data
  /// dependencies are only known at execution time, typically when data is
  /// loaded on demand from a file or database. In this case, this method should
  /// be used to declare those dynamic data dependencies.
  ///
  void addDataDependency( const DataObjID& key, AccessMode access ) final override {
    auto index = accessToIndex( access );
    m_dataDepKeys[index].insert( key );
  }

  /// Tell which whiteboard keys the algorithm will be reading or writing
  ///
  /// This function will only provide the full dependency list after it has
  /// been collected, which happens during initialization.
  ///
  const DataObjIDColl& dataDependencies( AccessMode access ) const final override
  {
    auto index = accessToIndex( access );
    return allDataDependencies()[index];
  }

  /// Declare ownership of a legacy DataHandle
  void declare( Gaudi::v1::DataHandle& handle ) override
  {
    if ( m_explicitDepsCollected && isNewDataDependency( handle ) ) {
      throw GaudiException( "Cannot register a new data handle after data dependency collection", this->name(), StatusCode::FAILURE );
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

  // Access the internal array of data dependencies. This is only meant for
  // internal use, but must be public as we access it via IDataHandleHolder.
  const DataObjIDColl* allDataDependencies() const final override {
    if ( !m_explicitDepsCollected ) {
      throw GaudiException( "Cannot read data dependencies before they are collected", this->name(), StatusCode::FAILURE );
    }
    return m_dataDepKeys;
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

  /// Update the key of each registered data dependency, using a
  /// user-defined mapping from the old to the new key.
  ///
  /// This method must be called at a framework initialization stage
  /// where the DataObjIDs associated with data dependencies are
  /// guaranteed not to change anymore. Otherwise the DataObjIDs changes
  /// brought by this update will be overwritten.
  ///
  /// It should also be called before the explicit dependencies are
  /// collected by collectExplicitDependencies(), as otherwise the DataObjIDs
  /// modifications will be carried out in two different places, resulting
  /// in worse performance.
  ///
  void updateDataDependencies( const DataObjIDMapping& keyMap )
  {
    updateDependencies( m_dataHandles, keyMap );
    updateDependencies( m_legacyHandles, keyMap );
    updateDependencies( m_dataDepKeys, keyMap );
    updateDependencies( m_extraDeps, keyMap );
  }

  /// Collect all explicit data dependencies in a single place
  ///
  /// Like updateDataDependencies(), this method must be called at a framework
  /// initialization stage where the DataObjIDs associated with data
  /// dependencies are set in stone and will not change anymore.
  ///
  /// For optimal performance, it should also be called after any call
  /// to updateKeys (see that method's documentation for more info).
  ///
  void collectExplicitDataDependencies()
  {
    // Considering this function's interface contract, calling it multiple
    // times is very likely to be the result of a usage error.
    if ( m_explicitDepsCollected ) {
      throw GaudiException( "Explicit dependencies may only be collected once", this->name(), StatusCode::FAILURE );
    }

    // Collect the centralized lists of inputs and outputs
    collectDependencies( m_dataHandles );
    collectDependencies( m_extraDeps );
    collectDependencies( m_legacyHandles );

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
  StatusCode handleCircularDataDependencies( CircularDepHandler&& circularDepHandler )
  {
    // Make sure that explicit dependencies have been collected beforehand
    if ( !m_explicitDepsCollected ) {
      throw GaudiException( "Explicit dependencies must be collected first", this->name(), StatusCode::FAILURE );
    }

    // Go through the output dependencies, looking for circular input
    // dependencies
    //
    // We must use this inefficient O(N²) iteration pattern because circular
    // dependency handling may mutate the input dependency list,
    // invalidating any iterator to that list.
    //
    const DataObjIDColl& outputKeys = m_dataDepKeys[accessToIndex(AccessMode::Write)];
    DataObjIDColl& inputKeys = m_dataDepKeys[accessToIndex(AccessMode::Read)];
    for ( const auto& outputKey : outputKeys ) {
      auto inputPos = inputKeys.find( outputKey );
      if ( inputPos != inputKeys.end() ) {
        // Call the user circular dependency handler on each dependency
        switch ( circularDepHandler( *inputPos ) ) {
        // Abort iteration if asked to do so
        case CircularDepAction::Abort:
          return StatusCode::FAILURE;

        // Break the circular dependency otherwise
        case CircularDepAction::Ignore:
          inputKeys.erase( inputPos );
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
  void collectImplicitDataDependencies( const IDataHandleHolder* child )
  {
    if ( !child ) return;  // Framework randomly sends nulls to this function...
    collectDependencies( child->allDataDependencies() );
  }

  /// Tell which data dependencies have been ignored due to an empty key
  ///
  /// The interface caveats described in inputKeys() also apply here.
  ///
  const DataObjIDColl& ignoredDataDependencies( AccessMode access ) const
  {
    if ( !m_explicitDepsCollected ) {
      throw GaudiException( "Cannot read ignored data dependencies before they are collected", this->name(), StatusCode::FAILURE );
    }
    return m_ignoredDataDeps[accessToIndex(access)];
  }

  /// Initialize the DataHandles
  ///
  /// This operation accesses framework services and must therefore be run
  /// after the host Algorithm or AlgTool has been sysInitialized.
  ///
  void initializeDataHandleHolder()
  {
    for ( auto handle : m_legacyHandles ) handle->init();

    for ( auto handleArray : m_dataHandles ) {
      for ( auto handle: handleArray ) {
        handle->initialize( *this );
      }
    }
  }

private:
  // We need to know the number of DataHandle access modes
  static constexpr size_t NUM_ACCESS_MODES = static_cast<size_t>(AccessMode::NUM_ACCESS_MODES);

  // Data handles associated with input and output data
  using DataHandleList = std::vector<Gaudi::v2::DataHandle*>;
  DataHandleList m_dataHandles[NUM_ACCESS_MODES];

  // Legacy DataHandles
  std::unordered_set<Gaudi::v1::DataHandle*> m_legacyHandles;

  // Properties allowing extra input and output dependencies to be
  // declared at configuration time.
  static_assert( NUM_ACCESS_MODES == 2,
                 "This part of the code assumes that there are only two DataHandle access modes" );
  static_assert( static_cast<size_t>( AccessMode::Read ) == 0,
                 "This part of the code assumes that \"Read\" comes first in the AccessMode enum" );
  Gaudi::Property<DataObjIDColl> m_extraDeps[NUM_ACCESS_MODES] {
    { this, "ExtraInputs", DataObjIDColl{} },
    { this, "ExtraOutputs", DataObjIDColl{} }
  };

  // Location where all data dependencies will be eventually collected
  DataObjIDColl m_dataDepKeys[NUM_ACCESS_MODES];

  /// Truth that all explicit data dependencies have already been
  /// collected in a single list.
  bool m_explicitDepsCollected = false;

  // Places where we keep track of the inputs and outputs that were
  // ignored because they had an empty key
  DataObjIDColl m_ignoredDataDeps[NUM_ACCESS_MODES];

  /// Convert a data dependency access mode to its index in our internal storage
  static size_t accessToIndex( AccessMode access ) {
    return static_cast<size_t>( access );
  }

  /// Check if a data dependency has not been declared before (new DataHandle version)
  bool isNewDataDependency( const Gaudi::v2::DataHandle& handle ) const {
    auto index = accessToIndex( handle.access() );
    return m_dataDepKeys[index].find( handle.targetKey() ) == m_dataDepKeys[index].end();
  }

  /// Check if a data dependency has not been declared before (new DataHandle version)
  bool isNewDataDependency( const Gaudi::v1::DataHandle& handle ) const {
    bool result = false;
    if ( handle.mode() & Gaudi::v1::DataHandle::Reader ) {
      auto index = accessToIndex( AccessMode::Read );
      result &= ( m_dataDepKeys[index].find( handle.fullKey() ) == m_dataDepKeys[index].end() );
    }
    if ( handle.mode() & Gaudi::v1::DataHandle::Writer ) {
      auto index = accessToIndex( AccessMode::Write );
      result &= ( m_dataDepKeys[index].find( handle.fullKey() ) == m_dataDepKeys[index].end() );
    }
    return result;
  }

  /// Access the key of a DataHandle
  static const DataObjID& accessKey( const Gaudi::v2::DataHandle* handlePtr ) { return handlePtr->targetKey(); }

  /// Access the key of a DataObjID (identity function)
  static const DataObjID& accessKey( const DataObjID& key ) { return key; }

  /// Access the key of a legacy (non-reentrant) DataHandle
  static const DataObjID& accessKey( const Gaudi::v1::DataHandle* handlePtr ) { return handlePtr->fullKey(); }

  /// Extract the key of a key holder, putting empty keys on an ignore list
  template <typename KeyHolder>
  static void extractKey( DataObjIDColl& validOutput, DataObjIDColl& ignoredOutput, const KeyHolder& holder )
  {
    const DataObjID& key = accessKey( holder );
    if ( !key.empty() ) {
      validOutput.insert( key );
    } else {
      ignoredOutput.insert( key );
    }
  }

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
      extractKey( validOutput, ignoredOutput, holder );
    }
  }

  template <typename KeyHolderColl>
  void collectDependencies( const KeyHolderColl keyHolderArray[] ) {
    for ( size_t i = 0; i < NUM_ACCESS_MODES; ++i ) {
      extractKeys( m_dataDepKeys[i], m_ignoredDataDeps[i], keyHolderArray[i] );
    }
  }

  /// Specialization of extractDependencies for legacy DataHandles
  ///
  /// These need special treatment, among other things because they can be both
  /// reading from and writing to a given whiteboard location.
  ///
  void collectDependencies( const std::unordered_set<Gaudi::v1::DataHandle*> legacyHandles ) {
    DataObjIDColl& validInputs = m_dataDepKeys[accessToIndex(AccessMode::Read)];
    DataObjIDColl& validOutputs = m_dataDepKeys[accessToIndex(AccessMode::Write)];
    DataObjIDColl& ignoredInputs = m_ignoredDataDeps[accessToIndex(AccessMode::Read)];
    DataObjIDColl& ignoredOutputs = m_ignoredDataDeps[accessToIndex(AccessMode::Write)];

    for ( auto handle : legacyHandles ) {
      if ( handle->mode() & Gaudi::v1::DataHandle::Reader ) {
        extractKey( validInputs, ignoredInputs, handle );
      }
      if ( handle->mode() & Gaudi::v1::DataHandle::Writer ) {
        extractKey( validOutputs, ignoredOutputs, handle );
      }
    }
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
  static void updateDependencies( KeyHolderColl& keyHolders, const DataObjIDMapping& keyMap )
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
  static void updateDependencies( DataObjIDColl& keys, const DataObjIDMapping& keyMap )
  {
    DataObjIDColl result;
    result.reserve( keys.size() );
    for ( const auto& key : keys ) {
      result.emplace( keyMap( key ).value_or( key ) );
    }
    keys = std::move( result );
  }

  /// Specialization for properties which accesses the inner collection
  static void updateDependencies( Gaudi::Property<DataObjIDColl>& keys, const DataObjIDMapping& keyMap )
  {
    updateDependencies( keys.value(), keyMap );
  }

  /// Specialization of the above algorithm for arrays of KeyHolderColls
  template <typename KeyHolderColl>
  static void updateDependencies( KeyHolderColl keyHoldersArray[], const DataObjIDMapping& keyMap ) {
    for ( size_t i = 0; i < NUM_ACCESS_MODES; ++i ) {
      updateDependencies( keyHoldersArray[i], keyMap );
    }
  }
};

#endif // !GAUDIKERNEL_DATAHANDLEHOLDERBASE
