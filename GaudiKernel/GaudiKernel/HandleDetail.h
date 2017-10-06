#ifndef GAUDIKERNEL_HANDLEDETAIL
#define GAUDIKERNEL_HANDLEDETAIL 1

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "GaudiKernel/AnyDataWrapper.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataObjID.h"

//---------------------------------------------------------------------------

/** HandleDetail.h  GaudiKernel/HandleDetail.h
 *
 * Implementation details of the new DataHandle implementation
 *
 * @author Hadrien Grasland
 * @date   2017-10-04
 */

//---------------------------------------------------------------------------

class DataObject;

/// Implementation details of DataHandles
namespace Gaudi
{
  namespace experimental
  {
    namespace HandleDetail
    {
      // === FACTORING OUT SOME SFINAE BOILERPLATE ===

      /// Tells whether type T is a subclass of DataObject
      template<typename T> constexpr bool isDataObject() {
        return std::is_base_of<DataObject, T>::value;
      }

      /// Tells whether there is a parameter pack U... such as T = Range<U...>
      template<typename T> constexpr bool isRange() {
        // FIXME: Implement this
        return false;
      }

      // This will need to be undef'd later, but brings so much fresh air...
#define ENABLE_IF(condition) std::enable_if_t<condition>* = nullptr

      // === DECODING OF A DATAOBJECT* FROM THE TES ===

      // In the ReadHandle<T> implementation, we want to abstract away a
      // number of storage implementation scenarios:
      //   - T is a subclass of DataObject, stored as-is in the TES.
      //   - T is stored as an AnyDataWrapper<T> in the TES: must unwrap it.
      //   - T = Range<U> and the TES holds a Range<U>: copy it and return it.
      //   - T = Range<U> and the TES holds a T::underlying container: build a
      //     Range<U> that covers the whole container and return it.
      //
      // These various cases are resolved via the unwrapDataObject<T> method.

      /// T is a subclass of DataObject, and is not a range: we can just
      /// cast the pointer provided by the TES as-is.
      template<typename T, ENABLE_IF(isDataObject<T>() && !isRange<T>())>
      const T& unwrapDataObject(const DataObject& rawObject) {
        // FIXME: Dynamic casting on every access is expensive, but the only way
        //        to retain type safety if other code is allowed to perform
        //        arbitrary operations on the TES. The more we restrict what
        //        code other than handles is allowed to do with the TES, the
        //        further away from the hot code path we can push these checks.
        return dynamic_cast<const T&>(rawObject);
      }

      /// T is not a subclass of DataObject and is not a range. So it is stored
      /// in the TES as an AnyDataWrapper, which we need to un-wrap.
      template<typename T, ENABLE_IF(!isDataObject<T>() && !isRange<T>())>
      const T& unwrapDataObject(const DataObject& rawObject) {
        // FIXME: See comment above.
        return dynamic_cast<const AnyDataWrapper<T>&>(rawObject).getData();
      }

      /// T is a Range<...>: Either the TES contains a Range to be copied, or a
      /// container which must be turned into a range spanning its content.
      template<typename RangeT, ENABLE_IF(isRange<RangeT>())>
      const RangeT unwrapDataObject(const DataObject& rawObject) {
        // FIXME: Now that we know we're dealing with a range, we need to figure
        //        out whether the thing that lies in the event store is a RangeT
        //        or a container of RangeT::underlying.
        //          - If it's a RangeT, decode it as appropriate and copy it
        //          - If it's a container, create a suitable RangeT & return it
        throw std::runtime_error("Not implemented yet!");
      }

      // === WRAPPING DATA INTO A DATAOBJECT* FOR THE TES ===

      // The TES will only accept as input pointers to heap-allocated objects
      // that inherit from DataObject. We will manipulate those as unique_ptrs.
      using DataObjectPtr = std::unique_ptr<DataObject>;

      // In the process of producing a DataObjectPtr from arbitrary data, we may
      // need to perform heap allocation and move stuff around. Our client wants
      // a const-reference to the final object after all this has been done.
      template<typename T>
      using DataObjectAndRef = std::pair<DataObjectPtr, const T&>;

      // We received a heap-allocated DataObject. The TES is fine with that.
      template<typename T>
      DataObjectAndRef<T> wrapDataObject(DataObjectPtr&& ptr) {
        const T& ref = static_cast<const T&>(*ptr);
        return { std::move(ptr), ref };
      }

      // We received a DataObject rvalue. It must be moved to the heap first.
      template<typename T>
      DataObjectAndRef<T> wrapDataObject(DataObject&& data) {
        return wrapDataObject<T>(
          std::make_unique<T>(static_cast<T&&>(data))
        );
      }

      // This is not a DataObject, we must wrap it and move it to the heap.
      template<typename T, ENABLE_IF(!isDataObject<T>())>
      DataObjectAndRef<T> wrapDataObject(T&& data) {
        auto ptr = std::make_unique<AnyDataWrapper<T>>(std::forward<T>(data));
        const T& ref = ptr->getData();
        return { std::move(ptr), ref };
      }

      // Better not keep this macro around now that we're done...
#undef ENABLE_IF
    }
  }
}

#endif
