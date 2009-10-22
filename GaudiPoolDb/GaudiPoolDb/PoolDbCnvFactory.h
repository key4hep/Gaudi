// $Id: PoolDbCnvFactory.h,v 1.3 2006/11/22 18:10:43 hmd Exp $
//====================================================================
//	IPoolDbCnvFactory and PoolDbCnvFactory definition
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
#ifndef POOLDB_POOLDBCNVFACTORY_H
#define POOLDB_POOLDBCNVFACTORY_H 1
#include "GaudiKernel/ICnvFactory.h"
#include "GaudiKernel/FactoryTable.h"
#include "GaudiKernel/System.h"
#include "StorageSvc/DbType.h"

/** @class IPoolDbCnvFactory PoolDbCnvFactory.h PoolDb/PoolDbCnvFactory.h
  *
  * Description:
  * Interface definition for POOL converter factories.
  *
  * @author  M.Frank
  * @version 1.0
  */
class GAUDI_API IPoolDbCnvFactory: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IPoolDbCnvFactory,2,0);
  /** Instantiation method.
    *
    * @param  typ      [IN]    Storage type of the converter to be created
    * @param  clid     [IN]    Class identifier of the converter
    * @param  svcloc   [IN]    Pointer to service locator object
    *
    * @return Pointer to created converter object.
    */
  virtual IConverter* instantiate(long typ,
                                  const CLID& clid,
                                  ISvcLocator *svcloc )  const = 0;
};

/** @class PoolDbCnvFactory PoolDbCnvFactory.h PoolDb/PoolDbCnvFactory.h
  *
  * Description:
  * Implementation definition for POOL converter factories.
  *
  * @author  M.Frank
  * @version 1.0
  */
template <class ConcreteConverter, int i>
class GAUDI_API PoolDbCnvFactory : public implements2<ICnvFactory, IPoolDbCnvFactory>
{
  /// Object type name
  mutable std::string m_typeName;
public:
  /// Converter type
  typedef ConcreteConverter converter_t;

  /// Standard constructor
  PoolDbCnvFactory()  {
    m_typeName = "";
    FactoryTable::instance()->addFactory( this );
  }
  /// Standard destructor
  virtual ~PoolDbCnvFactory()  {  }
  /// Access to object type
  virtual const std::string& typeName()  const    {
    // The name MUST be set here, not in the constructor.
    // At construction time it is not yet clear if we run from
    // an executable or a DLL.
    if ( m_typeName == "" )  {
      if ( System::moduleType() == System::SHAREDLIB )  {
        m_typeName  = System::moduleName();
        m_typeName += ":";
      }
      // Get the class name using the RTTI.
      m_typeName += System::typeinfoName( typeid(ConcreteConverter) );
    }
    return m_typeName;
  }
  /// Access to the class type of the converter
  virtual const CLID& objType()  const;
  /// Access to the service type of the converter
  virtual const long repSvcType()   const
  {    return pool::POOL_StorageType.type();  }
  virtual unsigned long addRef()  const     {     return 1;           }
  virtual unsigned long release()   const   {     return 1;           }
  virtual const std::string& ident() const  {     return typeName();  }
  // IInterface implementation
  virtual unsigned long addRef()            {     return 1;           }
  virtual unsigned long release()           {     return 1;           }

  /** Instantiation method.
    *
    * @param  typ      [IN]    Storage type of the converter to be created
    * @param  clid     [IN]    Class identifier of the converter
    * @param  svcloc   [IN]    Pointer to service locator object
    *
    * @return Pointer to created converter object.
    */
  virtual IConverter* instantiate(long typ,
                                  const CLID& /* clid */,
                                  ISvcLocator *svcloc)  const
  {    return new ConcreteConverter(typ, svcloc);  }

  /// virtual overload: Instantiate an instance of a converter
  virtual IConverter* instantiate( ISvcLocator* /* svcloc */ )  const
  {    return 0;  }

  /// Create an instance of a generic Gaudi object: Method is disabled!
  virtual IInterface* instantiate( IInterface* /* parent */ ) const
  {    return 0;  }

};
#endif    // POOLDB_POOLDBCNVFACTORY_H
