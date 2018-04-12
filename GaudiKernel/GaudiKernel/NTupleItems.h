#ifndef GAUDI_NTUPLESVC_NTUPLEITEMS_H
#define GAUDI_NTUPLESVC_NTUPLEITEMS_H 1

// The converter understands all items
#define ALLOW_ALL_TYPES

// STL include files
#include <vector>

// Framework include files
#include "GaudiKernel/System.h"
#include "NTuple.h"
/**
 * @class NTupleItems NTupleItems.h GaudiKernel/NTupleItems.h
 *
 * NTupleItems namespace parts definition
 * This header file is not intended to be included by the public!
 * It's only used for the ntuple service
 *
 * @author M.Frank
 * @author Sebastien Ponce
*/

// Forward declarations
class IDataProviderSvc;
class IConversionSvc;

namespace NTuple
{
  // Local forward declarations
  template <class TYP>
  class DataItem;
  template <class TYP>
  class _DataImp;
  template <class TYP>
  class _ItemImp;
  template <class TYP>
  class _ArrayImp;
  template <class TYP>
  class _MatrixImp;

  /** Concrete class discribing basic data items in an N tuple.
  */
  template <class TYP>
  class _DataImp : virtual public _Data<TYP>
  {
    /// Inhibit Copy Constructor
    _DataImp( const _DataImp& ) = delete;

  protected:
    typedef const std::string&    CSTR;
    typedef const std::type_info& CTYPE;
    /// Entire buffer length
    long m_length;
    /// Pointer to N tuple
    INTuple* m_tuple;
    /// _Column name
    std::string m_name;
    /// Check that values are within a certain range while filling
    std::string m_index;
    /// Pointer to index item
    mutable INTupleItem* m_indexItem = nullptr;
    /// _Column type
    DataTypeInfo::Type m_type;
    /// Buffer with default value
    TYP m_def;
    /// Check that values are within a certain range while filling
    Range<TYP> m_range;
    /// Item type information
    const std::type_info& m_info;

  public:
    /// Set type definition to make life more easy easy
    typedef Range<TYP> ItemRange;
    /// Standard Constructor
    _DataImp( INTuple* tup, std::string name, const std::type_info& info, std::string index, long len, TYP low,
              TYP high, TYP def )
        : m_length( len )
        , m_tuple( tup )
        , m_name( std::move( name ) )
        , m_index( std::move( index ) )
        , m_def( std::move( def ) )
        , m_range( std::move( low ), std::move( high ) )
        , m_info( info )
    {
      m_type         = typeid( TYP ) == typeid( void* ) ? DataTypeInfo::POINTER : DataTypeInfo::ID( info );
      this->m_buffer = new TYP[m_length];
      reset();
    }
    /// Standard destructor
    ~_DataImp() override { delete[] this->m_buffer; }
    /// Get proper type name
    std::string typeName() const override { return System::typeinfoName( this->typeID() ); }
    /// Reset to default
    void reset() override { std::fill_n( this->m_buffer, m_length, m_def ); }
    /// Number of items filled
    long filled() const override
    {
      int len = 1;
      int nd  = ndim();
      if ( m_length > 1 ) {
        for ( int l = 0; l < nd - 1; l++ ) {
          len *= dim( l );
        }
        if ( indexItem() ) {
          long* ll = (long*)m_indexItem->buffer();
          len *= *ll;
        } else if ( nd > 0 ) {
          len *= dim( nd - 1 );
        }
      }
      return len;
    }
    /// Pointer to index column (if present, 0 else)
    INTupleItem* indexItem() override
    {
      if ( !m_indexItem ) m_indexItem = m_tuple->find( m_index );
      return m_indexItem;
    }
    /// Pointer to index column (if present, 0 else) (CONST)
    const INTupleItem* indexItem() const override
    {
      if ( !m_indexItem ) m_indexItem = m_tuple->find( m_index );
      return m_indexItem;
    }
    /// Compiler type ID
    const std::type_info& typeID() const override { return m_info; }
    /// Size of entire object
    long size() const override { return m_length * sizeof( TYP ); }
    /// Destruct object
    void release() override { delete this; }
    /// Is the tuple have an index column?
    bool hasIndex() const override { return m_index.length() > 0; }
    /// Access the index _Column
    const std::string& index() const override { return m_index; }
    /// Access _Column name
    const std::string& name() const override { return m_name; }
    /// TYP information of the item
    long type() const override { return m_type; }
    /// Set the properties of the _Column
    void setType( long t ) override { m_type = DataTypeInfo::Type( t ); }
    /// Set default value
    void setDefault( const TYP val ) override { m_def = val; }
    /// Access the range if specified
    const ItemRange& range() const override { return m_range; }
    /// Access the buffer length
    long length() const override { return m_length; }
    /// Access data buffer (CONST)
    const void* buffer() const override { return this->m_buffer; }
    /// Access data buffer
    virtual void* buffer() { return this->m_buffer; }
    /// Dimension
    long ndim() const override { return 0; }
    /// Access individual dimensions
    long dim( long i ) const override { return ( i == 0 ) ? 1 : 0; }
    /// Access to hosting ntuple
    INTuple* tuple() override { return m_tuple; }
  };

  /** Concrete class discribing a column in a N tuple.
  */
  template <class TYP>
  class _ItemImp : virtual public _DataImp<TYP>, virtual public _Item<TYP>
  {

  public:
    /// Set type definition to make life more easy easy
    typedef Range<TYP> ItemRange;
    /// Standard Constructor
    _ItemImp( INTuple* tup, const std::string& name, const std::type_info& info, TYP min, TYP max, TYP def )
        : _DataImp<TYP>( tup, name, info, "", 1, min, max, def )
    {
    }
    /// Compiler type ID
    // virtual const std::type_info& typeID() const             { return typeid(NTuple::_Item<TYP>);  }
    /// Set default value
    void setDefault( const TYP val ) override { this->m_def = val; }
    /// Access the range if specified
    const ItemRange& range() const override { return this->m_range; }
    /// Size of entire object
    long size() const override { return this->m_length * sizeof( TYP ); }
  };

  /** Concrete class discribing a column-array in a N tuple.
  */
  template <class TYP>
  class _ArrayImp : virtual public _DataImp<TYP>, virtual public _Array<TYP>
  {
  public:
    /// Set type definition to make life more easy easy
    typedef Range<TYP> ItemRange;
    /// Standard Constructor
    _ArrayImp( INTuple* tup, const std::string& name, const std::type_info& typ, const std::string& index, long len,
               TYP min, TYP max, TYP def )
        : _DataImp<TYP>( tup, name, typ, index, len, min, max, def )
    {
    }
    /// Compiler type ID
    // virtual const std::type_info& typeID() const             { return typeid(NTuple::_Array<TYP>); }
    /// Set default value
    void setDefault( const TYP val ) override { this->m_def = val; }
    /// Access the range if specified
    const ItemRange& range() const override { return this->m_range; }
    /// Size of entire object
    long size() const override { return this->m_length * sizeof( TYP ); }
    /// Dimension
    long ndim() const override { return 1; }
    /// Access individual dimensions
    long dim( long i ) const override { return ( i != 0 || this->hasIndex() ) ? 0 : this->m_length; }
  };

  /** Concrete class discribing a matrix column in a N tuple.
  */
  template <class TYP>
  class _MatrixImp : virtual public _DataImp<TYP>, virtual public _Matrix<TYP>
  {
  public:
    /// Set type definition to make life more easy easy
    typedef Range<TYP> ItemRange;
    /// Standard Constructor
    _MatrixImp( INTuple* tup, const std::string& name, const std::type_info& typ, const std::string& index, long ncol,
                long nrow, TYP min, TYP max, TYP def )
        : _DataImp<TYP>( tup, name, typ, index, nrow * ncol, min, max, def )
    {
      this->m_rows = nrow;
    }
    /// Compiler type ID
    // virtual const std::type_info& typeID() const             { return typeid(NTuple::_Matrix<TYP>);}
    /// Set default value
    void setDefault( const TYP val ) override { this->m_def = val; }
    /// Access the range if specified
    const ItemRange& range() const override { return this->m_range; }
    /// Size of entire object
    long size() const override { return this->m_length * sizeof( TYP ); }
    /// Dimension
    long ndim() const override { return 2; }
    /// Access individual dimensions
    long dim( long i ) const override
    {
      return ( this->hasIndex() ) ? ( ( i == 0 ) ? this->m_rows : this->m_length / this->m_rows )
                                  : ( ( i == 1 ) ? this->m_length / this->m_rows : this->m_rows );
    }
  };
} // end name space NTuple
#endif // GAUDI_NTUPLESVC_NTUPLEITEMS_H
