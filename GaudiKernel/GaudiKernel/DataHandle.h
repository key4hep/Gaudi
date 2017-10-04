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

class Algorithm;
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

  /// Work-in-progress rewrite of DataHandle and its children
  namespace experimental {
    /// Base class to all new-style data handles
    class DataHandle {
      public:
        /// (Configurable) ID of the data being accessed via this handle
        ///
        /// The current proposal is to only support one ID per handle. If
        /// experiments need to read from multiple whiteboard keys (LHCb
        /// alternate paths) or to write an object that's accessible via
        /// multiple whiteboard keys (ATLAS), they would need to implement the
        /// underlying support on their own.
        ///
        /// For LHCb alternate paths, this can be handled at the DataProvider
        /// level (try loading from alternate paths if original one is empty) or
        /// via good old DataOnDemand for the sequential case.
        ///
        /// FIXME: For aliased writes, how would that work with the Scheduler?
        ///
        const DataObjID& id() const { return m_id; }

        /// Initialize the data handle
        ///
        /// This must be done after the whiteboard has been set up. One safe
        /// place to do this is in Algorithm::initialize().
        ///
        void initialize();

      protected:
        /// Handles allow either to insert data ("write") or read it
        enum struct Mode { Read, Write };

        /// Which data store are we talking to
        enum struct DataStore { IDataProviderSvc };

        /// Construct like a Gaudi property
        /// FIXME: Correctly detect if Owner is the right type...
        template<typename Owner>
        DataHandle(Owner& owner,
                   const std::string& propertyName,
                   DataObjID defaultID,
                   const std::string& docString,
                   Mode accessMode,
                   DataStore /* store */)
          : m_id{&owner, propertyName, defaultID, docString}
          , m_owner{owner}
        {
          switch(accessMode) {
            case Mode::Read:
              owner.registerInputHandle(*this);
              break;
            case Mode::Write:
              owner.registerOutputHandle(*this);
              break;
            default:
              throw std::runtime_error("Unsupported access mode");
          }
        }

        /// Data object ID of the target data, as a configurable property
        /// FIXME: Expose to Property machinery whether this is an input/output
        ///        and what kind of store it is accessing
        Gaudi::Property<DataObjID> m_id;

        /// Reference to the owner Algorithm
        /// FIXME: Add support for owner tools?
        std::reference_wrapper<const Algorithm> m_owner;

        /// Pointer to the whiteboard, set during initialize()
        /// FIXME: Usage of the Gaudi whiteboard should not be mandated, for
        ///        example conditions do not require it.
        IDataProviderSvc* m_whiteBoard = nullptr;
    };

    /// Reentrant mechanism to read data from the EventStore
    /// FIXME: Support other stores by extracting the data access logic
    template<typename T>/*, typename DataStore>*/
    class ReadHandle/*Impl*/ : public DataHandle {
      public:
        /// Create a ReadHandle and set up the associated Gaudi property
        /// FIXME: Correctly detect if Owner is the right type...
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

    /// Reentrant mechanism to write data into the EventStore
    /// FIXME: Support other stores by extracting the data access logic
    template<typename T>/*, typename DataStore>*/
    class WriteHandle/*Impl*/ : public DataHandle {
      public:
        /// Create a WriteHandle and set up the associated Gaudi property
        /// FIXME: Correctly detect if Owner is the right type...
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
