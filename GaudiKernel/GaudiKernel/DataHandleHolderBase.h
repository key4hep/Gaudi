#ifndef GAUDIKERNEL_DATAHANDLEHOLDERBASE
#define GAUDIKERNEL_DATAHANDLEHOLDERBASE 1

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/Property.h"

#include <algorithm>
#include <unordered_set>

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
  void initDataHandleHolder()
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
    template<typename Base,
             std::enable_if_t<std::is_base_of<IDataHandleHolderReqs,
                                              Base>::value>* = nullptr>
    class GAUDI_API DataHandleHolder : public extends<Base, IDataHandleHolder> {
        using Super = extends<Base, IDataHandleHolder>;
      public:
        // NOTE: Cannot use "using extends<Base, IDataHandleHolder>::extends;"
        //       due to a GCC 6 bug
        template<typename... Args>
        DataHandleHolder( Args&&... args )
          : Super( std::forward<Args>(args)... )
        {}

        // === INTERFACE FOR GAUDI STATE MACHINE ===

        /// Defer handle initialization until the base class is ready
        ///
        /// FIXME: This is a hack. It will break as soon as a child Algorithm
        ///        or AlgTool will implement its own version of initialize() and
        ///        forget to call us back. The only long-term fix is to
        ///        integrate handle initialization into the sysInitialize
        ///        methods of Algorithm and AlgTool, a fairly invasive change
        ///        which I would like to postpone at this point in time.
        ///
        StatusCode initialize() override {
          auto sc = Base::initialize();
          if(sc.isFailure()) return sc;
          initializeHandles(m_eventInputHandles);
          initializeHandles(m_eventOutputHandles);
          return sc;
        }

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
            result.insert(handlePtr->id());
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
