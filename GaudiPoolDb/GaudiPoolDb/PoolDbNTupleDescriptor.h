// $Id: PoolDbNTupleDescriptor.h,v 1.1.1.1 2004/01/16 14:05:03 mato Exp $
//------------------------------------------------------------------------------
// Definition of class :  PoolDbNTupleDescriptor, PoolDbTokenWrap
//
//  Author     : M.Frank
//
//------------------------------------------------------------------------------
#ifndef POOLDB_POOLDBNTUPLEDESCRIPTOR_H
#define POOLDB_POOLDBNTUPLEDESCRIPTOR_H

// Framework include files
#include "POOLCore/Token.h"

/** @class PoolDbNTupleDescriptor PoolDbNTupleDescriptor.h GaudiPoolDb/PoolDbNTupleDescriptor.h
  *
  * Description:
  *
  * @author  M.Frank
  * @version 1.0
  */
struct PoolDbNTupleDescriptor {
  /// Description string
  std::string   description;
  /// Optional description 
  std::string   optional;
  /// Object giud of described object
  std::string   guid;
  /// Identifier of description
  std::string   container;
  /// Class ID of the described object
  unsigned long clid;

  /// Standard destructor
  virtual ~PoolDbNTupleDescriptor() {}
};


/** @class PoolDbTokenWrap PoolDbNTupleDescriptor.h PoolDb/PoolDbNTupleDescriptor.h
  *
  * Description:
  *
  * @author  M.Frank
  * @version 1.0
  */
struct PoolDbTokenWrap {
  /// Aggregated token object
  pool::Token token;

  /// Standard constructor
  PoolDbTokenWrap() {}

  /// Copy constructor
  PoolDbTokenWrap(const PoolDbTokenWrap& wrp)  {
    wrp.token.setData(&token);
  }

  /// Standard destructor
  virtual ~PoolDbTokenWrap() {}

  /// Equality operator
  bool operator==(const PoolDbTokenWrap& c) const {
    return token == c.token;
  }

  /// Assignment operator
  PoolDbTokenWrap& operator=(const PoolDbTokenWrap& wrp)  {
    wrp.token.setData(&token);
    return *this;
  }
};
#endif // POOLDB_POOLDBNTUPLEDESCRIPTOR_H
