#ifndef GAUDIKERNEL_DATAHANDLEHOLDERBASE
#define GAUDIKERNEL_DATAHANDLEHOLDERBASE 1

#include <algorithm>
#include <type_traits>
#include <unordered_set>
#include <vector>
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

private:
  std::unordered_set<Gaudi::DataHandle*> m_handles;

  Gaudi::Property<DataObjIDColl> m_extInputDataObjs{this, "ExtraInputs", DataObjIDColl{}};
  Gaudi::Property<DataObjIDColl> m_extOutputDataObjs{this, "ExtraOutputs", DataObjIDColl{}};
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

        // === INTERFACE FOR DATA HANDLES ===

        /// Register a data handle as an event data input of the algorithm
        void registerEventInput(DataHandle& handle) final override {
          m_eventInputHandles.push_back(&handle);
        }

        /// Register a data handle as an event data output of the algorithm
        void registerEventOutput(DataHandle& handle) final override {
          m_eventOutputHandles.push_back(&handle);
        }

        // === INTERFACE FOR THE SCHEDULER ===

        /// Tell which event store keys the algorithm will be reading from
        DataObjIDColl eventInputKeys() const final override {
          return getKeys(m_eventInputHandles);
        }

        /// Tell which event store keys the algorithm will be writing to
        DataObjIDColl eventOutputKeys() const final override {
          return getKeys(m_eventOutputHandles);
        }


      protected:
        /// DataHandle initialization will only be finalized once the underlying
        /// Algorithm or AlgTool has went through sysInitialize.
        void initDataHandleHolder() {
          Super::initLegacyDataHandleHolder();
          initializeHandles(m_eventInputHandles);
          initializeHandles(m_eventOutputHandles);
        }

      private:
        /// Data handles associated with input and output event data
        using HandleList = std::vector<DataHandle*>;
        HandleList m_eventInputHandles;
        HandleList m_eventOutputHandles;

        /// Query the keys associated with a set of handles
        static DataObjIDColl getKeys(const HandleList& handles) {
          DataObjIDColl result;
          result.reserve(handles.size());
          for(auto handlePtr: handles) {
            result.insert(handlePtr->targetID());
          }
          return result;
        }

        /// Initialize a set of handles
        void initializeHandles(HandleList& handles) {
          for(auto handlePtr: handles) {
            handlePtr->initialize(*this);
          }
        }
    };
  }
}

#endif // !GAUDIKERNEL_DATAHANDLEHOLDERBASE
