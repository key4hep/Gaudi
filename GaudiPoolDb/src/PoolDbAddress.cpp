// $Id: PoolDbAddress.cpp,v 1.8 2008/01/17 13:20:52 marcocle Exp $
//====================================================================
//	PoolDbAddress implementation
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: Pool opaque address implementation
//
//	Author     : M.Frank
//====================================================================

// Framework include files
#include "GaudiPoolDb/PoolDbAddress.h"
#include "StorageSvc/DataCallBack.h"
#include "StorageSvc/DbInstanceCount.h"
// C++ include files
#include <stdexcept>
#include <iostream>

static pool::DbInstanceCount::Counter* s_count =
  pool::DbInstanceCount::getCounter(typeid(PoolDbAddress));

/// Full constructor: all arguments MUST be valid, no checks!
PoolDbAddress::PoolDbAddress(pool::Token* aToken)
  : m_refCount(0), m_pRegistry(0), m_handler(0), m_token(aToken)
{
  s_count->increment();
  if ( m_token )  {
    m_token->addRef();
    return;
  }
  throw std::runtime_error("Invalid token (NULL) passed to PoolDbAddress");
}

/// Standard destructor
PoolDbAddress::~PoolDbAddress()
{
  pool::releasePtr(m_handler);
  s_count->decrement();
  unsigned int cnt = m_token->release();
  if ( cnt != 0 ) {
    if ( !(m_token->contID() == "/Event"      ||
           m_token->contID() == "/FileRecords" ||
	   m_token->contID() == "/RunRecords" ||
           clID() == CLID_RowWiseTuple        ||
           clID() == CLID_ColumnWiseTuple ) )  {
      std::cout << "PoolDbAddress::~PoolDbAddress> Token: " << cnt
                << "  " << m_token->dbID() << " :: "
                << m_token->contID() << std::endl
                << "Possible memory leak." << std::endl;
    }
  }
}

/// Release reference to object
unsigned long PoolDbAddress::release()
{
  int cnt = --m_refCount;
  if ( 0 == cnt )   {
    delete this;
  }
  return cnt;
}

/// Retrieve string parameters
const std::string* PoolDbAddress::par() const
{
  m_par[0] = m_token->dbID();
  m_par[1] = m_token->contID();
  return m_par;
}

/// Retrieve string parameters
const unsigned long* PoolDbAddress::ipar() const
{
  m_ipar[0] = m_token->oid().first;
  m_ipar[1] = m_token->oid().second;
  return m_ipar;
}

void PoolDbAddress::setHandler(pool::DataCallBack* h)
{
  pool::releasePtr(m_handler);
  m_handler = h;
}

bool PoolDbAddress::isValid()  const  {
  const pool::Token::OID_t& oid = m_token->oid();
  return oid.first != -1 && oid.second != -1;
}
