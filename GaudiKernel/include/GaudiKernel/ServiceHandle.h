/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_SERVICEHANDLE_H
#define GAUDIKERNEL_SERVICEHANDLE_H

// Includes
#include <GaudiKernel/Bootstrap.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/GaudiHandle.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/ServiceLocatorHelper.h>

#include <stdexcept>
#include <string>
#include <type_traits>

// class predeclarations
class IAlgTool;
class IToolSvc;
class ServiceHandleProperty;

/** @class ServiceHandle ServiceHandle.h GaudiKernel/ServiceHandle.h

    Handle to be used in lieu of naked pointers to services. This allows
    better control through the framework of service loading and usage.

    @author Martin.Woudstra@cern.ch
*/
template <class T>
class ServiceHandle : public GaudiHandle<T> {
public:
  //
  // Constructors etc.
  //
  /** Create a handle ('smart pointer') to a service.
      The arguments are passed on to ServiceSvc, and have the same meaning:
      @param serviceName name of the service
      @param parentName name of the parent Algorithm, AlgTool or Service.
             It is used for log printout at retrieve(), and for retrieving
             a thread-dependent service (if applicable)
  */
  ServiceHandle( const std::string& serviceName, const std::string& theParentName )
      : GaudiHandle<T>( serviceName, "Service", theParentName ) {}

  /** Copy constructor from a non const T to const T service handle */
  template <typename CT = T, typename NCT = std::remove_const_t<T>,
            typename = std::enable_if_t<std::is_const_v<CT> && !std::is_same_v<CT, NCT>>>
  ServiceHandle( const ServiceHandle<NCT>& other ) : GaudiHandle<CT>( other ) {}

  /// Autodeclaring constructor with property name, service type/name and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <class OWNER, typename = std::enable_if_t<std::is_base_of_v<IProperty, OWNER>>>
  inline ServiceHandle( OWNER* owner, std::string PropName, const std::string& svcName, std::string doc = "" )
      : ServiceHandle( svcName, owner->name() ) {
    auto p = owner->OWNER::PropertyHolderImpl::declareProperty( std::move( PropName ), *this, std::move( doc ) );
    p->template setOwnerType<OWNER>();
  }

  StatusCode initialize( const std::string& serviceName, const std::string& theParentName ) {

    GaudiHandleBase::setTypeAndName( serviceName );
    GaudiHandleBase::setParentName( theParentName );

    return StatusCode::SUCCESS;
  }

  /** Retrieve the Service. Release existing Service if needed.
      Function must be repeated here to avoid hiding the function retrieve( T*& ) */
  using GaudiHandle<T>::retrieve;

  //  /** Release the Service.
  //    Function must be repeated here to avoid hiding the function release( T*& ) */
  //   StatusCode release() const { // not really const, because it updates m_pObject
  //     return GaudiHandle<T>::release();
  //   }

  /// Allow non const access to the service, even from a const handle...
  T* get() const { return ::details::nonConst( GaudiHandle<T>::get() ); }

  /// Allow non const access to the service, even from a const handle...
  T* operator->() const { return ::details::nonConst( GaudiHandle<T>::operator->() ); }
  T& operator*() const { return *::details::nonConst( GaudiHandle<T>::operator->() ); }

protected:
  /** Do the real retrieval of the Service. */
  StatusCode retrieve( T*& service ) const override {
    const ServiceLocatorHelper helper( *serviceLocator(), GaudiHandleBase::messageName(), this->parentName() );
    return helper.getService( GaudiHandleBase::typeAndName(), true, T::interfaceID(), (void**)&service );
  }

  //   /** Do the real release of the Service */
  //   virtual StatusCode release( T* service ) const {
  //     return service->release();
  //   }

private:
  //
  // Private helper functions
  //
  SmartIF<ISvcLocator>& serviceLocator() const { // not really const, because it may change m_pSvcLocator
    if ( !m_pSvcLocator ) {
      m_pSvcLocator = Gaudi::svcLocator();
      if ( !m_pSvcLocator ) {
        throw GaudiException( "SvcLocator not found", "Core component not found", StatusCode::FAILURE );
      }
    }
    return m_pSvcLocator;
  }

  SmartIF<IMessageSvc>& messageSvc() const { // not really const, because it may change m_pMessageSvc
    if ( !m_pMessageSvc ) {
      m_pMessageSvc = serviceLocator(); // default message service
      if ( !m_pMessageSvc ) {
        throw GaudiException( "Service [MessageSvc] not found", this->parentName(), StatusCode::FAILURE );
      }
    }
    return m_pMessageSvc;
  }
  //
  // private data members
  //
  mutable SmartIF<ISvcLocator> m_pSvcLocator;
  mutable SmartIF<IMessageSvc> m_pMessageSvc;
};

/** @class ServiceHandleArray ServiceHandle.h GaudiKernel/ServiceHandle.h

    Array of Handles to be used in lieu of vector of naked pointers to tools.
    This allows better control through the framework of tool loading and usage.
    @parameter T is the AlgTool interface class (or concrete class) of
    the tool to use, and must derive from IAlgTool.

    @author Yushu Yao <yyao@lbl.gov>
*/

template <class T>
class ServiceHandleArray : public GaudiHandleArray<ServiceHandle<T>> {
public:
  //
  // Constructors
  //
  /** Generic constructor. Probably not very useful...
   **/
  ServiceHandleArray( const std::vector<std::string>& myTypesAndNamesList, const std::string& myComponentType,
                      const std::string& myParentName )
      : GaudiHandleArray<ServiceHandle<T>>( myTypesAndNamesList, myComponentType, myParentName ) {}

  virtual ~ServiceHandleArray() {}

  ServiceHandleArray( const std::string& myParentName )
      : GaudiHandleArray<ServiceHandle<T>>( "Service", myParentName ) {}

  virtual bool push_back( const std::string& serviceTypeAndName ) {
    ServiceHandle<T> handle( serviceTypeAndName, GaudiHandleInfo::parentName() );
    GaudiHandleArray<ServiceHandle<T>>::push_back( handle );
    return true;
  }

  virtual bool push_back( const ServiceHandle<T>& myHandle ) { return push_back( myHandle.typeAndName() ); }
};

template <class T>
inline std::ostream& operator<<( std::ostream& os, const ServiceHandle<T>& handle ) {
  return operator<<( os, static_cast<const GaudiHandleInfo&>( handle ) );
}

template <class T>
inline std::ostream& operator<<( std::ostream& os, const ServiceHandleArray<T>& handle ) {
  return operator<<( os, static_cast<const GaudiHandleInfo&>( handle ) );
}

#endif // ! GAUDIKERNEL_SERVICEHANDLE_H
