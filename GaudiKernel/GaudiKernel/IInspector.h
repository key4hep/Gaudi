// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IInspector.h,v 1.4 2006/11/30 15:22:07 dquarrie Exp $
#ifndef GAUDIKERNEL_IINSPECTOR_H
#define GAUDIKERNEL_IINSPECTOR_H

// STL Include files
#include <typeinfo>
#include <string>

// Framework Include files
#include "GaudiKernel/IInterface.h"

/** @class IInspector IInspector.h GaudiKernel/IInspector.h

    Inspector base class
*/
class GAUDI_API IInspector: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IInspector,1,0);

  enum { MUTABLE = 1<<1, CONST = 1<<2};

protected:

  /**
  */
  class IValue  {
  protected:
    void* m_P;
    IValue()                      {                            }
  public:
    virtual ~IValue()             { }
    virtual void release()        {      delete this;          }
    void* ptr()                   {      return m_P;           }
    const void* ptr()  const      {      return m_P;           }
    virtual long size()  const = 0;
    virtual void construct(void* buffer) const = 0;
  };

  /**
  */
  class Tag  {
  public:
    long             first;
    const std::type_info& second;
    Tag(long f, const std::type_info& s) : first(f), second(s) {    }
    Tag(const Tag& t) : first(t.first), second(t.second)  {    }
  };

private:
  /**
  */
  template <class T> class _V : public IValue    {
    T m_O;
  public:
    _V(const T& v) : m_O(v)       {      m_P = &m_O;           }
    virtual ~_V()                 {                            }
    virtual long size()  const    {      return sizeof(T);     }
    void construct(void* b) const {      ::new(b) T();         }
  };

  /**
  */
  template <class T> class _TT : public Tag   {
  public:    _TT() : Tag(sizeof(T), typeid(T))    {         }
  };

protected:
  // Referenced implementation of the IInspector interface:
  /// Inspect object by Reference
	virtual StatusCode inspectByRef(const void* pObj, const Tag& typ, void* pOwner, const Tag& otag, const std::string& comment, long flag) = 0;
  /// Inspect object by Value
	virtual StatusCode inspectByValue(IValue* pObj,   const Tag& typ, void* pOwner, const Tag& oTag, const std::string& comment) = 0;
  /// Inspect container of objects by reference
	virtual StatusCode inspectContByRef(const void* pObj, const Tag& tag, const Tag& rtag, const Tag& vtag, const void* pOwner, const Tag& otag, const std::string& comment, long flags) = 0;
  /// Inspect container of objects by value
	virtual StatusCode inspectContByValue(IValue* pObj, const Tag& tag, const Tag& rtag, const Tag& vtag, const void* pOwner, const Tag& otag, const std::string& comment) = 0;

public:
  // User interface of the IInspector interface
  /// Inspect single item by its reference (mutable and const)
	template < class T, class O >
  StatusCode inspectByRef(const T* pObj, const O* pOwner, const std::string& comment, long flag=MUTABLE)	{
		return inspectByRef(pObj, _TT<T>(), (void*)pOwner, _TT<O>(), comment, flag);
	}
  /// Inspect single item by its value (const)
	template < class T, class O >
  StatusCode inspectByValue(const T& obj, const O* pOwner, const std::string& comment) {
		return inspectByValue(new _V<T>(obj), _TT<T>(), (void*)pOwner, _TT<O>(), comment);
	}
  /// Inspect container of object items by its reference (mutable and const)
	template < class T, class O >
  StatusCode inspectContByRef( const T* pObj, const O* pOwner, const std::string& comment, long flag=MUTABLE)	  {
	  typedef typename T::value_type                 _VVV;
          typedef typename T::value_type                 _TTT;
	  // Unfortunately this is not implemented on G++:
          // typedef typename T::allocator_type::value_type _TTT;
    return inspectContByRef((void*)pObj, _TT<T>(), _TT<_VVV>(), _TT<_TTT>(), (void*)pOwner, _TT<O>(), comment, flag);
  }
  /// Inspect container of object items by its value (const)
	template < class T, class O >
  StatusCode inspectContByValue( const T& obj, const O* pOwner, const std::string& comment)	  {
	  typedef typename T::value_type                 _VVV;
          typedef typename T::value_type                 _TTT;
	  // Unfortunately this is not implemented on G++:
          // typedef typename T::allocator_type::value_type _TTT;
    return inspectContByValue(new _V<T>(obj), _TT<T>(), _TT<_VVV>(), _TT<_TTT>(), (void*)pOwner, _TT<O>(), comment);
  }
};
#endif // GAUDIKERNEL_IINSPECTOR_H
