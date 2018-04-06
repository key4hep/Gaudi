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

  std::vector<Gaudi::DataHandle*> inputHandles() const override
  {
    return handles( m_handles, Gaudi::DataHandle::Reader );
  }
  std::vector<Gaudi::DataHandle*> outputHandles() const override
  {
    return handles( m_handles, Gaudi::DataHandle::Writer );
  }

  virtual const DataObjIDColl& extraInputDeps() const override { return m_extInputDataObjs; }
  virtual const DataObjIDColl& extraOutputDeps() const override { return m_extOutputDataObjs; }

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

  const DataObjIDColl& inputDataObjs() const override { return m_inputDataObjs; }
  const DataObjIDColl& outputDataObjs() const override { return m_outputDataObjs; }

  void addDependency( const DataObjID& id, const Gaudi::DataHandle::Mode& mode ) override
  {
    if ( mode & Gaudi::DataHandle::Reader ) m_inputDataObjs.emplace( id );
    if ( mode & Gaudi::DataHandle::Writer ) m_outputDataObjs.emplace( id );
  }

protected:
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

        /// Tell which event store keys the algorithm will be reading from
        DataObjIDColl eventInputKeys() const final override {
          DataObjIDColl inputKeys;
          extractKeys(inputKeys, m_eventInputHandles);
          extractKeys(inputKeys, Super::inputHandles());
          return inputKeys;
        }

        /// Tell which event store keys the algorithm will be writing to
        DataObjIDColl eventOutputKeys() const final override {
          DataObjIDColl outputKeys;
          extractKeys(outputKeys, m_eventOutputHandles);
          extractKeys(outputKeys, Super::outputHandles());
          return outputKeys;
        }


      protected:
        /// DataHandle initialization will only be finalized once the underlying
        /// Algorithm or AlgTool has went through sysInitialize.
        void initDataHandleHolder() {
          Super::initLegacyDataHandleHolder();
          initializeHandles(m_eventInputHandles);
          initializeHandles(m_eventOutputHandles);
        }

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


      private:
        /// Data handles associated with input and output event data
        using HandleList = std::vector<DataHandle*>;
        HandleList m_eventInputHandles;
        HandleList m_eventOutputHandles;

        /// Initialize a set of handles
        void initializeHandles(HandleList& handles) {
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
        static void extractKeys(DataObjIDColl& output,
                                const KeyHolderColl& keyHolders) {
          output.reserve(keyHolders.size());
          for(const auto& holder: keyHolders) {
            const DataObjID& key = accessKey(holder);
            if( !key.empty() ) { output.insert(key); }
          }
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
