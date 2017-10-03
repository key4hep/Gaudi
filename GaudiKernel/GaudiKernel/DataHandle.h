#ifndef GAUDIKERNEL_DATAHANDLE
#define GAUDIKERNEL_DATAHANDLE 1

#include <memory>
#include <stdexcept>
#include "GaudiKernel/AnyDataWrapper.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IDataProviderSvc.h"

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
        /// FIXME: Is DataObjID the right type for data dependencies that are
        ///        not stored in the whiteboard, such as conditions?
        /// FIXME: How do we support alternate paths? ("Read from multiple IDs")
        /// FIXME: How do we support writing to multiple IDs? (for ATLAS)
        const DataObjID& id() const { return m_id; }

        /// Initialize the data handle
        ///
        /// This must be done after the whiteboard has been set up. One safe
        /// place to do this is Algorithm::initialize().
        ///
        void initialize();

      protected:
        /// Construct like a Gaudi property
        /// FIXME: Correctly detect if Owner is the right type...
        template<typename Owner>
        DataHandle(Owner& owner,
                   const std::string& propertyName,
                   DataObjID defaultID,
                   const std::string& docString)
          : m_id{&owner, propertyName, defaultID, docString}
          , m_owner{owner}
        {}

        /// Data object ID of the target data, as a configurable property
        /// FIXME: Expose to Property machinery whether this is an input/output
        Gaudi::Property<DataObjID> m_id;

        /// Reference to the owner Algorithm
        /// FIXME: Add support for owner tools?
        Algorithm& m_owner;

        /// Pointer to the whiteboard, set during initialize()
        /// FIXME: Usage of the Gaudi whiteboard should not be mandated, for
        ///        example conditions do not require it.
        IDataProviderSvc* m_whiteBoard = nullptr;
    };

    /// Reentrant mechanism to read data from the EventStore
    template<typename T>
    class ReadHandle : public DataHandle {
      public:
        /// Create a ReadHandle and set up the associated Gaudi property
        /// FIXME: Correctly detect if Owner is the right type...
        template<typename Owner>
        ReadHandle(Owner* owner,
                   const std::string& propertyName,
                   DataObjID defaultID,
                   const std::string& docString = "")
          : DataHandle{*owner, propertyName, defaultID, docString}
        {
          owner->registerInputHandle(*this);
        }

        /// Access the data associated with the current event context
        const T& get(const EventContext& ctx) const {
          // FIXME: Once this is working, start leveraging the new interface:
          //          - Shouldn't need to rely on implicit thread-local context
          //          - Shouldn't need to dynamic_cast the data
          DataObject* ptr = nullptr;
          auto sc = m_whiteBoard->retrieveObject(id().key(), ptr);
          if(sc.isFailure()) {
            throw std::runtime_error("Failed to read input from whiteboard");
          }
          return dynamic_cast<AnyDataWrapper<T>&>(*ptr).getData();
        }
    };

    /// Reentrant mechanism to write data into the EventStore
    template<typename T>
    class WriteHandle : public DataHandle {
      public:
        /// Create a WriteHandle and set up the associated Gaudi property
        /// FIXME: Correctly detect if Owner is the right type...
        template<typename Owner>
        WriteHandle(Owner* owner,
                    const std::string& propertyName,
                    DataObjID defaultID,
                    const std::string& docString = "")
          : DataHandle{*owner, propertyName, defaultID, docString}
        {
          owner->registerOutputHandle(*this);
        }

        /// Insert data for the current event
        const T& put(const EventContext& ctx, T data) const {
          // FIXME: Once this is working, start leveraging the new interface:
          //          - Shouldn't need to rely on implicit thread-local context
          auto ptr = new AnyDataWrapper<T>(std::move(data));
          auto sc = m_whiteBoard->registerObject(id().key(), ptr);
          if(sc.isFailure()) {
            throw std::runtime_error("Failed to write output into whiteboard");
          }
          return ptr->getData();
        }

#ifdef ATLAS
        /// (ATLAS-specific) Insert heap allocated data for the current event
        /// TODO: Implement this
        const T& put(const EventContext& ctx, std::unique_ptr<T> data) const;
#endif
    };
  }
}

#endif
