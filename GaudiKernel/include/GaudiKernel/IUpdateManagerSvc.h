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
#ifndef GAUDIKERNEL_IUPDATEMANAGERSVC_H
#define GAUDIKERNEL_IUPDATEMANAGERSVC_H 1

// Include files
// from STL
#include <string>
#include <typeinfo>

// from Gaudi
#include "GaudiKernel/IInterface.h"

// forward declarations
class DataObject;
class ValidDataObject;
class IUpdateManagerSvc;
class IDataProviderSvc;
class IDetDataSvc;
namespace Gaudi {
  class Time;
}

/** @class BaseObjectMemberFunction
 *
 * Base class of ObjectMemberFunction. It is used to allow to use containers of
 * different types of object member functions. \see ObjectMemberFunction for details.
 *
 * @author Marco Clemencic
 */
class BaseObjectMemberFunction {
public:
  /// Virtual destructor.
  virtual ~BaseObjectMemberFunction() = default;

  virtual StatusCode operator()() const = 0;

  virtual BaseObjectMemberFunction* makeCopy() const = 0;

  virtual const std::type_info& type() const = 0;

  virtual bool match( BaseObjectMemberFunction* ) const = 0;

  virtual DataObject*      castToDataObject() const      = 0;
  virtual ValidDataObject* castToValidDataObject() const = 0;
  virtual void*            castToVoid() const            = 0;
};

/** @class ObjectMemberFunction
 *
 * This class is used by IUpdateManagerSvc to keep pairs made of a member function and a pointer
 * to the object for which that member function has to be called.
 *
 * @author Marco Clemencic
 */
template <class CallerClass>
class ObjectMemberFunction final : public BaseObjectMemberFunction {
public:
  /// MemberFunctionType is the type for a pointer to a member function of class CallerClass.
  typedef StatusCode ( CallerClass::*MemberFunctionType )();

  /// Calls the member function of the object and returns the StatusCode.
  /// If the pointer to the member function is nullptr, do nothing and return success.
  StatusCode operator()() const override {
    return m_memberFunction ? ( m_instance->*m_memberFunction )() : StatusCode::SUCCESS;
  }

  /// Clone method to be able to copy an ObjectMemberFunction from the BaseObjectMemberFunction
  /// interface.
  BaseObjectMemberFunction* makeCopy() const override {
    return new ObjectMemberFunction{ m_instance, m_memberFunction };
  }

  /// Returns the type_info of the CallerClass
  const std::type_info& type() const override { return typeid( CallerClass ); }

  /// Comparison between two BaseObjectMemberFunction instances.
  bool match( BaseObjectMemberFunction* bmf ) const override {
    if ( bmf == (BaseObjectMemberFunction*)this ) return true;
    if ( type() == bmf->type() ) {
      ObjectMemberFunction* mf = dynamic_cast<ObjectMemberFunction*>( bmf );
      return m_instance == mf->m_instance && m_memberFunction == mf->m_memberFunction;
    }
    return false;
  }

  /// Cast the object to DataObject.
  DataObject* castToDataObject() const override { return dynamic_cast<DataObject*>( m_instance ); }

  /// Cast the object to ValidDataObject.
  ValidDataObject* castToValidDataObject() const override { return dynamic_cast<ValidDataObject*>( m_instance ); }

  /// Cast the object to void with dynamic_cast.
  void* castToVoid() const override { return dynamic_cast<void*>( m_instance ); }

protected:
  /// Standard constructor. Protected so that can be called only by itself or IUpdateManagerSvc.
  ObjectMemberFunction( CallerClass* instance, const MemberFunctionType& mf )
      : m_instance( instance ), m_memberFunction( mf ) {}

  /// Pointer to the object.
  CallerClass* m_instance;

  /// Pointer to the member function.
  MemberFunctionType m_memberFunction;

  friend class IUpdateManagerSvc;
};

/** @class BasePtrSetter
 *
 * Base class to set the pointer to an object of a class derived from DataObject in a generic way.
 *
 *  @author Marco CLEMENCIC
 *  @date   2005-12-14
 */
class BasePtrSetter {
public:
  /// Empty virtual destructor.
  virtual ~BasePtrSetter() = default;
  /// sets the internal pointer to the provided data object (with a dynamic_cast).
  virtual void set( DataObject* ) = 0;
  /// tells if the internal pointer is nullptr.
  virtual bool isNull() = 0;
};

/** @class IUpdateManagerSvc IUpdateManagerSvc.h GaudiKernel/IUpdateManagerSvc.h
 *
 *  Interface class to the Update Manager service. Users should only use this interface.
 *
 *  @author Marco CLEMENCIC
 *  @date   2005-03-30
 */
class GAUDI_API IUpdateManagerSvc : virtual public IInterface {
private:
  /** @class PtrSetter
   *
   * Templated specialization of BasePtrSetter.
   */
  template <class ActualType>
  class PtrSetter final : public BasePtrSetter {
  public:
    using dest_type = ActualType;
    PtrSetter( dest_type*& dest ) : m_storage( &dest ) { *m_storage = nullptr; }
    void set( DataObject* obj ) override { *m_storage = dynamic_cast<dest_type*>( obj ); }
    bool isNull() override { return *m_storage == nullptr; }

  private:
    /// pointer to the pointer to fill provided by the user.
    dest_type** m_storage;
  };

public:
  /// InterfaceID
  DeclareInterfaceID( IUpdateManagerSvc, 2, 0 );

  /// Give access to the data provider.
  virtual IDataProviderSvc* dataProvider() const = 0;

  /// Give access to the detector data service interface (usualy of the data provider itself).
  virtual IDetDataSvc* detDataSvc() const = 0;

  /// Register an object (algorithm instance) to the service.
  /// The object should provide a method to be called in case of an update of
  /// the needed condition. An object can register multiple conditions using the
  /// same method: it will be called if at least one of the specified conditions
  /// is updated, but only when all of them are up to date.
  /// \return StatusCode::SUCCESS if the registration went right.
  template <class CallerClass>
  inline void registerCondition( CallerClass* instance, const std::string& condition = "",
                                 typename ObjectMemberFunction<CallerClass>::MemberFunctionType mf = nullptr ) {
    i_registerCondition( condition, new ObjectMemberFunction{ instance, mf } );
  }

  template <class CallerClass, class CondType>
  inline void registerCondition( CallerClass* instance, const std::string& condition,
                                 typename ObjectMemberFunction<CallerClass>::MemberFunctionType mf,
                                 CondType*&                                                     condPtrDest ) {
    i_registerCondition( condition, new ObjectMemberFunction{ instance, mf }, new PtrSetter{ condPtrDest } );
  }

  /// See above. Needed to avoid conflicts with the next one.
  template <class CallerClass>
  inline void registerCondition( CallerClass* instance, const char* condition,
                                 typename ObjectMemberFunction<CallerClass>::MemberFunctionType mf = nullptr ) {
    i_registerCondition( std::string( condition ), new ObjectMemberFunction{ instance, mf } );
  }

  /// Like the first version of registerCondition, but instead declaring the dependency on a condition of the service
  /// privider, it uses an already registered object. It means that a generic object can depend on another generic
  /// object that depends on a ValidDataObject. The dependency network is kept consistent by the UpdateManagerSvc.
  template <class CallerClass, class ObjectClass>
  inline void registerCondition( CallerClass* instance, ObjectClass* obj,
                                 typename ObjectMemberFunction<CallerClass>::MemberFunctionType mf = nullptr ) {
    i_registerCondition( dynamic_cast<void*>( obj ), new ObjectMemberFunction{ instance, mf } );
  }

  /// Generic objects can be unregistered from the UpdateManagerSvc. The dependency network is always consistent, but
  /// internal IOVs are not modified. \warning{Removing a node which other nodes depends on can create problems if the
  /// methods of this other nodes are called.}
  template <class CallerClass>
  inline void unregister( CallerClass* instance ) {
    i_unregister( dynamic_cast<void*>( instance ) );
  }

  /// Invalidate the given object in the dependency network. It means that all the objects depending on that one will
  /// be updated before the next event.
  template <class CallerClass>
  inline void invalidate( CallerClass* instance ) {
    i_invalidate( dynamic_cast<void*>( instance ) );
  }

  /// Retrieve the interval of validity (in the UpdateManagerSvc) of the given item.
  /// @return false if the item was not found.
  virtual bool getValidity( const std::string path, Gaudi::Time& since, Gaudi::Time& until,
                            bool path_to_db = false ) = 0;

  /// Change the interval of validity of the given item to the specified values, updating parents if needed.
  /// The change can only restrict the current IOV, If you want to expand the validity you should act on the transient
  /// data store
  /// and the change will be reflected at the next update.
  virtual void setValidity( const std::string path, const Gaudi::Time& since, const Gaudi::Time& until,
                            bool path_to_db = false ) = 0;

  /// Start an update loop using the event time given by the detector data service.
  virtual StatusCode newEvent() = 0;
  /// Start an update loop using the give event time.
  /// NOTE: the given event time is only used to check if updates are needed, the real update is done
  /// using the detector data service event time. (may change in future)
  virtual StatusCode newEvent( const Gaudi::Time& ) = 0;

  /// Update the given instance. This method should be called by the object after registration to ensure
  /// that the needed operations is done immediately and not before the next event.
  template <class CallerClass>
  inline StatusCode update( CallerClass* instance ) {
    return i_update( dynamic_cast<void*>( instance ) );
  }

  /// Debug method: it dumps the dependency network through the message service (not very readable, for experts only).
  virtual void dump() = 0;

  /// Force the update manager service to wait before entering the newEvent loop.
  virtual void acquireLock() = 0;
  /// Let the update manager service enter the newEvent loop.
  virtual void releaseLock() = 0;

  /// Remove all the items referring to objects present in the transient store.
  /// This is needed when the Detector Transient Store is purged, otherwise we
  /// will keep pointers to not existing objects.
  virtual void purge() = 0;

  /** @class IUpdateManagerSvc::PythonHelper
   * Helper class implemented in the python dictionary to allow access from python to
   * template member functions.
   */
  class PythonHelper;

protected:
  // virtual StatusCode i_registerCondition(const std::string &condition, BaseObjectMemberFunction *mf) = 0;
  virtual void       i_registerCondition( const std::string& condition, BaseObjectMemberFunction* mf,
                                          BasePtrSetter* ptr_dest = nullptr )       = 0;
  virtual void       i_registerCondition( void* obj, BaseObjectMemberFunction* mf ) = 0;
  virtual StatusCode i_update( void* instance )                                     = 0;
  virtual void       i_unregister( void* instance )                                 = 0;
  virtual void       i_invalidate( void* instance )                                 = 0;

  friend class PythonHelper;
};
#endif // GAUDIKERNEL_IUPDATEMANAGERSVC_H
