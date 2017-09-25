#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include <functional>

IClassIDSvc* DataObjID::p_clidSvc( nullptr );
std::once_flag DataObjID::m_ip;

void DataObjID::hashGen()
{

  if ( m_clid == 0 ) {
    m_hash = ( std::hash<std::string>()( m_key ) );
  } else {
    // this is a bit redundant since hash<int> is a pass-through
    m_hash = ( std::hash<std::string>()( m_key ) ) ^ ( std::hash<CLID>()( m_clid ) << 1 );
  }
}

void DataObjID::getClidSvc() { p_clidSvc = Gaudi::svcLocator()->service<IClassIDSvc>( "ClassIDSvc" ).get(); }

void DataObjID::setClid()
{

  std::call_once( m_ip, &DataObjID::getClidSvc );

  if ( p_clidSvc == nullptr || p_clidSvc->getIDOfTypeName( m_className, m_clid ).isFailure() ) {
    m_clid      = 0;
    m_className = "UNKNOWN_CLASS:" + m_className;
  }
}

void DataObjID::setClassName()
{

  std::call_once( m_ip, &DataObjID::getClidSvc );

  if ( p_clidSvc == nullptr || p_clidSvc->getTypeNameOfID( m_clid, m_className ).isFailure() ) {
    m_className = "UNKNOW_CLID:" + std::to_string( m_clid );
  }
}

std::ostream& operator<<( std::ostream& str, const DataObjID& d )
{
  if ( d.m_clid == 0 && d.m_className == "" ) {
    str << "('" << d.m_key << "')";
  } else {
    str << "('" << d.m_className << "','" << d.m_key << "')";
  }
  return str;
}

std::string DataObjID::fullKey() const
{
  if ( m_clid == 0 && m_className == "" ) {
    return m_key;
  } else {
    return ( m_className + "/" + m_key );
  }
}
