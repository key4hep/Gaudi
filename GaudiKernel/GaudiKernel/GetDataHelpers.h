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
    { typedef TYPE* return_type ; };
    // ========================================================================
    /// the template specialization for pointers
    template <class TYPE>
    struct _GetType<TYPE*>
    { typedef TYPE* return_type ; };
    // ========================================================================
    /// the template specialization for references
    template <class TYPE>
    struct _GetType<TYPE&>
    { typedef TYPE* return_type ; };
    // ========================================================================
    /// the template specialization for "ranges"
    template <class CONTAINER>
    struct _GetType<Gaudi::Range_<CONTAINER> >
    { typedef Gaudi::Range_<CONTAINER>      return_type ; };
    // ========================================================================
    /// the template specialization for "named ranges"
    template <class CONTAINER>
    struct _GetType<Gaudi::NamedRange_<CONTAINER> >
    { typedef Gaudi::NamedRange_<CONTAINER> return_type ; };
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
      typedef TYPE                                   Type        ;
      /// the actual return type
      typedef typename _GetType<Type>::return_type   return_type ;
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
    template <class data_type, typename wrapper_type>
    inline data_type& unwrap_data(wrapper_type* wrapper) {
      return *wrapper;
    }

    template <class data_type, typename wrapper_type,
              typename std::enable_if<!std::is_base_of<DataObject,data_type>::value, data_type>::type>
    inline data_type& unwrap_data(wrapper_type* wrapper) {
      return wrapper->getData();
    }

    template <typename return_type, class ITERATOR> return_type make_range(ITERATOR first,
                                                                           ITERATOR last) {
      auto _begin = reinterpret_cast<typename return_type::const_iterator*>(&first);
      auto _end   = reinterpret_cast<typename return_type::const_iterator*>(&last);
      return return_type(*_begin, *_end);
    }

    template <class TYPE, typename return_type>
    inline return_type getData_operator(DataObject *obj) {
      /// the actual types in the TES
      typedef typename std::conditional<std::is_base_of<DataObject,typename TYPE::Selection>::value,
                                        typename TYPE::Selection,
                                        AnyDataWrapper<typename TYPE::Selection>>::type selection_type;
      typedef typename std::conditional<std::is_base_of<DataObject,typename TYPE::Container>::value,
                                        typename TYPE::Container,
                                        AnyDataWrapper<typename TYPE::Container>>::type container_type;
      const auto tmp = dynamic_cast<selection_type*>(obj);
      if (tmp) {
        auto& selection = unwrap_data<typename TYPE::Selection, selection_type>(tmp);
        return return_type(selection.begin(), selection.end());
      } else {
        const auto tmpcont = dynamic_cast<container_type*>(obj);
        if (tmpcont) {
          auto& container = unwrap_data<typename TYPE::Container, container_type>(tmpcont);
          return make_range<return_type, decltype(container.begin())>(container.begin(), container.end());
        }
      }
      return return_type();
    }

    /// the template specialization for named ranges
    template <class TYPE>
    struct GetData<Gaudi::NamedRange_<std::vector<const TYPE*> > > {
      /// the actual return type
      typedef Gaudi::NamedRange_<std::vector<const TYPE*> > Type;
      typedef typename _GetType<Type>::return_type return_type ;
      ///       *  @param DataObjectHandleBase base
      inline return_type operator()(DataObject *obj) const {
        return getData_operator<TYPE, return_type>(obj);
      }
    };

    /// the template specialization for ranges
    template <class TYPE>
    struct GetData<Gaudi::Range_<std::vector<const TYPE*> > > {
      /// the actual return type
      typedef Gaudi::Range_<std::vector<const TYPE*> > Type;
      typedef typename _GetType<Type>::return_type return_type ;
      ///       *  @param DataObjectHandleBase base
      inline return_type operator()(DataObject *obj) const {
        return getData_operator<TYPE, return_type>(obj);
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
