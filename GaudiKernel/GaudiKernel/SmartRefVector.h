//	====================================================================
//	SmartRefVector.h
//	--------------------------------------------------------------------
//
//	Package   : Kernel
//
//	Author    : Markus Frank
//
//	====================================================================
#ifndef KERNEL_SMARTREFVECTOR_H
#define KERNEL_SMARTREFVECTOR_H 1

// STL include files
#include <vector>

// Include files
#include "GaudiKernel/SmartRef.h"

/** Kernel objects: SmartRefVector

    Description:
    The SmartRefVector class allows transparent handling of multiple object links
    within the data store. Links are unloaded a priori and will only be
    loaded "on demand", i.e. when dereferenced.

    SmartRefVectors should behave in the same way as normal vectors of pointers;

    Using SmartRefVectors the environment of the vector is automatically
    propagated to each contained reference.

    In order to speed things up, an attempt was made to avoid any virtual
    functions.

    Base Class:
    STL vector

    Dependencies:
    <UL>
    <LI> Base class:                vector
    <LI> SmartRef constructs:       Gaudi/Kernel/SmartRef.h
    </UL>

    <P> History    :
    <PRE>
    +---------+----------------------------------------------+--------+
    |    Date |                 Comment                      | Who    |
    +---------+----------------------------------------------+--------+
    | 12/07/99| Initial version.                             | MF     |
    +---------+----------------------------------------------+--------+
    </PRE>
    Author:  M.Frank
    Version: 1.0
*/
template <class TYPE> class SmartRefVector : public std::vector < SmartRef<TYPE> >   {
protected:
  /// That's the type of crap I am hosting
  typedef SmartRef<TYPE>          _Entry;
  /// Object types: typedef myself as Base
  typedef std::vector <_Entry>    _Base;
  typedef typename std::vector<_Entry>::const_iterator    _BaseConstIter;
  typedef typename std::vector<_Entry>::value_type        _BaseValueType;

  /// Object data: Pointer to the identifiable object the link originates
  mutable const DataObject*      m_data;
  /// Object data: Pointer to the Contained object (if applicable)
  mutable const ContainedObject* m_contd;

  /// Set the environment for the vector and all contained objects references
  void _setEnvironment(const DataObject* pObj, const ContainedObject* pContd)  const  {
    m_data  = pObj;
    m_contd = pContd;
    for ( _BaseConstIter i = _Base::begin(); i != _Base::end(); i++ )   {
      (*i)._setEnvironment(pObj, pContd);
    }
  }
public:
  /// Standard Constructor
  SmartRefVector()  {
    m_contd  = 0;
    m_data   = 0;
  }
  /// templated Constructor
  template <class ITERATOR>
  SmartRefVector( ITERATOR first , ITERATOR last ) 
    :  std::vector< SmartRef<TYPE> >( first , last ) 
    , m_data  ( 0 ) 
    , m_contd ( 0 ) 
  {
   
  }
  /// Copy Constructor
  SmartRefVector(const SmartRefVector& copy) 
    :  std::vector< SmartRef<TYPE> >(copy) {
    *this = copy;
  }
  /// Standard destructor
  //virtual ~SmartRefVector()                  {
  //}

  /// operator(): assigns parent object for serialisation
  SmartRefVector<TYPE>& operator() (ContainedObject* pObj)    {
    _setEnvironment((0==pObj) ? 0 : pObj->parent(), pObj);
    return *this;
  }
  /// operator() const: assigns parent object for serialisation
  const SmartRefVector<TYPE>& operator() (const ContainedObject* pObj)  const  {
    _setEnvironment((0==pObj) ? 0 : pObj->parent(), pObj);
    return *this;
  }
  /// operator(): assigns parent object for serialisation
  SmartRefVector<TYPE>& operator() (DataObject* pObj)    {
    _setEnvironment(pObj,0);
    return *this;
  }
  /// operator() const: assigns parent object for serialisation
  const SmartRefVector<TYPE>& operator() (const DataObject* pObj)  const  {
    _setEnvironment(pObj,0);
    return *this;
  }
  /// Assignment
  SmartRefVector<TYPE>& operator=(const SmartRefVector<TYPE>& copy)  {
    _Base::operator=(copy);
    // Harms.... MF 
    // on copy we MUST make a 1 to 1 copy
    // _setEnvironment( copy.m_data, copy.m_contd );
    // use instead:
    m_data  = copy.m_data;
    m_contd = copy.m_contd;
    return *this;
  }
  /// Access to embedded type
  const std::type_info* type()  const   {
    return &typeid(TYPE);
  }
  /// Helper to read references
  StreamBuffer& readRefs(StreamBuffer& s);
  /// Helper to write references
  StreamBuffer& writeRefs(StreamBuffer& s)  const;
  /// Output Streamer operator
  // MCl: it is "_s" instead of the most common "s" to avoid a fake icc remark #1599
  friend StreamBuffer& operator<< (StreamBuffer& _s, const SmartRefVector<TYPE>& ptr)   {
    return ptr.writeRefs(_s);
  }
  /// Input Streamer operator
  // MCl: it is "_s" instead of the most common "s" to avoid a fake icc remark #1599
  friend StreamBuffer& operator>> (StreamBuffer& _s, SmartRefVector<TYPE>& ptr)     {
    return ptr.readRefs(_s);
  }
};

template <class TYPE> inline
StreamBuffer& SmartRefVector<TYPE>::writeRefs(StreamBuffer& s)  const  {
  long len = _Base::size();
  s << len;
  for ( _BaseConstIter i = _Base::begin(); i != _Base::end(); i++ )   {
    (*i)._setEnvironment(m_data, m_contd);
    (*i).writeRef(s);
  }
  return s;
}

template <class TYPE> inline
StreamBuffer& SmartRefVector<TYPE>::readRefs(StreamBuffer& s)    {
  long len;
  _Base::erase( _Base::begin(), _Base::end() );
  s >> len;
  for ( long i = 0; i < len; i++ )    {
    _BaseValueType entry;
    entry._setEnvironment(m_data, m_contd);
    entry.readRef(s);
    _Base::push_back( entry );
  }
  return s;
}

#endif // KERNEL_SMARTREFVECTOR_H
