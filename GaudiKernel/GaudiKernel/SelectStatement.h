//==============================================================================
// $Id: SelectStatement.h,v 1.3 2004/07/06 10:17:40 mato Exp $
//------------------------------------------------------------------------------
//
// Package : Kernel
//
// Author  : M.Frank 10/10/00
//
//==============================================================================
#ifndef KERNEL_SELECTSTATEMENT_H
#define KERNEL_SELECTSTATEMENT_H

// STL include files
#include <string>

// Framework include files
#include "GaudiKernel/ISelectStatement.h"

/** Class of a selection statement.
  A select statement can either contain
  - a string e.g. for refining an SQL statement
  - a function object, which will be called back
    in order to refine a selection.
    This happens in calling sequences like the following:

  bool MySelect::operator()(IValueLocator* l)   {
    float px, py, pz;
    if ( l->get("PX",px) && l->get("PY",py) && l->get("PZ",pz) )  {
      float mom = sqrt(px*px+py*py+pz*pz);
      return mom > 100.0 * GeV;
    }
    return false;
  }

    if "true" is returned, the object will be loaded completely.

    History:
    +---------+----------------------------------------------+--------+
    |    Date |                 Comment                      | Who    |
    +---------+----------------------------------------------+--------+
    | 21/10/99| Initial version.                             | MF     |
    +---------+----------------------------------------------+--------+
    Author:  M.Frank
    Version: 1.0
*/
class GAUDI_API SelectStatement: public implements1<ISelectStatement> {
public:
  /// Standard Constructor initializing select string
  explicit SelectStatement(const std::string& s, long typ)
  : m_select(s), m_isActive(false), m_type(typ)
  {
  }
  /// Standard Constructor initializing select string
  explicit SelectStatement(const std::string& s)
  : m_select(s),m_isActive(false), m_type(STRING)
  {
  }
  /// Standard Constructor initializing function call
  explicit SelectStatement()
  : m_isActive(false), m_type(FUNCTION)
  {
  }
  /// Standard Destructor
  virtual ~SelectStatement()  {
  }
  /// Access the type of the object
  long type()   const   {
    return m_type;
  }
  /// Access the selection string
  const std::string& criteria()  const    {
    return m_select;
  }
  /// Set the type
  void setCriteria(const std::string& crit)    {
    m_select = crit;
    (m_select.length() > 0) ? m_type |= STRING : m_type &= ~STRING;
  }
  /// Change activity flag
  void setActive(bool flag = true)   {
    m_isActive = flag;
  }
  /// Check if selection is active
  bool isActive()   const   {
    return m_isActive;
  }
  /// Stupid default implementation
  virtual bool operator()(void* /* val */ )   {
    return true;
  }
protected:
  /// Select string
  std::string   m_select;
  /// Activation flag
  bool m_isActive;
  /// Type identifier
  long m_type;
};
#endif  // KERNEL_SELECTSTATEMENT_H
