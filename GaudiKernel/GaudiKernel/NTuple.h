// $Id: NTuple.h,v 1.23 2008/10/27 19:22:20 marcocle Exp $
// ============================================================================
#ifndef GAUDIKERNEL_NTUPLE_H
#define GAUDIKERNEL_NTUPLE_H
// ============================================================================
// STL include files
// ============================================================================
#include <string>
#include <limits>
#include <cfloat>
#include <stdexcept>
// ============================================================================
// Framework include files
// ============================================================================
#include "GaudiKernel/DataTypeInfo.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/INTuple.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/IOpaqueAddress.h"
// ============================================================================
// Forward declarations
// ============================================================================
class NTupleFile;
class NTupleDirectory;
// ============================================================================
/**
 * NTuple name space
 *
 *  Definition of helpers to access N tuple implementations
 *
 * @author Markus Frank
 * @author Sebastien Ponce
 */
namespace NTuple
{
  // local forward declarations
  template <class TYP> class Range;
  template <class TYP> class _Data;
  template <class TYP> class _Item;
  template <class TYP> class _Array;
  template <class TYP> class _Matrix;
  template <class TYP> class _Accessor;
  template <class TYP> class Item;
  template <class TYP> class Array;
  template <class TYP> class Matrix;
  // ==========================================================================
  /// Class defining a range
  template <class TYP>
  class Range  {
    /// Lower boundary of range
    /*const*/ TYP    m_lower;
    /// Upper boundary of range
    /*const*/ TYP    m_upper;
  public:
    /// Standard constructor
    Range(const TYP low, const TYP upper) : m_lower(low), m_upper(upper)   {
    }
    /// Copy constructor
    Range(const Range<TYP>& copy) : m_lower(copy.m_lower),
                                    m_upper(copy.m_upper) {
    }
    /// Adjust ranges
    Range& operator=(const Range<TYP>& copy)    {
      m_lower = copy.m_lower;
      m_upper = copy.m_upper;
      return *this;
    }
    /// Destructor
    virtual ~Range()       {                               }
    /// Lower boundary of range
    TYP lower()    const   { return m_lower;               }
    /// Upper boundary of range
    TYP upper()    const   { return m_upper;               }
    /// Distance between lower and upper range
    TYP distance() const   { return m_upper-m_lower;       }
    /// Minimal number of data
    static TYP min() { return std::numeric_limits<TYP>::min() ; }
    /// Maximal number of data
    static TYP max() { return std::numeric_limits<TYP>::max() ; }
  };
  // ==========================================================================
  template <> class Range<bool>
  {
  public:
    /// Standard constructor
    Range(const bool /* low */ , const bool /* upper */ )    {}
    /// Copy constructor
    Range(const Range<bool>& /* copy */ )  {}
    /// Destructor
    virtual ~Range()        {                               }
    /// Lower boundary of range
    bool lower()    const   { return false;                 }
    /// Upper boundary of range
    bool upper()    const   { return true;                  }
    /// Distance between lower and upper range
    bool distance() const   { return true;                  }
    /// Minimal number of data
    static bool min()       { return false;                 }
    /// Maximal number of data
    static bool max()       { return true;                  }
  };
  // ==========================================================================
  template <>
  inline IOpaqueAddress*
  Range<IOpaqueAddress* >::min()     { return (IOpaqueAddress*)0x0        ; }
  template <>
  inline IOpaqueAddress*
  Range<IOpaqueAddress* >::max()     { return (IOpaqueAddress*)0xffffffff ; }
  // ==========================================================================
  /** Abstract class describing basic data in an Ntuple.
   */
  template <class TYP> class GAUDI_API _Data : virtual public INTupleItem  {
  protected:
    /// Pointer to data buffer
    TYP* m_buffer;
  public:
    /// Set type definition to make life more easy easy
    typedef Range<TYP>  ItemRange;
    /// Set default value
    virtual void setDefault(const TYP d)          = 0;
    /// Access the range if specified
    virtual const ItemRange& range() const        = 0;
  };
  // ==========================================================================
  /** Abstract class describing a column in a N tuple.
   */
  template <class TYP> class GAUDI_API _Item : virtual public _Data<TYP>  {
  public:
    /// Destructor.
    virtual ~_Item() {}
    /// Create instance
    static _Item* create(INTuple* tup,
                         const std::string& name,
                         const std::type_info& info,
                         TYP min,
                         TYP max,
                         TYP def);
    /// Assignment operator
    template <class T>
    _Item<TYP>& operator=(const _Item<T>& copy)   {
      *this->m_buffer = copy.get();
      return *this;
    }
    /// Access to data by reference
    void       set(const TYP& item) { *this->m_buffer = item;             }
    /// Access to data by reference (CONST)
    virtual TYP get() const   { return *this->m_buffer;             }
  };
  // ==========================================================================
  /** Abstract class describing a column-array in a N tuple.
   */
  template <class TYP> class GAUDI_API _Array : virtual public _Data<TYP>  {
  public:
    /// Create instance
    static _Array* create(INTuple* tup,
                          const std::string& name,
                          const std::type_info& info,
                          const std::string& index,
                          long len,
                          TYP min,
                          TYP max,
                          TYP def);
    /// Assignment operator
    template <class T>
    _Array<TYP>& operator=(const _Array<T>& copy)   {
      long len = this->length();
      if ( len == copy.length() )    {
        const T* source = (const T*)copy.buffer();
        for ( int i = 0; i < len; i++ )  {
          *(this->m_buffer + i) = *(source + i);
        }
        return *this;
      }
      throw std::out_of_range
        ("N-tuple matrix cannot be copied! The index range does not match!");
      return *this;
    }
    /// Access to data by reference (CONST)
    const TYP& data(long i)  const  { return *(this->m_buffer + i);        }
    /// Access to data by reference (CONST)
    TYP&       data(long i)         { return *(this->m_buffer + i);        }
  };
  // ==========================================================================
  /** Abstract class describing a matrix column in a N tuple.
   */
  template <class TYP> class GAUDI_API _Matrix : virtual public _Data<TYP>    {
  protected:
    /// Number of rows per column
    long  m_rows;
  public:
    /// Create instance
    static _Matrix* create(INTuple* tup,
                           const std::string& name,
                           const std::type_info& info,
                           const std::string& index,
                           long ncol,
                           long nrow,
                           TYP min,
                           TYP max,
                           TYP def);
    /// Assignment operator
    template <class T>
    _Matrix<TYP>& operator=(const _Matrix<T>& copy)   {
      long len = this->length();
      if ( len == copy.length() )    {
        const T* source = (const T*)copy.buffer();
        for ( int i = 0; i < len; i++ )  {
          *(this->m_buffer + i) = *(source + i);
        }
        return *this;
      }
      throw std::out_of_range
        ("N-tuple matrix cannot be copied! The index range does not match!");
      return *this;
    }
    /// Access to data by reference
    TYP*       column(long i)       { return (this->m_buffer + i*m_rows);  }
    /// Access to data by reference (CONST)
    const TYP* column(long i) const { return (this->m_buffer + i*m_rows);  }
  };
  // ==========================================================================
  /** Class acting as a smart pointer holding a N tuple entry.
   */
  template <class TYP> class _Accessor  {
    friend class Tuple;
  private:
    _Accessor<TYP>& operator=(const _Accessor<TYP>&)   {
      return *this;
    }
  protected:
    /// Pointer to instance
    mutable TYP*  m_ptr;
  public:
    /// Standard Constructor
    _Accessor() : m_ptr(0)           {                                }
    /// Standard Destructor
    virtual ~_Accessor()             {                                }
    /// Check if column is present
    bool          operator !() const { return m_ptr != 0;             }
    /// Check if column is present
    operator const void*()     const { return m_ptr;                  }
    /// Dereference operator
    TYP*         operator->()        { return m_ptr;                  }
    /// Dereference operator (CONST)
    const TYP*   operator->() const  { return m_ptr;                  }
    /// Access the range
    const Range<TYP>& range()  const { return m_ptr->range();         }
  };
  // ==========================================================================
  /** Class acting as a smart pointer holding a N tuple _Item.
   */
  template <class TYP> class Item : virtual public _Accessor< _Item<TYP> > {
    typedef Item<TYP> _My;
  public:
    /// Standard Constructor
    Item()                          {    }
    /// Automatic type conversion
    operator const TYP () const     { return this->m_ptr->get();       }
    /// Dereference operator for pointers
    TYP          operator*()       { return this->m_ptr->get();       }
    /// Dereference operator for pointers(CONST)
    const TYP    operator*() const { return this->m_ptr->get();       }
    // Arithmetic operators defined on NTuple column entries
    Item& operator ++ ()            { return *this += TYP(1);          }
    Item& operator ++ (int)         { return *this += TYP(1);          }
    Item& operator -- ()            { return *this -= TYP(1);          }
    Item& operator -- (int)         { return *this -= TYP(1);          }
    /// Assignment operator
    Item& operator=(const TYP data) {
      this->m_ptr->set( data );
      return *this;
    }
    /// Assignment operator
    template<class T>
    Item& operator=(const Item<T>& data) {
      this->m_ptr->set( data->get() );
      return *this;
    }
    Item<TYP>& operator += (const TYP data)    {
      this->m_ptr->set ( this->m_ptr->get() + data );
      return *this;
    }
    Item<TYP>& operator -= (const TYP data)    {
      this->m_ptr->set ( this->m_ptr->get() - data );
      return *this;
    }
    Item<TYP>& operator *= (const TYP data)    {
      this->m_ptr->set ( this->m_ptr->get() * data );
      return *this;
    }
    Item<TYP>& operator /= (const TYP data)    {
      this->m_ptr->set ( this->m_ptr->get() / data );
      return *this;
    }
  };
  // ==========================================================================
  /** Specialization acting as a smart pointer holding a N tuple _Item.
   */
  template <> class Item<bool> : virtual public _Accessor< _Item<bool> > {
    typedef Item<bool> _My;
  public:
    /// Standard Constructor
    Item()                          {    }
    /// Automatic type conversion
    operator bool () const     { return this->m_ptr->get();             }
    /// Assignment operator
    Item& operator=(const bool data) {
      this->m_ptr->set( data );
      return *this;
    }
    /// Assignment operator
    template<class T>
    Item& operator=(const Item<T>& data) {
      this->m_ptr->set( data->get() );
      return *this;
    }
  };
  // ==========================================================================
  /** Class acting as a smart pointer holding a N tuple _Item.
   */
  template <class TYP> class Array  : virtual public _Accessor < _Array<TYP> > {
  public:
    /// Standard Constructor
    Array()                                  {    }
    /// Assignment operator
    template <class T>
    Array& operator=(const Array<T>& copy)  {
      *(this->m_ptr) = *(copy.operator->());
      return *this;
    }
    /// Array operator
    template <class T>
    TYP&       operator[] (const T i)        { return this->m_ptr->data(i);  }
    /// Array operator
    template <class T>
    const TYP& operator[] (const T i) const  { return this->m_ptr->data(i);  }
    virtual ~Array() {}
  };
  // =========================================================================
  /** Class acting as a smart pointer holding a N tuple _Item.
   */
  template <class TYP> class Matrix : virtual public _Accessor< _Matrix<TYP> > {
  public:
    /// Standard Constructor
    Matrix()                                  {    }
    /// Assignment operator
    template <class T>
    Matrix& operator=(const Matrix<T>& copy) {
      *(this->m_ptr) = *(copy.operator->());
      return *this;
    }
    /// Array operator
    template <class T>
    TYP*       operator[] (const T i)         { return this->m_ptr->column(i); }
    /// Array operator
    template <class T>
    const TYP* operator[] (const T i)  const  { return this->m_ptr->column(i); }
    virtual ~Matrix() {}
  };
  // =========================================================================
  /** Abstract base class which allows the user to interact with the
      actual N tuple implementation.
      The class is abstract, because the template methods must
      be instantiated by the compiler at compile time. Otherwise the
      references would be unresolved.
  */
  class Tuple : public DataObject, virtual public INTuple  {

  protected:
    /// Locate a _Column of data to the N tuple type safe
    template <class TYPE> StatusCode i_item(const std::string& name,
                                            _Item<TYPE>*& result)  const   {
      try   {
        result = dynamic_cast< _Item<TYPE>* > (i_find(name));
      }
      catch (...)   {
        result = 0;
      }
      return (0==result) ? StatusCode::FAILURE : StatusCode::SUCCESS;
    }
    /// Locate a _Column of data to the N tuple type unsafe for objects
    template <class TYPE> StatusCode i_item(const std::string& name,
                                            _Item<TYPE*>*& result)  const   {
      try   {
        _Item<void*>* p = dynamic_cast< _Item<void*>* > (i_find(name));
        result = (_Item<TYPE*>*)p;
      }
      catch (...)   {
        result = 0;
      }
      return (0==result) ? StatusCode::FAILURE : StatusCode::SUCCESS;
    }
    /// Locate a _Column of data to the N tuple type safe
    StatusCode i_item(const std::string& name,
                                            _Item<IOpaqueAddress*>*& result)  const   {
      try   {
        result = dynamic_cast< _Item<IOpaqueAddress*>* > (i_find(name));
      }
      catch (...)   {
        result = 0;
      }
      return (0==result) ? StatusCode::FAILURE : StatusCode::SUCCESS;
    }
    /// Locate a _Array of data to the N tuple type safe
    template <class TYPE> StatusCode i_item(const std::string& name,
                                            _Array<TYPE>*& result)  const   {
      try   {
        if ( clID() == CLID_ColumnWiseTuple )   {
          result = dynamic_cast< _Array<TYPE>* > (i_find(name));
        }
      }
      catch (...)   {
        result = 0;
      }
      return (0==result) ? StatusCode::FAILURE : StatusCode::SUCCESS;
    }
    /// Locate a _Matrix of data to the N tuple type safe
    template <class TYPE> StatusCode i_item(const std::string& name,
                                            _Matrix<TYPE>*& result)  const   {
      try   {
        if ( clID() == CLID_ColumnWiseTuple )   {
          result = dynamic_cast< _Matrix<TYPE>* > (i_find(name));
        }
      }
      catch (...)   {
        result = 0;
      }
      return (0==result) ? StatusCode::FAILURE : StatusCode::SUCCESS;
    }
    /// Add a _Item of data to the N tuple
    template <class TYPE>
    StatusCode i_addItem(const std::string& name,
                         long,
                         const std::string&,
                         TYPE low,
                         TYPE high,
                         _Item<TYPE>*& result)   {
      if ( !i_find(name) )    {
        TYPE nil;
        nil = 0;
        return add( result = _Item<TYPE>::create(this, name, typeid(TYPE), low, high, nil) );
      }
      return StatusCode::FAILURE;
    }
    /// Add a _Item of data to the N tuple
    template <class TYPE>
    StatusCode i_addItem(const std::string& name,
                         long dim,
                         const std::string& index,
                         TYPE low,
                         TYPE high,
                         _Array<TYPE>*& result)   {
      if ( !i_find(name) && clID() == CLID_ColumnWiseTuple )    {
        return add( result = _Array<TYPE>::create(this,
                                                  name,
                                                  typeid(TYPE),
                                                  index,
                                                  dim,
                                                  low,
                                                  high,
                                                  TYPE(0)) );
      }
      return StatusCode::FAILURE;
    }
    /// Add a _Item of data to the N tuple
    template <class TYPE>
    StatusCode i_addItem(const std::string& name,
                         long dim1,
                         long dim2,
                         const std::string& index,
                         TYPE low,
                         TYPE high,
                         _Matrix<TYPE>*& result)   {
      if ( !i_find(name) && clID() == CLID_ColumnWiseTuple )    {
        return add( result = _Matrix<TYPE>::create(this,
                                                   name,
                                                   typeid(TYPE),
                                                   index,
                                                   dim1,
                                                   dim2,
                                                   low,
                                                   high,
                                                   TYPE(0)) );
      }
      return StatusCode::FAILURE;
    }
    template <class TYPE> StatusCode
    i_addObject(const std::string& name,_Item<TYPE*>*& result,const std::type_info& /* typ */)  {
      if ( !i_find(name) && clID() == CLID_ColumnWiseTuple )    {
        return add( result = (_Item<TYPE*>*)_Item<void*>::create(this, name, typeid(TYPE),0,0,0) );
      }
      return StatusCode::FAILURE;
    }

  public:
    /// Standard destructor
    virtual ~Tuple()   {
    }
    /// Locate a scalar Item of data to the N tuple type safe
    template <class TYPE> StatusCode item(const std::string& name,
                                          Item<TYPE>& result)
    {
      return i_item(name, result.m_ptr);
    }
    /// Locate a scalar Item of data to the N tuple type safe (CONST)
    template <class TYPE> StatusCode item(const std::string& name,
                                          const Item<TYPE>& result)  const
    {
      return i_item(name, result.m_ptr);
    }
    /// Locate a Array of data to the N tuple type safe
    template <class TYPE> StatusCode
      item(const std::string& name, Array<TYPE>& result)
    {
      return i_item(name, result.m_ptr);
    }
    /// Locate a Array of data to the N tuple type safe (CONST)
    template <class TYPE> StatusCode item(const std::string& name,
                                          const Array<TYPE>& result)  const
    {
      return i_item(name, result.m_ptr);
    }
    /// Locate a Matrix of data to the N tuple type safe
    template <class TYPE> StatusCode item(const std::string& name,
                                          Matrix<TYPE>& result)
    {
      return i_item(name, result.m_ptr);
    }

    /// Locate a Matrix of data to the N tuple type safe (CONST)
    template <class TYPE> StatusCode item( const std::string& name,
                                           const Matrix<TYPE>& result)  const
    {
      return i_item(name, result.m_ptr);
    }

    /** Add a scalar data item a N tuple.

        Use this entry point to connect any allowed scalar data type
        to an N-tuple. The value filled, may have any range.
        Do NOT use this entry point to specify an index
        column in a column wise N-tuple.

        @param  name       Name of the column in the column wise N-tuple
        @param  itm        Reference to the Item<TYPE> datatype, which
                           should be connected to the N-tuple.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE> StatusCode
    addItem(const std::string& name, Item<TYPE>& itm)  {
      typedef Range<TYPE> _R;
      return i_addItem(name, 1, "", _R::min(), _R::max(), itm.m_ptr);
    }

    /** Add an simple object item to an N tuple.

        @param  name       Name of the column in the column wise N-tuple
        @param  itm        Reference to the Item<TYPE> datatype, which
                           should be connected to the N-tuple.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE> StatusCode
    addItem(const std::string& name, Item<TYPE*>& itm) {
      return i_addObject(name,itm.m_ptr,typeid(TYPE));
    }

    /** Add an address object item to an N tuple: specialized call

        @param  name       Name of the column in the column wise N-tuple
        @param  itm        Reference to the Item<TYPE> datatype, which
                           should be connected to the N-tuple.

        @return StatusCode indicating success or failure.
    */
    StatusCode
    addItem(const std::string& name, Item<IOpaqueAddress*>& itm) {
      typedef Range<IOpaqueAddress*> _R;
      return i_addItem(name, 1, "", _R::min(), _R::max(), itm.m_ptr);
    }

    /** Add a scalar data item a N tuple with a range.

        Typically this entry point is used to specuify
        index column with a fixed data range for a column wise N-tuple.

        Note: Checks on the data range are not implemented!

        @param  name       Name of the column in the column wise N-tuple
        @param  itm        Reference to the Item<TYPE> datatype, which
                           should be connected to the N-tuple.
        @param  low        Lower edge of client data values allowed
                           to fill into the N-tuple array.
        @param  high       Upper edge of client data values allowed
                           to fill into the N-tuple array.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class RANGE> StatusCode
      addItem(const std::string& name,
              Item<TYPE>& itm,
              const RANGE low,
              const RANGE high)
    {
      return i_addItem( name, 1, "", TYPE(low), TYPE(high), itm.m_ptr);
    }

    /** Add an fixed-size Array of data to a column wise N tuple.

        You should use this entry point to add a FIXED SIZE ARRAY
        to a column wise N-tuple. The dimension of the array must
        be specified.

        @param  name       Name of the column in the column wise N-tuple
        @param  dim        Length of the array to be added to the N-tuple
        @param  array      Reference to the Array<TYPE> datatype, which
                           should be connected to the N-tuple.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE> StatusCode
      addItem(const std::string& name,
              long dim,
              Array<TYPE>& array)
    {
      return i_addItem(name,
                       dim,
                       "",
                       Range<TYPE>::min(),
                       Range<TYPE>::max(),
                       array.m_ptr);
    }

    /** Add an fixed-size Array of data to a column wise N tuple with a range.

        You should use this entry point to add a FIXED SIZE ARRAY
        to a column wise N-tuple. The dimension of the array must
        be specified.

        Note: Checks on the data range are not implemented!

        @param  name       Name of the column in the column wise N-tuple
        @param  dim        Length of the array to be added to the N-tuple
        @param  array      Reference to the Array<TYPE> datatype, which
                           should be connected to the N-tuple.
        @param  low        Lower edge of client data values allowed
                           to fill into the N-tuple array.
        @param  high       Upper edge of client data values allowed
                           to fill into the N-tuple array.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class RANGE> StatusCode
      addItem(const std::string& name,
              long dim,
              Array<TYPE>& array,
              const RANGE low,
              const RANGE high)
    {
      return i_addItem(name,
                       dim,
                       "",
                       TYPE(low),
                       TYPE(high),
                       array.m_ptr);
    }

    /** Add an indexed Array of data to a column wise N tuple with a range.

        You should use this entry point to add a VARIABLE SIZE ARRAY
        to a column wise N-tuple. The dimension of the array is
        unspecified and depends on the data range, which you allowed
        for the index column.

        Hence you have to be careful on the allowed data range of the
        index column, because the index column determines the total
        allocated memory.

        Note: Checks on the data range are not implemented!

        Note: Due to confusion with the entry point to connecting
              a fixed size array the use of this function is
                  *** DEPRECATED ***
              please use addIndexedItem(...) instead.

        @param  name       Name of the column in the column wise N-tuple
        @param  index      Reference to the scalar index column used to
                           determine the size of the array.
        @param  array      Reference to the Array<TYPE> datatype, which
                           should be connected to the N-tuple.
        @param  low        Lower edge of client data values allowed
                           to fill into the N-tuple array.
        @param  high       Upper edge of client data values allowed
                           to fill into the N-tuple array.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class INDEX, class RANGE> StatusCode
      addItem(const std::string& name,
              Item<INDEX>& index,
              Array<TYPE>& array,
              const RANGE low,
              const RANGE high)
    {
      return i_addItem( name,
                        index->range().distance(),
                        index->name(),
                        TYPE(low),
                        TYPE(high),
                        array.m_ptr);
    }

    /** Add an indexed Array of data to a column wise N tuple with a range.

        You should use this entry point to add a VARIABLE SIZE ARRAY
        to a column wise N-tuple. The dimension of the array is
        unspecified and depends on the data range, which you allowed
        for the index column.

        Hence you have to be careful on the allowed data range of the
        index column, because the index column determines the total
        allocated memory.

        Note: Checks on the data range are not implemented!

        @param  name       Name of the column in the column wise N-tuple
        @param  index      Reference to the scalar index column used to
                           determine the size of the array.
        @param  array      Reference to the Array<TYPE> datatype, which
                           should be connected to the N-tuple.
        @param  low        Lower edge of client data values allowed
                           to fill into the N-tuple array.
        @param  high       Upper edge of client data values allowed
                           to fill into the N-tuple array.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class INDEX, class RANGE> StatusCode
      addIndexedItem( const std::string& name,
                      Item<INDEX>& index,
                      Array<TYPE>& array,
                      const RANGE low,
                      const RANGE high)
    {
      return i_addItem( name,
                        index->range().distance(),
                        index->name(),
                        TYPE(low),
                        TYPE(high),
                        array.m_ptr);
    }

    /** Add an indexed Array of data to a column wise N tuple.

        You should use this entry point to add a VARIABLE SIZE ARRAY
        to a column wise N-tuple. The dimension of the array is
        unspecified and depends on the data range, which you allowed
        for the index column.

        Hence, you have to be careful on the allowed data range of the
        index column, because the index column determines the total
        allocated memory.

        Note: Due to confusion with the entry point to connecting
              a fixed size array the use of this member function is
                  *** DEPRECATED ***
              please use addIndexedItem(...) instead.

        @param  name       Name of the column in the column wise N-tuple
        @param  index      Reference to the scalar index column used to
                           determine the size of the array.
        @param  array      Reference to the Array<TYPE> datatype, which
                           should be connected to the N-tuple.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class INDEX> StatusCode
      addItem(const std::string& name,
              Item<INDEX>& index,
              Array<TYPE>& array)
    {
      return i_addItem( name,
                        index->range().distance(),
                        index->name(),
                        Range<TYPE>::min(),
                        Range<TYPE>::max(),
                        array.m_ptr);
    }

    /** Add an indexed Array of data to a column wise N tuple.

        You should use this entry point to add a VARIABLE SIZE ARRAY
        to a column wise N-tuple. The dimension of the array is
        unspecified and depends on the data range, which you allowed
        for the index column.

        Hence, you have to be careful on the allowed data range of the
        index column, because the index column determines the total
        allocated memory.

        @param  name       Name of the column in the column wise N-tuple
        @param  index      Reference to the scalar index column used to
                           determine the size of the array.
        @param  array      Reference to the Array<TYPE> datatype, which
                           should be connected to the N-tuple.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class INDEX> StatusCode
      addIndexedItem( const std::string& name,
                      Item<INDEX>& index,
                      Array<TYPE>& array)
    {
      return i_addItem( name,
                        index->range().distance(),
                        index->name(),
                        Range<TYPE>::min(),
                        Range<TYPE>::max(),
                        array.m_ptr);
    }

    /** Add an fixed size Matrix of data to a column wise N tuple.

        You should use this entry point to add a FIXED SIZE MATRIX
        to a column wise N-tuple. The dimension of the matrix must
        be specified.

        Attention: Do not confuse with entry point to add a
                   VARIABLE SIZE MATRIX!

        @param  name       Name of the column in the column wise N-tuple
        @param  cols       Number of data columns in the Matrix.
        @param  rows       Number of data rows in the Matrix.
        @param  matrix     Reference to the Matrix<TYPE> datatype, which
                           should be connected to the N-tuple.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE> StatusCode
      addItem(const std::string& name,
              long cols,
              long rows,
              Matrix<TYPE>& matrix)
    {
      return i_addItem(name,
                       cols,
                       rows,
                       "",
                       Range<TYPE>::min(),
                       Range<TYPE>::max(),
                       matrix.m_ptr);
    }

    /** Add an fixed size Matrix of data to a column wise N tuple.

        You should use this entry point to add a FIXED SIZE MATRIX
        to a column wise N-tuple. The dimension of the matrix must
        be specified.

        Note: Checks on the data range are not implemented!
        Attention: Do not confuse with entry point to add a
                   VARIABLE SIZE MATRIX!

        @param  name       Name of the column in the column wise N-tuple
        @param  cols       Number of data columns in the Matrix.
        @param  rows       Number of data rows in the Matrix.
        @param  matrix     Reference to the Matrix<TYPE> datatype, which
                           should be connected to the N-tuple.
        @param  low        Lower edge of client data values allowed
                           to fill into the N-tuple matrix.
        @param  high       Upper edge of client data values allowed
                           to fill into the N-tuple matrix.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class RANGE> StatusCode
      addItem(const std::string& name,
              long cols,
              long rows,
              Matrix<TYPE>& result,
              const RANGE low,
              const RANGE high)
    {
      return i_addItem(name,
                       cols,
                       rows,
                       "",
                       TYPE(low),
                       TYPE(high),
                       result.m_ptr);
    }

    /** Add an variable size Matrix of data to a column wise N tuple.

        You should use this entry point to add a VARIABLE SIZE MATRIX
        to a column wise N-tuple. The number of columns of the
        matrix is given by the allowed data range of the index column.
        The number of rows however, which are allowed in the matrix
        must be specified explicitly and cannot be variable.

        Attention: Do not confuse with entry point to add a
                   FIXED SIZE MATRIX.

        Note: Due to confusion with the entry point to connecting
              a fixed size matrix the use of this member function is
                  *** DEPRECATED ***
              please use addIndexedItem(...) instead.

        @param  name       Name of the column in the column wise N-tuple
        @param  index      Reference to the scalar index column used to
                           determine the number of data columns in the matrix
        @param  matrix     Reference to the Matrix<TYPE> datatype, which
                           should be connected to the N-tuple.
        @param  rows       Number of data rows in the Matrix.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class INDEX> StatusCode
      addItem(const std::string& name,
              Item<INDEX>& index,
              Matrix<TYPE>& matrix,
              long rows)
    {
      return i_addItem( name,
                        index->range().distance(),
                        rows,
                        index->name(),
                        Range<TYPE>::min(),
                        Range<TYPE>::max(),
                        matrix.m_ptr);
    }

    /** Add an variable size Matrix of data to a column wise N tuple.

        You should use this entry point to add a VARIABLE SIZE MATRIX
        to a column wise N-tuple. The number of columns of the
        matrix is given by the allowed data range of the index column.
        The number of rows however, which are allowed in the matrix
        must be specified explicitly and cannot be variable.

        Attention: Do not confuse with entry point to add a
                   FIXED SIZE MATRIX.

        @param  name       Name of the column in the column wise N-tuple
        @param  index      Reference to the scalar index column used to
                           determine the number of data columns in the matrix
        @param  rows       Number of data rows in the Matrix.
        @param  matrix     Reference to the Matrix<TYPE> datatype, which
                           should be connected to the N-tuple.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class INDEX> StatusCode
      addIndexedItem( const std::string& name,
                      Item<INDEX>& col_index,
                      long rows,
                      Matrix<TYPE>& matrix)
    {
      return i_addItem( name,
                        col_index->range().distance(),
                        rows,
                        col_index->name(),
                        Range<TYPE>::min(),
                        Range<TYPE>::max(),
                        matrix.m_ptr);
    }

    /** Add an variable size Matrix of data to a column wise N tuple.

        You should use this entry point to add a VARIABLE SIZE MATRIX
        to a column wise N-tuple. The number of columns of the
        matrix is given by the allowed data range of the index column.
        The number of rows however, which are allowed in the matrix
        must be specified explicitly and cannot be variable. Also the
        range of allowed data values to be filled into the data area
        of the matrix can be specified.

        Note: Checks on the data range are not implemented!
        Attention: Do not confuse with entry point to add a
                   FIXED SIZE MATRIX.

        Note: Due to confusion with the entry point to connecting
              a fixed size matrix the use of this member function is
                  *** DEPRECATED ***
              please use addIndexedItem(...) instead.

        @param  name       Name of the column in the column wise N-tuple
        @param  index      Reference to the scalar index column used to
                           determine the number of data columns in the matrix
        @param  matrix     Reference to the Matrix<TYPE> datatype, which
                           should be connected to the N-tuple.
        @param  rows       Number of data rows in the Matrix.
        @param  low        Lower edge of client data values allowed
                           to fill into the N-tuple matrix.
        @param  high       Upper edge of client data values allowed
                           to fill into the N-tuple matrix.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class INDEX, class RANGE> StatusCode
      addItem(const std::string& name,
              Item<INDEX>& index,
              Matrix<TYPE>& matrix,
              long rows,
              const RANGE low,
              const RANGE high)
    {
      return i_addItem( name,
                        index->range().distance(),
                        rows,
                        index->name(),
                        TYPE(low),
                        TYPE(high),
                        matrix.m_ptr);
    }

    /** Add an variable size Matrix of data to a column wise N tuple.

        You should use this entry point to add a VARIABLE SIZE MATRIX
        to a column wise N-tuple. The number of columns of the
        matrix is given by the allowed data range of the index column.
        The number of rows however, which are allowed in the matrix
        must be specified explicitly and cannot be variable. Also the
        range of allowed data values to be filled into the data area
        of the matrix can be specified.

        Note: Checks on the data range are not implemented!
        Attention: Do not confuse with entry point to add a
                   FIXED SIZE MATRIX.

        @param  name       Name of the column in the column wise N-tuple
        @param  index      Reference to the scalar index column used to
                           determine the number of data columns in the matrix
        @param  rows       Number of data rows in the Matrix.
        @param  matrix     Reference to the Matrix<TYPE> datatype, which
                           should be connected to the N-tuple.
        @param  low        Lower edge of client data values allowed
                           to fill into the N-tuple matrix.
        @param  high       Upper edge of client data values allowed
                           to fill into the N-tuple matrix.

        @return StatusCode indicating success or failure.
    */
    template <class TYPE, class INDEX, class RANGE> StatusCode
      addIndexedItem( const std::string& name,
                      Item<INDEX>& index,
                      long rows,
                      Matrix<TYPE>& matrix,
                      const RANGE low,
                      const RANGE high)
    {
      return i_addItem( name,
                        index->range().distance(),
                        rows,
                        index->name(),
                        TYPE(low),
                        TYPE(high),
                        matrix.m_ptr);
    }
  };

  /** Small class representing an N tuple directory in the transient store
  */
  class Directory : public DataObject    {
  public:
    /// Standard constructor
    Directory()   {
    }
    /// Standard destructor
    virtual ~Directory()   {
    }
    /// class ID of the object
    static const CLID& classID()    {
      return CLID_NTupleDirectory;
    }
    /// class ID of the object
    virtual const CLID& clID()    const   {
      return classID();
    }
  };

  /** Small class representing an N tuple file in the transient store
  */
  class File : public Directory    {
  protected:
    /// Physical file name
    std::string   m_name;
    /// Logical file name
    std::string   m_logName;
    /// Access type
    long          m_type;
    /// Flag to indicate wether the file was opened already
    bool          m_isOpen;
  public:
    File() : m_type(0), m_isOpen(false)   {
    }
    /// Standard constructor
    File(long type, const std::string name, const std::string& logName)
    : m_name(name), m_logName(logName), m_type(type), m_isOpen(false)  {
    }
    /// Standard destructor
    virtual ~File()   {
    }
    /// class ID of the object
    static const CLID& classID()    {
      return CLID_NTupleFile;
    }
    /// class ID of the object
    virtual const CLID& clID()    const   {
      return classID();
    }
    /// Set access type
    void setType(const long typ)   {
      m_type = typ;
    }
    /// Return access type
    long type()      const {
      return m_type;
    }
    /// Retrun physical file name
    const std::string& name()    const   {
      return m_name;
    }
    /// Set access type
    void setName(const std::string& nam)    {
      m_name = nam;
    }
    //// Return logical file name
    const std::string& logicalName()    const   {
      return m_logName;
    }
    //// Return logical file name
    void setLogicalName( const std::string& l)  {
      m_logName = l;
    }
    /// Set "open" flag
      void setOpen(bool flag)   {
      m_isOpen = flag;
    }
      /// Access "open" flag
    bool isOpen()   const   {
      return m_isOpen;
    }
  };
  // =========================================================================
  // inhibit certain types by defining specialized templates which do not
  // allow for construction.
  template <>
  class Array <IOpaqueAddress*>
  {
  private:
    Array(){}
  public:
    virtual ~Array() {}
    virtual void dummy() = 0;
  };
  template <>
  class Matrix<IOpaqueAddress*>
  {
  private:
    Matrix(){}
  public:
    virtual ~Matrix() {}
    virtual void dummy() = 0;
  };
  // =========================================================================
#ifndef ALLOW_ALL_TYPES
#else
  typedef Item<bool>              BoolItem;
  typedef Item<char>              CharItem;
  typedef Item<unsigned char>     UCharItem;
  typedef Item<short>             ShortItem;
  typedef Item<unsigned short>    UShortItem;
  typedef Item<long>              LongItem;
  typedef Item<long long>         LongLongItem;
  typedef Item<unsigned long>     ULongItem;
  typedef Item<unsigned long long> ULongLongItem;
  typedef Item<int>               IntItem;
  typedef Item<unsigned int>      UIntItem;
  typedef Item<float>             FloatItem;
  typedef Item<double>            DoubleItem;
  typedef Array<bool>             BoolArray;
  typedef Array<char>             CharArray;
  typedef Array<unsigned char>    UCharArray;
  typedef Array<short>            ShortArray;
  typedef Array<unsigned short>   UShortArray;
  typedef Array<long>             LongArray;
  typedef Array<unsigned long>    ULongArray;
  typedef Array<int>              IntArray;
  typedef Array<unsigned int>     UIntArray;
  typedef Array<float>            FloatArray;
  typedef Array<double>           DoubleArray;
  typedef Matrix<bool>            BoolMatrix;
  typedef Matrix<char>            CharMatrix;
  typedef Matrix<unsigned char>   UCharMatrix;
  typedef Matrix<short>           ShortMatrix;
  typedef Matrix<unsigned short>  UShortMatrix;
  typedef Matrix<long>            LongMatrix;
  typedef Matrix<unsigned long>   ULongMatrix;
  typedef Matrix<int>             IntMatrix;
  typedef Matrix<unsigned int>    UIntMatrix;
  typedef Matrix<float>           FloatMatrix;
  typedef Matrix<double>          DoubleMatrix;
#endif

  template <class T>
  inline std::ostream& operator<<(std::ostream& s, const Item<T>& obj)
  {
    return s << T(obj);
  }
} // end of namespace NTuple

// Useful:
typedef SmartDataPtr<NTuple::Tuple>     NTuplePtr;
typedef SmartDataPtr<NTuple::Directory> NTupleDirPtr;
typedef SmartDataPtr<NTuple::File>      NTupleFilePtr;

#endif // GAUDIKERNEL_NTUPLE_H
