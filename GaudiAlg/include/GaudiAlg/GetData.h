/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIUTILS_GETDATA_H
#define GAUDIUTILS_GETDATA_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// GaudiUtils
// ============================================================================
#include "GaudiKernel/AnyDataWrapper.h"
#include "GaudiKernel/NamedRange.h"
#include "GaudiKernel/Range.h"
// ============================================================================
// Forward declaration
// ============================================================================
template <class BASE>
class GaudiCommon; // GaudiAlg
// ============================================================================
namespace Gaudi {
  namespace Utils {
    // ========================================================================
    /** @struct _GetType
     *  Helper structure to define the proper return type for
     *  "get"-functions
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-07-22
     */
    template <class TYPE>
    struct _GetType {
      typedef TYPE* return_type;
    };
    // ========================================================================
    /// the template specialization for pointers
    template <class TYPE>
    struct _GetType<TYPE*> {
      typedef TYPE* return_type;
    };
    // ========================================================================
    /// the template specialization for references
    template <class TYPE>
    struct _GetType<TYPE&> {
      typedef TYPE* return_type;
    };
    // ========================================================================
    /// the template specialization for "ranges"
    template <class CONTAINER>
    struct _GetType<Gaudi::Range_<CONTAINER>> {
      typedef Gaudi::Range_<CONTAINER> return_type;
    };
    // ========================================================================
    /// the template specialization for "named ranges"
    template <class CONTAINER>
    struct _GetType<Gaudi::NamedRange_<CONTAINER>> {
      typedef Gaudi::NamedRange_<CONTAINER> return_type;
    };
    // ========================================================================
    /** Helper function to provide the minimal lookup and cast functionality
     *  of SmartDataPtr used in the helper classes.
     *  This version is dealing with the general case, where AnyDataWrapper cannot be used
     */
    template <class TYPE, std::enable_if_t<!std::is_constructible_v<TYPE>, void*> = nullptr>
    typename _GetType<TYPE>::return_type getFromTS( IDataProviderSvc* service, std::string_view location ) {
      DataObject* obj = nullptr;
      // Return the casted pointer if the retrieve was successful or nullptr otherwise.
      return service->retrieveObject( location, obj ).isSuccess()
                 ? dynamic_cast<typename _GetType<TYPE>::return_type>( obj )
                 : nullptr;
    }
    // ========================================================================
    /** Helper function to provide the minimal lookup and cast functionality
     *  of SmartDataPtr used in the helper classes.
     *  This version allows the use of AnyDataWrapper but restricts to the case
     *  where TYPE is constructible
     */
    template <class TYPE, std::enable_if_t<std::is_constructible_v<TYPE>, void*> = nullptr>
    typename _GetType<TYPE>::return_type getFromTS( IDataProviderSvc* service, std::string_view location ) {
      DataObject* obj = nullptr;
      // Return the casted pointer if the retrieve was successful or nullptr otherwise.
      StatusCode sc = service->retrieveObject( location, obj );
      if ( sc.isFailure() ) return nullptr;
      auto tobj = dynamic_cast<typename _GetType<TYPE>::return_type>( obj );
      if ( !tobj ) {
        // try with AnyDataWrapper
        if ( auto tobj2 = dynamic_cast<AnyDataWrapper<TYPE>*>( obj ); tobj2 ) { tobj = &( tobj2->getData() ); }
      }
      return tobj;
    }
    // ========================================================================
    /** @struct GetData GaudiUtils/GetData.h
     *
     *  Helper structure for implementation of  "get"-functions for
     *  GaudiCommon<BASE>
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-07-22
     */
    template <class TYPE>
    struct GetData {
    public:
      // ======================================================================
      typedef TYPE Type;
      /// the actual return type
      typedef typename _GetType<Type>::return_type return_type;
      // ======================================================================
    public:
      // ======================================================================
      /** the only one essential method
       *  @param common the actual "worker"
       *  @param service pointer to Data Provider Service
       *  @param location location of objects in TES
       *  @param checkData whether to check if the pointer is valid before returning it
       *  @return the data
       */
      template <class COMMON>
      return_type operator()( const COMMON& common, IDataProviderSvc* service, std::string_view location,
                              const bool checkData = true ) const {
        // use Data Provider Service
        return_type obj = getFromTS<Type>( service, location );
        if ( checkData ) { // check the data
          common.Assert( obj, std::string{ "get():: No valid data at '" }.append( location ).append( "'" ) );
        }
        // debug printout
        if ( common.msgLevel( MSG::DEBUG ) ) {
          common.debug() << "The object of type '" << System::typeinfoName( typeid( obj ) ) << "' "
                         << ( obj ? "has been" : "could not be" ) << " retrieved from TS at address '" << location
                         << "'" << endmsg;
        }
        // return located data
        return obj;
        // ======================================================================
      }
    };
    // ========================================================================
    /// the template specialization for ranges
    template <class TYPE>
    struct GetData<Gaudi::Range_<std::vector<const TYPE*>>> {
    public:
      // ======================================================================
      /// the actual return type
      typedef Gaudi::Range_<std::vector<const TYPE*>> Type;
      typedef typename _GetType<Type>::return_type    return_type;
      // ======================================================================
    public:
      // ======================================================================
      /** the only one essential method
       *  @param common the actual "worker"
       *  @param service pointer to Data Provider Service
       *  @param location location of objects in TES
       *  @param checkData whether to check if the pointer is valid before returning it
       *  @return the data
       */
      template <class COMMON>
      return_type operator()( const COMMON& common, IDataProviderSvc* service, std::string_view location,
                              const bool checkData = true ) const {
        /// try to be efficient:
        /// 1. load object only once:
        DataObject* object = this->getData( service, std::string{ location } );
        if ( object ) {
          /// 2. try to get the selection
          typedef typename TYPE::Selection Selection_;
          const Selection_*                sel = dynamic_cast<Selection_*>( object );
          if ( sel ) {
            if ( common.msgLevel( MSG::DEBUG ) ) {
              common.debug() << "The object of type '" << System::typeinfoName( typeid( *object ) )
                             << "' has been retrieved from TS at address '" << location << "'" << endmsg;
            }
            return make_range( sel );
          }
          /// 3. try to get the container
          typedef typename TYPE::Container Container_;
          const Container_*                cnt = dynamic_cast<Container_*>( object );
          if ( cnt ) {
            if ( common.msgLevel( MSG::DEBUG ) ) {
              common.debug() << "The object of type '" << System::typeinfoName( typeid( *object ) )
                             << "' has been retrieved from TS at address '" << location << "'" << endmsg;
            }
            return make_range( cnt );
          }
          // no valid data
          if ( checkData )
            common.Assert( false, std::string{ "get():: No valid data at '" }.append( location ).append( "'" ) );
        }
        // no valid data
        if ( checkData ) common.Assert( false, std::string{ "get():: No data at '" }.append( location ).append( "'" ) );
        // the fictive return
        return return_type();
      }
      // ======================================================================
    public:
      // ======================================================================
      // create the range from the container
      return_type make_range( const typename TYPE::Container* cnt ) const {
        return 0 == cnt ? return_type() : make_range( cnt->begin(), cnt->end() );
      }
      // create the range from the selection
      return_type make_range( const typename TYPE::Selection* cnt ) const {
        return 0 == cnt ? return_type() : return_type( cnt->begin(), cnt->end() );
      }
      // ======================================================================
      /** get the data form transient store
       *  @param service   pointer to data provider service
       *  @param location  the location
       *  @return the object for TES
       */
      DataObject* getData( IDataProviderSvc* service, std::string location ) const {
        /// Try to be efficient
        SmartDataObjectPtr getter( SmartDataObjectPtr::ObjectLoader::access(), service, nullptr,
                                   std::move( location ) );
        return getter.accessData();
      }
      // ======================================================================
    private:
      // ======================================================================
      template <class ITERATOR>
      return_type make_range( ITERATOR first, ITERATOR last ) const {
        auto _begin = reinterpret_cast<typename return_type::const_iterator*>( &first );
        auto _end   = reinterpret_cast<typename return_type::const_iterator*>( &last );
        return return_type( *_begin, *_end );
      }
      // ======================================================================
    };
    // ========================================================================
    /// the template specialization for named ranges
    template <class TYPE>
    struct GetData<Gaudi::NamedRange_<std::vector<const TYPE*>>> {
    public:
      // ======================================================================
      /// the actual return type
      typedef Gaudi::NamedRange_<std::vector<const TYPE*>> Type;
      typedef typename _GetType<Type>::return_type         return_type;
      // ======================================================================
    public:
      // ======================================================================
      /** the only one essential method
       *  @param common the actual "worker"
       *  @param service pointer to Data Provider Service
       *  @param location location of objects in TES
       *  @param checkData whether to check if the pointer is valid before returning it
       *  @return the data
       */
      template <class COMMON>
      return_type operator()( const COMMON& common, IDataProviderSvc* service, std::string_view location,
                              const bool checkData = true ) const {
        return return_type( m_range( common, service, location, checkData ), std::string{ location } );
      }
      // ======================================================================
    public:
      // ======================================================================
      // create the range from the container
      return_type make_range( const typename TYPE::Container* cnt ) const {
        if ( !cnt ) { return return_type(); }
        static const std::string s_empty = "";
        const IRegistry*         reg     = cnt->registry();
        return return_type( m_range.make_range( cnt ), reg ? reg->identifier() : s_empty );
      }
      // create the range from the selection
      return_type make_range( const typename TYPE::Selection* cnt ) const {
        if ( !cnt ) { return return_type(); }
        static const std::string s_empty = "";
        const IRegistry*         reg     = cnt->registry();
        return return_type( m_range.make_range( cnt ), reg ? reg->identifier() : s_empty );
      }
      // ======================================================================
      /** get the data form transient store
       *  @param service   pointer to data provider service
       *  @param location  the location
       *  @return the object for TES
       */
      DataObject* getData( IDataProviderSvc* service, std::string location ) const {
        return m_range.getData( service, std::move( location ) );
      }
      // ======================================================================
    private:
      /// =====================================================================
      /// the actual processor
      GetData<Gaudi::Range_<std::vector<const TYPE*>>> m_range;
      // ======================================================================
    };
    // ========================================================================
    /// the template specialization for const types
    template <class TYPE>
    struct GetData<const TYPE> : public GetData<TYPE> {};
    // ========================================================================
    /// the template specialization for pointer types
    template <class TYPE>
    struct GetData<TYPE*> : public GetData<TYPE> {};
    // ========================================================================
    /// the template specialization for reference types
    template <class TYPE>
    struct GetData<TYPE&> : public GetData<TYPE> {};
    // ========================================================================
    /** @struct CheckData  GaudiAlg/GetData.h
     *
     *  Helper structure for implementation of  "exists"-functions for
     *  GaudiCommon<BASE>
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-07-22
     */
    template <class TYPE>
    struct CheckData {
    public:
      // ======================================================================
      /** the only one essential method
       *  @param common the actual "worker"
       *  @param service pointer to Data Provider Service
       *  @param location location of objects in TES
       *  @return true for valid data
       */
      bool operator()( IDataProviderSvc* service, std::string_view location ) const {
        /// use Data Provider Service
        return getFromTS<TYPE>( service, location );
      }
      // ======================================================================
    };
    // ========================================================================
    /// the template specialization for ranges
    template <class TYPE>
    struct CheckData<Gaudi::Range_<std::vector<const TYPE*>>> {
    public:
      // ======================================================================
      /** the only one essential method
       *  @param common the actual "worker"
       *  @param service pointer to Data Provider Service
       *  @param location location of objects in TES
       *  @return true for valid data
       */
      bool operator()( IDataProviderSvc* service, std::string location ) const {
        DataObject* object = this->getData( service, std::move( location ) );
        return object && ( dynamic_cast<typename TYPE::Selection*>( object ) ||
                           dynamic_cast<typename TYPE::Container*>( object ) );
      }
      // ======================================================================
    protected:
      // ======================================================================
      /** get the data form transient store
       *  @param service   pointer to data provider service
       *  @param location  the location
       *  @return the object for TES
       */
      DataObject* getData( IDataProviderSvc* service, std::string location ) const {
        /// Try to be efficient
        SmartDataObjectPtr getter( SmartDataObjectPtr::ObjectLoader::access(), service, nullptr,
                                   std::move( location ) );
        return getter.accessData();
      }
      // ======================================================================
    };
    // ========================================================================
    /// the template specialization for ranges
    template <class TYPE>
    struct CheckData<Gaudi::NamedRange_<std::vector<const TYPE*>>>
        : public CheckData<Gaudi::Range_<std::vector<const TYPE*>>> {};
    // ========================================================================
    /// the template specialization for pointer types
    template <class TYPE>
    struct CheckData<TYPE*> : public CheckData<TYPE> {};
    // ========================================================================
    /// the template specialization for reference types
    template <class TYPE>
    struct CheckData<TYPE&> : public CheckData<TYPE> {};
    // ========================================================================
    /// the template specialization for 'const'-type
    template <class TYPE>
    struct CheckData<const TYPE> : public CheckData<TYPE> {};
    // ========================================================================
    /** @struct GetOrCreateData GaudiUtils/GetData.h
     *
     *  Helper structure for implementation of  "getOrCreate"-functions for
     *  GaudiCommon<BASE>
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-07-22
     */
    template <class TYPE, class TYPE2>
    struct GetOrCreateData {
    private:
      // ======================================================================
      /// the actual data getter
      typedef GetData<TYPE> Getter; // the actual data getter
      // ======================================================================
    public:
      // ======================================================================
      typedef typename Getter::Type Type;
      /// the actual return type
      typedef typename Getter::return_type return_type;
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
      return_type operator()( const COMMON& common, IDataProviderSvc* service, std::string_view location,
                              std::string_view location2 ) const {
        SmartDataPtr<TYPE> obj( service, std::string{ location } );
        if ( !obj ) {
          auto o = std::make_unique<TYPE2>();
          auto r = o.get();
          common.put( service, std::move( o ), location2 );
          if ( common.msgLevel( MSG::DEBUG ) ) {
            common.debug() << "The object of type '" << System::typeinfoName( typeid( *r ) )
                           << "' has been created from TS at address '" << location2 << "'" << endmsg;
          }
          return r;
        }
        auto ret = obj.ptr();
        /// check the data
        common.Assert( !( !ret ), std::string{ "get():: No valid data at '" }.append( location ).append( "\'" ) );
        if ( common.msgLevel( MSG::DEBUG ) ) {
          common.debug() << "The object of type '" << System::typeinfoName( typeid( *ret ) )
                         << "' has been retrieved from TS at address '" << location << "'" << endmsg;
        }
        // return *VALID* data
        return ret;
        // ====================================================================
      }
    };
    // ========================================================================
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<Gaudi::Range_<std::vector<const TYPE*>>, TYPE2> {
    private:
      // ======================================================================
      typedef Gaudi::Range_<std::vector<const TYPE*>> Range;
      /// the actual data getter
      typedef GetData<Range> Getter; //  the actual data getter
      /// the actual data checker
      typedef CheckData<Range> Checker; // the actual data checker
      // ======================================================================
    public:
      // ======================================================================
      typedef typename Getter::Type Type;
      /// the actual return type
      typedef typename Getter::return_type return_type;
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
      return_type operator()( const COMMON& common, IDataProviderSvc* service, std::string_view location,
                              std::string_view location2 ) const {
        DataObject* obj = m_getter.getData( service, std::string{ location } );
        if ( !obj ) {
          common.put( service, std::make_unique<TYPE2>(), location2 );
          if ( common.msgLevel( MSG::DEBUG ) ) {
            common.debug() << "The object of type '" << System::typeinfoName( typeid( TYPE2 ) )
                           << "' has been created from TS at address '" << location2 << "'" << endmsg;
          }
        }
        return m_getter( common, service, location );
        // ====================================================================
      }
      // ======================================================================
    private:
      // ======================================================================
      /// the actual data getter
      Getter m_getter; // the actual data getter
      // ======================================================================
    };
    // ========================================================================
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<Gaudi::NamedRange_<std::vector<const TYPE*>>, TYPE2> {
    private:
      // ======================================================================
      typedef Gaudi::NamedRange_<std::vector<const TYPE*>> Range;
      typedef Gaudi::Range_<std::vector<const TYPE*>>      Range_;
      typedef GetOrCreateData<Range_, TYPE2>               Helper;
      /// the actual data getter
      typedef GetData<Range> Getter; //  the actual data getter
      // ======================================================================
    public:
      // ======================================================================
      typedef typename Getter::Type Type;
      /// the actual return type
      typedef typename Getter::return_type return_type;
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
      return_type operator()( const COMMON& common, IDataProviderSvc* service, std::string location,
                              std::string_view location2 ) const {
        auto range = m_range( common, service, location, location2 );
        return return_type( std::move( range ), std::move( location ) );
      }
      // ======================================================================
    private:
      // ======================================================================
      /// the actual data getter
      Helper m_range; // the actual data getter
      // ======================================================================
    };
    // ========================================================================
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<TYPE, TYPE2*> : public GetOrCreateData<TYPE, TYPE2> {};
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<TYPE*, TYPE2> : public GetOrCreateData<TYPE, TYPE2> {};
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<TYPE*, TYPE2*> : public GetOrCreateData<TYPE, TYPE2> {};
    // ========================================================================
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<TYPE, const TYPE2> : public GetOrCreateData<TYPE, TYPE2> {};
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<const TYPE, TYPE2> : public GetOrCreateData<TYPE, TYPE2> {};
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<const TYPE, const TYPE2> : public GetOrCreateData<TYPE, TYPE2> {};
    // ========================================================================
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<TYPE, TYPE2&> : public GetOrCreateData<TYPE, TYPE2> {};
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<TYPE&, TYPE2> : public GetOrCreateData<TYPE, TYPE2> {};
    template <class TYPE, class TYPE2>
    struct GetOrCreateData<TYPE&, TYPE2&> : public GetOrCreateData<TYPE, TYPE2> {};
    // ========================================================================
  } // namespace Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
//                                                                      The END
// ============================================================================
#endif // GAUDIUTILS_GETDATA_H
