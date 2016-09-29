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
    struct GetData
    {
    public:
      // ======================================================================
      typedef TYPE                                   Type        ;
      /// the actual return type
      typedef typename _GetType<Type>::return_type   return_type ;
      // ======================================================================
    public:
      // ======================================================================
      /** the only one essential method
       *  @param base  ref to the Handle base to retrieve the data from
       *  @return the data
       */
      inline return_type operator()
        (DataObject *obj) const
      {
        return dynamic_cast<return_type>(obj);
      }
    };


    /// the template specialization for named ranges
    template <class TYPE>
    struct GetData<Gaudi::NamedRange_<std::vector<const TYPE*> > >
    {
    public:
      // ======================================================================
      /// the actual return type
      typedef Gaudi::NamedRange_<std::vector<const TYPE*> > Type        ;
      typedef typename _GetType<Type>::return_type          return_type ;
      // ======================================================================
    public:
      // ======================================================================
      /** the only one essential method
       *  @param DataObjectHandleBase base
       *  @return the data
       */
      inline return_type operator()
        (DataObject *obj) const
      {
        
        const auto tmp = dynamic_cast<AnyDataWrapper<typename TYPE::Selection> *>(obj);
        if (tmp) {
          return return_type(tmp->getData().begin(), tmp->getData().end());
        } else {
          const auto tmpcont = dynamic_cast<AnyDataWrapper<typename TYPE::Container> *>(obj);      
          if (tmpcont) {
            return make_range(tmpcont->getData().begin(), tmpcont->getData().end());
          }
        }
        return return_type();
      }
    private:
      
      template <class ITERATOR>
      return_type make_range
      ( ITERATOR first ,
        ITERATOR last  ) const
      {
        auto _begin = reinterpret_cast<typename return_type::const_iterator*>(&first);
        auto _end   = reinterpret_cast<typename return_type::const_iterator*>(&last);
        return return_type(*_begin, *_end);
      }
    };

    /// the template specialization for ranges
    template <class TYPE>
    struct GetData<Gaudi::Range_<std::vector<const TYPE*> > >
    {
    public:
      // ======================================================================
      /// the actual return type
      typedef Gaudi::Range_<std::vector<const TYPE*> > Type        ;
      typedef typename _GetType<Type>::return_type          return_type ;
      // ======================================================================
    public:
      // ======================================================================
      /** the only one essential method
       *  @param DataObjectHandleBase base
       *  @return the data
       */
      inline return_type operator()
        (DataObject *obj) const
      {
        
        const auto tmp = dynamic_cast<AnyDataWrapper<typename TYPE::Selection> *>(obj);
        if (tmp) {
          return return_type(tmp->getData().begin(), tmp->getData().end());
        } else {
          const auto tmpcont = dynamic_cast<AnyDataWrapper<typename TYPE::Container> *>(obj);      
          if (tmpcont) {
            return make_range(tmpcont->getData().begin(), tmpcont->getData().end());
          }
        }
        return return_type();
      }
    private:
      
      template <class ITERATOR>
      return_type make_range
      ( ITERATOR first ,
        ITERATOR last  ) const
      {
        auto _begin = reinterpret_cast<typename return_type::const_iterator*>(&first);
        auto _end   = reinterpret_cast<typename return_type::const_iterator*>(&last);
        return return_type(*_begin, *_end);
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
