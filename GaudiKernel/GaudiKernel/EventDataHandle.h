#ifndef GAUDIKERNEL_EVENTDATAHANDLE
#define GAUDIKERNEL_EVENTDATAHANDLE 1

#include <memory>
#include <stdexcept>
#include <string>
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/HandleDetail.h"
#include "GaudiKernel/IDataProviderSvc.h"

//---------------------------------------------------------------------------

/** EventDataHandle.h  GaudiKernel/EventDataHandle.h
 *
 * Implementation of the DataHandle concept that interacts with the standard
 * Gaudi Transient Event Store.
 *
 * @author Hadrien Grasland
 * @date   2017-10-06
 */

//---------------------------------------------------------------------------

class IDataHandleHolder;

/// Work-in-progress rewrite of the DataHandle infrastructure
namespace Gaudi
{
  namespace experimental
  {
    /// Base class to all DataHandles interacting with the transient event store
    class EventDataHandle : public DataHandle {
      public:
        /// Initialize the data handle
        void initialize(const IDataHandleHolder& owner) final override;

      protected:
        using AccessMode = IDataHandleMetadata::AccessMode;

        /// DataHandle subclass metadata for event data handles
        class Metadata : public IDataHandleMetadata {
          public:
            // Initialize by specifying which access mode should be used
            Metadata(AccessMode access)
              : m_access{access}
            {}

            // Identifier for the whiteboard implementation that we use
            std::string whiteBoard() const final override {
              return "IDataProviderSvc";
            }

            // Tell what is the access mode
            AccessMode access() const final override { return m_access; }

          private:
            AccessMode m_access;
        };

        /// Handles are constructed like a Gaudi property (and effectively
        /// behave as one, which sets the associated data object identifier)
        template<typename Owner>
        EventDataHandle(Owner& owner,
                        const std::string& propertyName,
                        DataObjID&& defaultID,
                        const std::string& docString,
                        const Metadata& metadata)
          : DataHandle{owner,
                       propertyName,
                       std::move(defaultID),
                       docString,
                       metadata}
        {
          registerToOwner(owner, metadata.access());
        }

        /// Pointer to the whiteboard, set during initialize()
        IDataProviderSvc* m_whiteBoard = nullptr;

      private:
        /// Register ourselves to the owner (algorithm or tool)
        void registerToOwner(IDataHandleHolder& owner, AccessMode access);
    };


    /// Reentrant read handle for event data
    template<typename T>
    class EventReadHandle : public EventDataHandle {
      protected:
        using AccessMode = IDataHandleMetadata::AccessMode;

      public:
        /// Create a ReadHandle and set up the associated Gaudi property
        template<typename Owner>
        EventReadHandle(Owner* owner,
                        const std::string& propertyName,
                        DataObjID defaultID,
                        const std::string& docString = "")
          : EventDataHandle{*owner,
                            propertyName,
                            std::move(defaultID),
                            docString,
                            {AccessMode::Read}}
        {}

        /// Access the data for the current event context
        decltype(auto) get(const EventContext& /* ctx */) const {
          // TODO: Introduce and use an EventContext-aware whiteboard interface
          DataObject* ptr = nullptr;
          auto sc = m_whiteBoard->retrieveObject(targetKey().key(), ptr);
          if(sc.isFailure()) {
            throw std::runtime_error("Failed to read input from whiteboard");
          }
          return HandleDetail::unwrapDataObject<T>(*ptr);
        }
    };


    /// Reentrant write handle for event data
    template<typename T>
    class EventWriteHandle : public EventDataHandle {
      protected:
        using AccessMode = IDataHandleMetadata::AccessMode;

      public:
        /// Create a WriteHandle and set up the associated Gaudi property
        template<typename Owner>
        EventWriteHandle(Owner* owner,
                         const std::string& propertyName,
                         DataObjID defaultID,
                         const std::string& docString = "")
          : EventDataHandle{*owner,
                            propertyName,
                            std::move(defaultID),
                            docString,
                            {AccessMode::Write}}
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
          // TODO: Introduce and use an EventContext-aware whiteboard interface
          auto sc = m_whiteBoard->registerObject(targetKey().key(),
                                                 ptr.release());
          if(sc.isFailure()) {
            throw std::runtime_error("Failed to write output into whiteboard");
          }
        }
    };
  }
}

#endif
