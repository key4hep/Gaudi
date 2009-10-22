// $Id: GetData.h,v 1.1 2008/10/10 08:06:33 marcocle Exp $
// ============================================================================
#ifndef GAUDIUTILS_GETDATA_H
#define GAUDIUTILS_GETDATA_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
// ============================================================================
// GaudiUtils
// ============================================================================
#include "GaudiKernel/Range.h"
#include "GaudiKernel/NamedRange.h"
// ============================================================================
// Forward declaration
// ============================================================================
template <class BASE> class GaudiCommon ; // GaudiAlg
// ============================================================================
namespace Gaudi
{
  namespace Utils
  {
    // ========================================================================
    /** @struct _GetType
     *  Helper structure to define the proper return type for
     *  "get"-functions
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-07-22
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
    /** @struct GetData GetData.h GaudiUtils/GetData.h
     *
     *  Helper structure for implementation of  "get"-functions for
     *  GaudiCommon<BASE>
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-07-22
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
       *  @param common the actual "worker"
       *  @param service pointer to Data Provider Service
       *  @param location location of objects in TES
       *  @return the data
       */
      template <class COMMON>
      inline return_type operator()
        ( const COMMON&            common    ,
          IDataProviderSvc*        service   ,
          const std::string&       location  ) const
      {
        /// use Data Provider Service
        SmartDataPtr<TYPE> obj ( service , location ) ;
        return_type aux = obj ;
        /// check the data
        common.Assert ( !(!aux) , "get():: No valid data at '" + location + "'"  ) ;
        /// debug printout
        if ( common.msgLevel ( MSG::DEBUG ) )
        { common.debug() << "The object of type '"
                         << System::typeinfoName(typeid(aux))
                         << "' has been retrieved from TS at address '"
                         << location << "'" << endmsg ; }
        // return located *VALID* data
        return aux ;
        // ======================================================================
      }
    };
    // ========================================================================
    /// the template specialization for ranges
    template <class TYPE>
    struct GetData<Gaudi::Range_<std::vector<const TYPE*> > >
    {
    public:
      // ======================================================================
      /// the actual return type
      typedef Gaudi::Range_<std::vector<const TYPE*> >   Type        ;
      typedef typename _GetType<Type>::return_type       return_type ;
      // ======================================================================
    public:
      // ======================================================================
      /** the only one essential method
       *  @param common the actual "worker"
       *  @param service pointer to Data Provider Service
       *  @param location location of objects in TES
       *  @return the data
       */
      template <class COMMON>
      inline return_type operator()
        ( const COMMON&            common    ,
          IDataProviderSvc*        service   ,
          const std::string&       location  ) const
      {
        { // try to get the selection from TES
          SmartDataPtr<typename TYPE::Selection> obj ( service , location ) ;
          typename TYPE::Selection* aux = obj ;
          if ( 0 != aux )
          {
            if ( common.msgLevel ( MSG::DEBUG ) )
            { common.debug() << "The object of type '"
                             << System::typeinfoName(typeid(*aux))
                             << "' has been retrieved from TS at address '"
                             << location << "'" << endmsg ; }
            return make_range ( aux->begin() , aux->end() ) ;
          }
        }
        { // get from TES the container
          SmartDataPtr<typename TYPE::Container> obj ( service , location ) ;
          typename TYPE::Container* aux = obj ;
          if ( 0 != aux )
          {
            if ( common.msgLevel ( MSG::DEBUG ) )
            { common.debug() << "The object of type '"
                             << System::typeinfoName(typeid(*aux))
                             << "' has been retrieved from TS at address '"
                             << location << "'" << endmsg ; }
            return make_range ( aux->begin() , aux->end() ) ;
          }
        }
        // no valid data
        common.Assert ( false , "get():: No valid data at '" + location + "'"  ) ;
        // the fictive return
        return return_type () ;
      }
      // ======================================================================
    private:
      // ======================================================================
      template <class ITERATOR>
      return_type make_range
      ( ITERATOR first ,
        ITERATOR last  ) const
      {
        typename return_type::const_iterator* _begin = reinterpret_cast<typename return_type::const_iterator*>(&first);
        typename return_type::const_iterator* _end   = reinterpret_cast<typename return_type::const_iterator*>(&last);
        return return_type(*_begin, *_end);
      }
      // ======================================================================
    } ;
    // ========================================================================
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
       *  @param common the actual "worker"
       *  @param service pointer to Data Provider Service
       *  @param location location of objects in TES
       *  @return the data
       */
      template <class COMMON>
      inline return_type operator()
        ( const COMMON&            common    ,
          IDataProviderSvc*        service   ,
          const std::string&       location  ) const
      {
        { // try to get the selection from TES
          SmartDataPtr<typename TYPE::Selection> obj ( service , location ) ;
          typename TYPE::Selection* aux = obj ;
          if ( 0 != obj )
          {
            if ( common.msgLevel ( MSG::DEBUG ) )
            { common.debug() << "The object of type '"
                             << System::typeinfoName(typeid(*aux))
                             << "' has been retrieved from TS at address '"
                             << location << "'" << endmsg ; }
            return make_range ( aux -> begin() , aux -> end() , location ) ;
          }
        }
        { // get from TES the container
          SmartDataPtr<typename TYPE::Container> obj ( service , location ) ;
          typename TYPE::Container* aux = obj ;
          if ( 0 != obj )
          {
            if ( common.msgLevel ( MSG::DEBUG ) )
            { common.debug() << "The object of type '"
                             << System::typeinfoName(typeid(*aux))
                             << "' has been retrieved from TS at address '"
                             << location << "'" << endmsg ; }
            return make_range ( aux -> begin() , aux -> end() , location ) ;
          }
        }
        // no valid data
        common.Assert ( false , "get():: No valid data at '" + location + "'"  ) ;
        // the fictive return
        return return_type () ;
      }
      // ======================================================================
    private:
      // ======================================================================
      template <class ITERATOR>
      return_type make_range
      ( ITERATOR           first    ,
        ITERATOR           last     ,
        const std::string& location ) const
      {
        typename return_type::const_iterator* _begin = reinterpret_cast<typename return_type::const_iterator*>(&first);
        typename return_type::const_iterator* _end   = reinterpret_cast<typename return_type::const_iterator*>(&last);
        return return_type(*_begin, *_end, location);
      }
      // ======================================================================
    } ;
    // ========================================================================
    /// the template specialization for pointer types
    template <class TYPE>
    struct GetData<TYPE*> : public GetData<TYPE> {} ;
    // ========================================================================
    /// the template specialization for reference types
    template <class TYPE>
    struct GetData<TYPE&> : public GetData<TYPE> {} ;
    // ========================================================================
  } // end of namespace Gaudi::Utils
  // ==========================================================================
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIUTILS_GETDATA_H
// ============================================================================
