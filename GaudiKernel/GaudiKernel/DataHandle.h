#ifndef GAUDIKERNEL_DATAHANDLE
#define GAUDIKERNEL_DATAHANDLE 1

#include <memory>
#include <stdexcept>
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/HandleDetail.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/Property.h"

//---------------------------------------------------------------------------

/** DataHandle.h  GaudiKernel/DataHandle.h
 *
 * Base class for Handles to access data in Event Store, shared between
 * Gaudi and Atlas
 *
 * Objects are identified via a DataObjID
 *
 * Once object is created, Mode is not modifiable
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class IDataHandleHolder;

namespace Gaudi
{

  class DataHandle
  {
  public:
    enum Mode { Reader = 1 << 2, Writer = 1 << 4, Updater = Reader | Writer };

    DataHandle( const DataObjID& k, Mode a = Reader, IDataHandleHolder* owner = nullptr )
        : m_key( k ), m_owner( owner ), m_mode( a ){};

    DataHandle( const DataObjID& k, const bool& isCond, Mode a = Reader, IDataHandleHolder* owner = nullptr )
        : m_key( k ), m_owner( owner ), m_mode( a ), m_isCond( isCond ){};

    virtual ~DataHandle() = default;

    virtual void setOwner( IDataHandleHolder* o ) { m_owner = o; }
    virtual IDataHandleHolder*                owner() const { return m_owner; }

    virtual Mode mode() const { return m_mode; }

    virtual void setKey( const DataObjID& key ) const { m_key = key; }
    virtual void updateKey( const std::string& key ) const { m_key.updateKey( key ); }

    virtual const std::string& objKey() const { return m_key.key(); }
    virtual const DataObjID&   fullKey() const { return m_key; }

    virtual void reset( bool ){};

    virtual std::string pythonRepr() const;
    virtual bool        init() { return true; }

    // is this a ConditionHandle?
    virtual bool isCondition() const { return m_isCond; }

  protected:
    /**
     * The key of the object behind this DataHandle
     * Although it may look strange to have it mutable, this can actually
     * change in case the object had alternative names, and it should not
     * be visible to the end user, for which the Handle is still the same
     */
    mutable DataObjID  m_key   = {"NONE"};
    IDataHandleHolder* m_owner = nullptr;

  private:
    Mode m_mode   = Reader;
    bool m_isCond = false;
  };

  /// Work-in-progress rewrite of the DataHandle infrastructure
  namespace experimental
  {
    class IDataHandleHolder;

    /// Base class to all new-style data handles
    class DataHandle {
      public:
        // DataHandles are used for the purpose of accounting component data
        // dependencies, and should thus never be copied.
        DataHandle(const DataHandle&) = delete;
        DataHandle& operator=(const DataHandle&) = delete;

        // DataHandles pass a pointer to themselves to the host Algorithm during
        // registration, which happens at construction time. They should thus
        // not be moved, as that would invalidate said pointer. The only reason
        // why we don't delete the DataHandle move constructor as well is that
        // it would also forbid safe and useful forms of in-place construction.
        DataHandle(DataHandle&&) = default;
        DataHandle& operator=(DataHandle&&) = delete;

        /// (Configurable) ID of the data being accessed via this handle
        ///
        /// The current proposal is to only support one ID per handle. If
        /// experiments need to read from multiple whiteboard keys (LHCb
        /// alternate paths) or to write an object that's accessible via
        /// multiple whiteboard keys (ATLAS), they would need to implement the
        /// underlying support on their own.
        ///
        /// For LHCb alternate paths, this can be handled at the DataLoader
        /// level (try linking from alternate paths if original one is empty),
        /// or via good old DataOnDemand for the sequential case. See
        /// https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/422 .
        ///
        /// FIXME: For aliased writes, how would that work with the Scheduler?
        ///
        const DataObjID& id() const { return m_id; }

        /// Initialize the data handle
        ///
        /// This must be done by the owner once it has initialized itself. It
        /// is the step at which the handle performs deferred framework
        /// operations such as acquiring access to the data stores.
        ///
        void initialize(const IDataHandleHolder& owner);

      protected:
        /// Handles allow either to insert data ("write") or read it back
        enum struct Mode { Read, Write };

        /// Handles will eventually support multiple storage backends, even if
        /// they currently only support the standard Gaudi event store
        enum struct DataStore { IDataProviderSvc };

        /// Handles are constructed like a Gaudi property (and effectively
        /// behave as one, which sets the associated data object identifier)
        template<typename Owner,
                 std::enable_if_t<std::is_base_of<IDataHandleHolder,
                                                  Owner>::value>* = nullptr>
        DataHandle(Owner& owner,
                   const std::string& propertyName,
                   DataObjID defaultID,
                   const std::string& docString,
                   Mode accessMode,
                   DataStore store)
          : m_id{&owner, propertyName, defaultID, docString}
        {
          registerToOwner(owner, accessMode, store);
        }

        /// Pointer to the whiteboard, set during initialize()
        /// FIXME: Usage of the Gaudi whiteboard should not be mandated, for
        ///        example conditions do not require it.
        IDataProviderSvc* m_whiteBoard = nullptr;

      private:
        /// The data object ID of the target data can be configured
        /// FIXME: Expose to Property machinery whether this is an input/output
        ///        and what kind of store it is accessing
        Gaudi::Property<DataObjID> m_id;

        /// Register ourselves to the owner (algorithm or tool)
        void registerToOwner(IDataHandleHolder& owner,
                             Mode accessMode,
                             DataStore store);
    };

    /// Read handles provide a reentrant whiteboard readout mechanism
    /// FIXME: Support other stores by extracting the data access logic
    template<typename T>/*, typename DataStore>*/
    class ReadHandle/*Impl*/ : public DataHandle {
      public:
        /// Create a ReadHandle and set up the associated Gaudi property
        template<typename Owner>
        ReadHandle(Owner* owner,
                   const std::string& propertyName,
                   DataObjID defaultID,
                   const std::string& docString = "")
          : DataHandle{*owner,
                       propertyName,
                       defaultID,
                       docString,
                       DataHandle::Mode::Read,
                       DataHandle::DataStore::IDataProviderSvc}
        {}

        /// Access the data for the current event context
        decltype(auto) get(const EventContext& /* ctx */) const {
          // FIXME: Once this is working, start leveraging the new interface:
          //          - Shouldn't need to rely on implicit thread-local context
          DataObject* ptr = nullptr;
          auto sc = m_whiteBoard->retrieveObject(id().key(), ptr);
          if(sc.isFailure()) {
            throw std::runtime_error("Failed to read input from whiteboard");
          }
          return HandleDetail::unwrapDataObject<T>(*ptr);
        }
    };

    /// Read handles provide a reentrant whiteboard insertion mechanism
    /// FIXME: Support other stores by extracting the data access logic
    template<typename T>/*, typename DataStore>*/
    class WriteHandle/*Impl*/ : public DataHandle {
      public:
        /// Create a WriteHandle and set up the associated Gaudi property
        template<typename Owner>
        WriteHandle(Owner* owner,
                    const std::string& propertyName,
                    DataObjID defaultID,
                    const std::string& docString = "")
          : DataHandle{*owner,
                       propertyName,
                       defaultID,
                       docString,
                       DataHandle::Mode::Write,
                       DataHandle::DataStore::IDataProviderSvc}
        {}

        /// Move data into the store
        const T& put(const EventContext& ctx, T data) const {
          auto ptrAndRef = HandleDetail::wrapDataObject<T>(std::move(data));
          putImpl(ctx, std::move(ptrAndRef.first));
          return ptrAndRef.second;
        }

        /// Transfer ownership of heap-allocated data to the store
        ///
        /// This is intended as a way to inject legacy non-movable DataObjects
        /// into the store. New data types should be movable and use the other
        /// overload of put. This method will eventually be removed.
        ///
        const T& put(const EventContext& ctx,
                     std::unique_ptr<DataObject> ptr) const
        {
          auto ptrAndRef = HandleDetail::wrapDataObject<T>(std::move(ptr));
          putImpl(ctx, std::move(ptrAndRef.first));
          return ptrAndRef.second;
        }

      private:
        /// Insert a valid DataObject into the transient event store
        void putImpl(const EventContext& /* ctx */,
                     std::unique_ptr<DataObject>&& ptr) const
        {
          // FIXME: Once this is working, start leveraging the new interface:
          //          - Shouldn't need to rely on implicit thread-local context
          auto sc = m_whiteBoard->registerObject(id().key(), ptr.release());
          if(sc.isFailure()) {
            throw std::runtime_error("Failed to write output into whiteboard");
          }
        }
    };

    // FIXME: Fill in those typedefs later on
    /* template<typename T> using EventReadHandle = ReadHandleImpl<T, IDataProviderSvc>;
    template<typename T> using EventWriteHandle = WriteHandleImpl<T, IDataProviderSvc>; */
    // template<typename T> using ConditionReadHandle = ReadHandleImpl<T, IConditionStore>;
    // template<typename T> using ConditionWriteHandle = WriteHandleImpl<T, IConditionStore>;
  }
}

#endif
