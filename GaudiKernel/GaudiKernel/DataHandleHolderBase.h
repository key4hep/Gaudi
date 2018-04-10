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
#include "GaudiKernel/Property.h"

namespace
{
  template <typename Container>
  std::vector<Gaudi::DataHandle*> handles( Container& c, Gaudi::DataHandle::Mode mode )
  {
    std::vector<Gaudi::DataHandle*> h;
    std::copy_if( std::begin( c ), std::end( c ), std::back_inserter( h ),
                  [&]( const Gaudi::DataHandle* hndl ) -> bool { return hndl->mode() & mode; } );
    return h;
  }
}

template <class BASE>
class GAUDI_API DataHandleHolderBase : public extends<BASE, IDataHandleHolder>
{
public:
  using extends<BASE, IDataHandleHolder>::extends;

  void declare( Gaudi::DataHandle& handle ) override
  {
    if ( !handle.owner() ) handle.setOwner( this );

    if ( handle.owner() != this ) {
      throw GaudiException( "Attempt to declare foreign handle with algorithm!", this->name(), StatusCode::FAILURE );
    }

    m_handles.insert( &handle );
  }

  void renounce( Gaudi::DataHandle& handle ) override
  {
    if ( handle.owner() != this ) {
      throw GaudiException( "Attempt to renounce foreign handle with algorithm!", this->name(), StatusCode::FAILURE );
    }
    m_handles.erase( &handle );
  }

protected:
  // Access the legacy input and output handles
  // TODO: Remove this during DataHandleHolderBase / DataHandleHolder merge
  std::vector<Gaudi::DataHandle*> inputHandles() const
  {
    return handles( m_handles, Gaudi::DataHandle::Reader );
  }
  std::vector<Gaudi::DataHandle*> outputHandles() const
  {
    return handles( m_handles, Gaudi::DataHandle::Writer );
  }

  /// initializes all handles - called by the sysInitialize method
  /// of any descendant of this
  void initLegacyDataHandleHolder()
  {
    for ( auto h : m_handles ) h->init();
  }

  DataObjIDColl m_inputDataObjs, m_outputDataObjs;
  Gaudi::Property<DataObjIDColl> m_extInputDataObjs{this, "ExtraInputs", DataObjIDColl{}};
  Gaudi::Property<DataObjIDColl> m_extOutputDataObjs{this, "ExtraOutputs", DataObjIDColl{}};

private:
  std::unordered_set<Gaudi::DataHandle*> m_handles;
};


/// Work-in-progress rewrite of the DataHandle infrastructure
/// FIXME: If accepted, rename header to "DataHandleHolder"
namespace Gaudi
{
  namespace experimental
  {
    /// Implementation of new-style data handle holders (Tools, Algorithms...)
    ///
    /// TODO: Replace DataHandleHolderBase<BASE> with just BASE and delete the
    ///       legacy DataHandleHolderBase once migration to the new DataHandle
    ///       mechanism is complete.
    ///
    template<typename BASE>
    class GAUDI_API DataHandleHolder : public extends<DataHandleHolderBase<BASE>, IDataHandleHolder>
    {
      using Super = extends<DataHandleHolderBase<BASE>, IDataHandleHolder>;
      static_assert(std::is_base_of<IDataHandleHolderReqs, BASE>::value,
                    "Base class must implement IDataHandleHolderReqs");

      public:
        // NOTE: Cannot use "using Super::Super;" due to a GCC 6 bug
        template<typename... Args>
        DataHandleHolder( Args&&... args )
          : Super( std::forward<Args>(args)... )
        {}

        /// Register a data handle as an event data input of the algorithm
        void registerEventInput(DataHandle& handle) final override {
          m_eventInputHandles.push_back(&handle);
        }

        /// Register a data handle as an event data output of the algorithm
        void registerEventOutput(DataHandle& handle) final override {
          m_eventOutputHandles.push_back(&handle);
        }

        /// Add an event data output dynamically at run time
        void addDynamicEventOutput(const DataObjID& key) final override {
          Super::m_outputDataObjs.insert(key);
        }

        /// Tell which event store keys the algorithm will be reading from
        ///
        /// This function should only be called after the explicit data
        /// dependencies have been collected by collectExplicitDeps(), as
        /// otherwise some dependencies will not be reported. But since it will
        /// be called frequently, we only detect this usage error in debug mode.
        ///
        const DataObjIDColl& eventInputKeys() const final override {
          assert(m_explicitDepsCollected);
          return Super::m_inputDataObjs;
        }

        /// Tell which event store keys the algorithm will be writing to
        ///
        /// The same caveats apply as in eventInputKeys()
        ///
        const DataObjIDColl& eventOutputKeys() const final override {
          assert(m_explicitDepsCollected);
          return Super::m_outputDataObjs;
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
        using DataObjIDMapping = std::function<boost::optional<DataObjID>(const DataObjID&)>;

        /// Update the key of each registered event data dependency, using a
        /// user-defined mapping from the old to the new key.
        ///
        /// This method must be called at a framework initialization stage
        /// where the DataObjIDs associated with data dependencies are
        /// guaranteed not to change anumore. Otherwise the DataObjIDs changes
        /// brought by this update will be overwritten.
        ///
        /// It should also be called before the explicit dependencies are
        /// collected by collectExplicitDeps(), as otherwise the DataObjIDs
        /// modifications will be carried out in two different places, resulting
        /// in worse performance.
        ///
        void updateEventKeys(const DataObjIDMapping& keyMap) {
          auto legacyInputHandles = Super::inputHandles();
          auto legacyOutputHandles = Super::outputHandles();
          updateKeys(legacyInputHandles, keyMap);
          updateKeys(legacyOutputHandles, keyMap);
          updateKeys(m_eventInputHandles, keyMap);
          updateKeys(m_eventOutputHandles, keyMap);
          updateKeys(Super::m_inputDataObjs, keyMap);
          updateKeys(Super::m_outputDataObjs, keyMap);
          updateKeys(Super::m_extInputDataObjs.value(), keyMap);
          updateKeys(Super::m_extOutputDataObjs.value(), keyMap);
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
        void collectExplicitDeps() {
          // Considering this function's interface contract, calling it multiple
          // times is very likely to be the result of a usage error.
          if(m_explicitDepsCollected) {
            throw std::runtime_error("Explicit deps may only be collected once");
          }

          // Collect all input dependencies in a single place
          extractEventInputs(m_eventInputHandles);
          extractEventInputs(Super::inputHandles());
          extractEventInputs(Super::m_extInputDataObjs.value());

          // Collect all output dependencies in a single place
          extractEventOutputs(m_eventOutputHandles);
          extractEventOutputs(Super::outputHandles());
          extractEventOutputs(Super::m_extOutputDataObjs.value());

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
        /// DataHandleHolder of its decision with this enum:
        ///
        /// * "Abort" means that the framework cannot recover from this circular
        ///   dependency and will shut down.
        /// * "Ignore" means that the circular dependency is assumed to be a
        ///   false alarm (e.g. data dependencies from inner AlgTools) and the
        ///   corresponding input dependency can be deleted.
        ///
        enum class CircularDepAction { Abort, Ignore };

        /// A circular dependency handler, which can take any user-specified
        /// step (e.g. logging), then produces a dependency handling action.
        using CircularDepHandler = std::function<CircularDepAction(const DataObjID&)>;

        /// Look for circular dependencies and let a user-specified handler deal
        /// with each of them.
        ///
        /// This method must be called after explicit dependencies have been
        /// collected, as otherwise it may miss some circular dependencies.
        ///
        StatusCode handleCircularDeps(CircularDepHandler&& circularDepHandler) {
          // Make sure that explicit dependencies have been collected beforehand
          if(!m_explicitDepsCollected) {
            throw std::runtime_error("Dependencies must be collected first");
          }

          // Access the list of input and output event data dependencies
          DataObjIDColl& inputDeps = Super::m_inputDataObjs;
          const DataObjIDColl& outputDeps = Super::m_outputDataObjs;

          // Go through the output dependencies, looking for circular input
          // dependencies. We must use this inefficient iteration pattern
          // because circular dependency handling may mutate the input
          // dependency list, invalidating any iterator.
          for(const auto& outputKey: outputDeps) {
            auto inputPos = inputDeps.find(outputKey);
            if(inputPos != inputDeps.end()) {
              // Call the user circular dependency handler on each dependency
              switch(circularDepHandler(*inputPos)) {
                // Abort iteration if asked to do so
                case CircularDepAction::Abort:
                  return StatusCode::FAILURE;

                // Break the circular dependency otherwise
                case CircularDepAction::Ignore:
                  inputDeps.erase(inputPos);
              }
            }
          }

          // If control reached this point, we are fine
          return StatusCode::SUCCESS;
        }

        /// Add the explicit dependencies of another DataHandleHolder to the
        /// dependency list of this holder
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
        void addImplicitDeps(const IDataHandleHolder* child) {
          if(!child) return;
          extractEventInputs(child->eventInputKeys());
          extractEventOutputs(child->eventOutputKeys());
        }

        /// Tell which input dependencies have been ignored due to an empty key
        ///
        /// The same caveats apply as in eventInputKeys()
        ///
        const DataObjIDColl& ignoredEventInputs() const {
          assert(m_explicitDepsCollected);
          return m_ignoredEventInputs;
        }

        /// Tell which output dependencies have been ignored due to an empty key
        ///
        /// The same caveats apply as in eventInputKeys()
        ///
        const DataObjIDColl& ignoredEventOutputs() const {
          assert(m_explicitDepsCollected);
          return m_ignoredEventOutputs;
        }

        /// Initialize the DataHandles
        ///
        /// This operation must be run after the host Algorithm or AlgTool has
        /// been sysInitialized, as it accesses framework services.
        ///
        void initDataHandleHolder() {
          Super::initLegacyDataHandleHolder();
          initializeHandles(m_eventInputHandles);
          initializeHandles(m_eventOutputHandles);
        }


      private:
        /// Data handles associated with input and output event data
        using DataHandleList = std::vector<DataHandle*>;
        DataHandleList m_eventInputHandles;
        DataHandleList m_eventOutputHandles;

        /// Truth that all explicit data dependencies have already been
        /// collected in a single list.
        bool m_explicitDepsCollected = false;

        /// Places where we keep track of the inputs and outputs that were
        /// ignored because they had an empty key
        DataObjIDColl m_ignoredEventInputs;
        DataObjIDColl m_ignoredEventOutputs;

        /// Initialize a set of handles
        void initializeHandles(DataHandleList& handles) {
          for(auto handlePtr: handles) {
            handlePtr->initialize(*this);
          }
        }

        /// Access the key of a DataHandle
        static const DataObjID& accessKey(const DataHandle* handlePtr) {
          return handlePtr->targetKey();
        }

        /// Access the key of a DataObjID (identity function)
        static const DataObjID& accessKey(const DataObjID& key) {
          return key;
        }

        /// Access the key of a legacy (non-reentrant) DataHandle
        static const DataObjID& accessKey(const Gaudi::DataHandle* handlePtr) {
          return handlePtr->fullKey();
        }

        /// Extract non-empty DataObjID keys from a collection of key holders
        ///
        /// Given an output key collection and a collection of objects that
        /// have an associated DataObjID key which can be accessed via an
        /// overload of accessKey(), extract the keys, filter out empty ones,
        /// and store the rest in the output collection.
        ///
        template<typename KeyHolderColl>
        static void extractKeys(DataObjIDColl& validOutput,
                                DataObjIDColl& ignoredOutput,
                                const KeyHolderColl& keyHolders) {
          validOutput.reserve(keyHolders.size());
          for(const auto& holder: keyHolders) {
            const DataObjID& key = accessKey(holder);
            if( !key.empty() ) {
              validOutput.insert(key);
            } else {
              ignoredOutput.insert(key);
            }
          }
        }

        /// Specialization of extractKeys for event data inputs
        template<typename KeyHolderColl>
        void extractEventInputs(const KeyHolderColl& keyHolders) {
          extractKeys(Super::m_inputDataObjs, m_ignoredEventInputs, keyHolders);
        }

        /// Specialization of extractKeys for event data outputs
        template<typename KeyHolderColl>
        void extractEventOutputs(const KeyHolderColl& keyHolders) {
          extractKeys(Super::m_outputDataObjs, m_ignoredEventOutputs, keyHolders);
        }

        /// Update the key of a DataHandle
        static void updateKey(DataHandle* target, DataObjID&& key) {
          target->setTargetKey(std::move(key));
        }

        /// Update the key of a DataObjID
        static void updateKey(DataObjID& target, DataObjID&& key) {
          target = std::move(key);
        }

        /// Update the key of a legacy (non-reentrant) DataHandle
        static void updateKey(Gaudi::DataHandle* target, DataObjID&& key) {
          target->setKey(std::move(key));
        }

        /// Update the DataObjID keys of a collection of key holders
        ///
        /// Access the key from each key holder using accessKey, pass it
        /// through a user-defined transform, and if the result is different
        /// from the original key update the key using updateKey.
        ///
        template<typename KeyHolderColl>
        static void updateKeys(KeyHolderColl& keyHolders,
                               const DataObjIDMapping& keyMap) {
          for(auto& holder: keyHolders) {
            const DataObjID& oldKey = accessKey(holder);
            auto mappedKey = keyMap(oldKey);
            if( mappedKey ) { updateKey(holder, *std::move(mappedKey)); }
          }
        }

        /// Specialization of the above algorithm for DataObjIDColls, where
        /// updating the keys in place is not possible.
        static void updateKeys(DataObjIDColl& keys,
                               const DataObjIDMapping& keyMap) {
          DataObjIDColl result;
          result.reserve(keys.size());
          for(const auto& key: keys) {
            result.emplace(keyMap(key).value_or(key));
          }
          keys = std::move(result);
        }
    };
  }
}

#endif // !GAUDIKERNEL_DATAHANDLEHOLDERBASE
