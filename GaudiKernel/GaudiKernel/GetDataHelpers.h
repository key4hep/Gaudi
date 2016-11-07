#ifndef GAUDIKERNEL_GETDATAHELPER_H
#define GAUDIKERNEL_GETDATAHELPER_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Range.h"
#include "GaudiKernel/NamedRange.h"
#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/AnyDataWrapper.h"
// ============================================================================
namespace Gaudi
{
  namespace Helpers
  {
    // ========================================================================
    /** @struct _GetType
     *  Helper structure to define the proper return type for
     *  "get"-functions
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     */
    template <class TYPE>
    struct _GetType
    { using return_type = TYPE* ; };
    // ========================================================================
    /// the template specialization for pointers
    template <class TYPE>
    struct _GetType<TYPE*>
    { using return_type = TYPE* ; };
    // ========================================================================
    /// the template specialization for references
    template <class TYPE>
    struct _GetType<TYPE&>
    { using return_type = TYPE* ; };
    // ========================================================================


    // ========================================================================
    /** @struct GetData GaudiKernel/GetDataHelpers.h
     *
     *  Helper structure for implementation of  "get"-functions for
     *  GaudiCommon<BASE>
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     */
    template <class TYPE>
    class GetData {
    public:
      using Type = TYPE;
      /// the actual return type
      using return_type = typename _GetType<Type>::return_type;
    public:
      /** the only one essential method
       *  @param base  ref to the Handle base to retrieve the data from
       *  @return the data
       */
      inline return_type operator()(DataObject *obj) const {
        return dynamic_cast<return_type>(obj);
      }
    };

    /// Some helper methods for the Range/namedRange specialization
    template <class container_type,
              typename std::enable_if<std::is_base_of<DataObject,container_type>::value>::type>
    inline container_type& unwrap_data(container_type* content_ptr) {
      return *content_ptr;
    }

    template <class container_type>//,
      //typename std::enable_if<!std::is_base_of<DataObject,container_type>::value>::type>
    inline container_type& unwrap_data(AnyDataWrapper<container_type>* wrapper_ptr) {
      return wrapper_ptr->getData();
    }

    template <typename range_type>
    inline range_type getData_operator(DataObject *obj) {
      /// the actual types in the TES
      using stored_type = typename std::conditional<std::is_base_of<DataObject, typename range_type::Container>::value,
                                                    typename range_type::Container,
                                                    AnyDataWrapper<typename range_type::Container>>::type;
      const auto tmp = dynamic_cast<stored_type*>(obj);
      if (tmp) {
        auto& cont = unwrap_data<typename range_type::Container>(tmp);
        return range_type(cont.begin(), cont.end());
      }
      return range_type();
    }

    /// the template specialization for named ranges
    template <class container_type>
    struct GetData<Gaudi::NamedRange_<container_type>> {
      using range_type = Gaudi::NamedRange_<container_type>;
      inline range_type operator()(DataObject *obj) const {
        return getData_operator<range_type>(obj);
      }
    };

    /// the template specialization for ranges
    template <class container_type>
    struct GetData<Gaudi::Range_<container_type>> {
      using range_type = Gaudi::Range_<container_type>;
      inline range_type operator()(DataObject *obj) const {
        return getData_operator<range_type>(obj);
      }
    } ;


    // ========================================================================
  } //                                          end of namespace Gaudi::Helpers
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
//                                                                      The END
// ============================================================================
#endif // GAUDIKERNEL_GETDATAHELPER_H
// ============================================================================
