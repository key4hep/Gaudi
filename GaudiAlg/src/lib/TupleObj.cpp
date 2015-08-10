// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <cstdarg>
#include <algorithm>
#include <map>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/GaudiException.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Tuples.h"
#include "GaudiAlg/TupleObj.h"
// ============================================================================
// Boost
// ============================================================================
#include  "boost/integer_traits.hpp"
#include  "boost/static_assert.hpp"
// ============================================================================
/** @file TupleObj.cpp
 *
 *  Implementation file for class Tuples::TupleObj
 *
 *  @date 2004-01-23
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */
// ============================================================================
namespace {

    template <typename T> struct tuple_type_ ;

    template <> struct tuple_type_<typename Tuples::TupleObj::Float>  
    { static constexpr const char* fmt = "F"; 
      static constexpr const char* typ = "floats";  };
    template <> struct tuple_type_<typename Tuples::TupleObj::Double> 
    { static constexpr const char* fmt = "D"; 
      static constexpr const char* typ = "doubles"; };
    template <> struct tuple_type_<typename Tuples::TupleObj::Bool>   
    { static constexpr const char* fmt = "I"; 
      static constexpr const char* typ = "bools";};
    template <> struct tuple_type_<typename Tuples::TupleObj::Char>   
    { static constexpr const char* fmt = "I"; 
      static constexpr const char* typ = "chars";};
    template <> struct tuple_type_<typename Tuples::TupleObj::UChar>  
    { static constexpr const char* fmt = "I"; 
      static constexpr const char* typ = "uchars";};
    template <> struct tuple_type_<typename Tuples::TupleObj::Short>  
    { static constexpr const char* fmt = "I"; 
      static constexpr const char* typ = "shorts";};
    template <> struct tuple_type_<typename Tuples::TupleObj::UShort> 
    { static constexpr const char* fmt = "I"; 
      static constexpr const char* typ = "ushorts";};
    template <> struct tuple_type_<typename Tuples::TupleObj::Int>    
    { static constexpr const char* fmt = "I"; 
      static constexpr const char* typ = "ints";};
    template <> struct tuple_type_<typename Tuples::TupleObj::UInt>   
    { static constexpr const char* fmt = "I"; 
      static constexpr const char* typ = "uints";};
    template <> struct tuple_type_<typename Tuples::TupleObj::LongLong> 
    { static constexpr const char* fmt = "ULL"; 
      static constexpr const char* typ = "longlongs";};
    template <> struct tuple_type_<typename Tuples::TupleObj::ULongLong> 
    { static constexpr const char* fmt = "ULL"; 
      static constexpr const char* typ = "ulonglongs";};
    template <> struct tuple_type_<typename Tuples::TupleObj::Address> 
    { static constexpr const char* fmt = "IOpaqueAddress*" ; 
      static constexpr const char* typ = "addresses";};
    template <> struct tuple_type_<typename Tuples::TupleObj::FArray> 
    { static constexpr const char* fmt = "FArray"; 
      static constexpr const char* typ = "farray";};
    template <> struct tuple_type_<typename Tuples::TupleObj::FMatrix> 
    { static constexpr const char* fmt = "FMatrix"; 
      static constexpr const char* typ = "fmatrix"; };

    // helper function to simplify things...
    template <typename C, typename AddItem>
    typename C::mapped_type::pointer create_(Tuples::TupleObj* parent, C& container, const std::string& name, AddItem addItem  ) {
        using element_t = typename C::mapped_type::element_type;
        using map_t = struct tuple_type_<element_t> ;
        auto item = container.insert({ name , std::unique_ptr<element_t>{new element_t()} }) ;
        if (!item.second)
        { parent->Error ( std::string{map_t::typ} + " ('" + name + "'): item is not inserted"   ) ; }
        StatusCode sc = addItem( name , *(item.first->second) );
        if ( sc.isFailure() )
        { parent->Error ( std::string{map_t::typ} + " ('" + name + "'): item is not added",  sc ) ; }
        if ( !parent->addItem(name,map_t::fmt) )
        { parent->Error ( std::string{map_t::typ} + " ('" + name + "'): item is not unique"     ) ; }
        return item.first->second.get() ;
    }

}
namespace Tuples
{
  namespace Local
  {
    class Counter
    {
    public:
      // constructor
      Counter ( std::string msg = " Misbalance ")
        : m_message ( std::move(msg) )
      {}
      // destructor
      ~Counter() { report(); }
      // make the increment
      long increment ( const std::string& object ) { return ++m_map[object]; }
      // make the decrement
      long decrement ( const std::string& object ) { return --m_map[object]; }
      // current count
      long counts    ( const std::string& object ) { return   m_map[object]; }
      // make a report
      void report() const
      {
        for ( auto& entry : m_map )
        {
          if( 0 == entry.second ) { continue ; }
          std::cout << "Tuples::TupleObj WARNING "          << m_message
                    << "'" << entry.first << "' Counts = " << entry.second
                    << std::endl ;
        }
      };

    private:
      std::map<std::string,long> m_map     ;
      std::string m_message ;
    };

    /** @var s_InstanceCounter
     *  The instance counter for all 'TupleObj' based classes
     *  @author Vanya BELYAEV Ivan.Belyaev@Ivan.Belyaev@itep.ru
     *  @date   2004-01-19
     */
    static Counter s_InstanceCounter ( " Create/Destroy      (mis)balance " ) ;
  }
}
// ============================================================================
// Standard constructor
// ============================================================================
Tuples::TupleObj::TupleObj
( std::string           name  ,
  NTuple::Tuple*        tuple ,
  const CLID&           clid  ,
  const Tuples::Type    type  )
//
  : m_name     ( std::move(name) )
  , m_tuple    ( tuple )
  , m_clid     ( clid )
  , m_type     ( type )
// for error handling
  , m_refCount ( 0 )
{
  // make counts
  Tuples::Local::s_InstanceCounter.increment ( m_name ) ;
}
// ============================================================================
// destructor
// ============================================================================
Tuples::TupleObj::~TupleObj()
{
  // make counts
  Tuples::Local::s_InstanceCounter.decrement ( m_name ) ;
}
// ============================================================================
// release the reference to TupleObj
// if reference counter becomes zero,
// object will be automatically deleted
// ============================================================================
void Tuples::TupleObj::release ()
{
  // decrease the reference counter
  if( 0 < refCount() ) { --m_refCount; }
  // check references
  if( 0 != refCount() ) { return; }
  // delete the object
  delete this  ;
}
// ============================================================================
// write a record to NTuple
// ============================================================================
StatusCode Tuples::TupleObj::write ()
{
  if ( invalid()  ) { return InvalidTuple ; }
  return tuple()->write() ;
}
// ============================================================================
namespace
{
  /// useful typedef
  typedef std::vector<std::string> Tokens;
  /** Very simple tokenizer for TupleObj
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date   2002-07-14
   */
  size_t tokenize( const std::string& value            ,
                   Tokens&            tokens           ,
                   const std::string& separators = " " )
  {
    // reset the existing tokens
    tokens.clear();
    if( value       .empty () ) { return tokens.size () ; }
    auto it1 = value.begin() ;
    auto it2 = value.begin() ;
    while( value.end() != it1 && value.end() != it2 )
    {
      it2 = std::find_first_of( it1                  ,
                                value.end         () ,
                                separators.begin  () ,
                                separators.end    () ) ;
      if( it2 != it1 )
      {
        std::string aux( value , it1 - value.begin() , it2 - it1 ) ;
        tokens.push_back( aux ) ;
        it1  = it2 ;
      }
      else { ++it1 ; }

    }
    return tokens.size();
  }
}
// ============================================================================
StatusCode Tuples::TupleObj::fill( const char*  format ... )
{
  // check the underlying tuple
  if ( invalid()      ) { return InvalidTuple ; }
  // decode format string into tokens
  Tokens tokens ;
  tokenize( format , tokens , " ,;" );
  if ( tokens.empty() ) { return StatusCode::SUCCESS ; }
  /// decode arguments
  va_list valist ;
  va_start( valist , format ) ;
  // loop over all tokens
  StatusCode status = StatusCode::SUCCESS ;
  for( auto token = tokens.cbegin() ;
       tokens.cend() != token && status.isSuccess() ; ++token )
  {
    const double val = va_arg( valist , double );
    status = column( *token , val );
    if( status.isFailure() )
    { Error ( "fill(): Can not add column '" + *token + "' " ) ; }
  }
  // mandatory !!!
  va_end( valist );
  //
  return status ;
}

// ============================================================================
// put IOpaqueAddress in NTuple (has sense only for Event tag collection Ntuples)
// ============================================================================
StatusCode Tuples::TupleObj::column
( const std::string&    name    ,
  IOpaqueAddress*       address )
{
  if (  invalid    () ) { return InvalidTuple     ; }
  if ( !evtColType () ) { return InvalidOperation ; }
  if ( !address )
  { return Error ( "column('" + name +
                   "') IOpaqueAddress* is NULL!" , InvalidObject ) ; }
  Address* item = addresses( name );
  if ( !item      ) { return InvalidItem      ; }
  *item = address ;
  return StatusCode::SUCCESS ;
}

// ============================================================================
// put IOpaqueAddress in NTuple (has sense only for Event tag collection Ntuples)
// ============================================================================
StatusCode Tuples::TupleObj::column
( IOpaqueAddress*       address )
{
  return column ( "Address" , address ) ;
}

// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string& name   ,
                                      const float        value  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  Float* item = floats ( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string& name   ,
                                      const double       value  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  Double * item = doubles ( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&  name  ,
                                      const char          value )
{
  if ( invalid() ) { return InvalidTuple  ; }
  Char* item = chars( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string& name  ,
                                      const char         value ,
                                      const char         minv  ,
                                      const char         maxv  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  Char* item = chars ( name , minv , maxv ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const unsigned char value )
{
  if ( invalid() ) { return InvalidTuple  ; }
  UChar* item = uchars( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name ,
                                      const unsigned char value ,
                                      const unsigned char minv  ,
                                      const unsigned char maxv  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  UChar* item = uchars ( name , minv , maxv ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const short          value )
{
  if ( invalid() ) { return InvalidTuple  ; }
  Short* item = shorts( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&  name  ,
                                      const short         value ,
                                      const short         minv  ,
                                      const short         maxv  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  Short* item = shorts ( name , minv , maxv ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const unsigned short value )
{
  if ( invalid() ) { return InvalidTuple  ; }
  UShort* item = ushorts( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const unsigned short value ,
                                      const unsigned short minv  ,
                                      const unsigned short maxv  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  UShort* item = ushorts ( name , minv , maxv ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const int            value )
{
  if ( invalid() ) { return InvalidTuple  ; }
  Int* item = ints( name ) ;
  if ( !item ) { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const int            value ,
                                      const int            minv  ,
                                      const int            maxv  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  Int* item = ints ( name , minv , maxv ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const unsigned int   value )
{
  if ( invalid() ) { return InvalidTuple  ; }
  UInt* item = uints( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const unsigned int   value ,
                                      const unsigned int   minv  ,
                                      const unsigned int   maxv  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  UInt* item = uints ( name , minv , maxv ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const long           value )
{
  Warning( "'long' has different sizes on 32/64 bit systems. Casting '" +
           name + "' to 'long long'", StatusCode::SUCCESS ).ignore();
  return column( name, (long long)value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const long           value ,
                                      const long           minv  ,
                                      const long           maxv  )
{
  Warning( "'long' has different sizes on 32/64 bit systems. Casting '" +
           name + "' to 'long long'", StatusCode::SUCCESS ).ignore();
  return column( name, 
                 (long long)value, 
                 (long long)minv, 
                 (long long)maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const unsigned long  value )
{
  Warning( "'unsigned long' has different sizes on 32/64 bit systems. Casting '" +
           name  + "' to 'unsigned long long'", StatusCode::SUCCESS ).ignore();
  return column( name, (unsigned long long)value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const unsigned long  value ,
                                      const unsigned long  minv  ,
                                      const unsigned long  maxv  )
{
  Warning( "'unsigned long' has different sizes on 32/64 bit systems. Casting '" +
           name + "' to 'unsigned long long'", StatusCode::SUCCESS ).ignore();
  return column( name, 
                 (unsigned long long)value, 
                 (unsigned long long)minv, 
                 (unsigned long long)maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const long long      value )
{
  if ( invalid() ) { return InvalidTuple  ; }
  LongLong* item = longlongs( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&   name  ,
                                      const long long      value ,
                                      const long long      minv  ,
                                      const long long      maxv  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  LongLong* item = longlongs ( name , minv , maxv ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&       name  ,
                                      const unsigned long long value )
{
  if ( invalid() ) { return InvalidTuple  ; }
  ULongLong* item = ulonglongs( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string&       name  ,
                                      const unsigned long long value ,
                                      const unsigned long long minv  ,
                                      const unsigned long long maxv  )
{
  if ( invalid() ) { return InvalidTuple  ; }
  ULongLong* item = ulonglongs ( name , minv , maxv ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode Tuples::TupleObj::column ( const std::string& name  ,
                                      const bool         value )
{
  if ( invalid() ) { return InvalidTuple  ; }
  Bool* item = bools( name ) ;
  if ( !item )     { return InvalidColumn ; }
  *item = value ;
  return StatusCode::SUCCESS ;
}
// ============================================================================
Tuples::TupleObj::Float* Tuples::TupleObj::floats
( const std::string& name )
{
  auto found = m_floats.find( name ) ;
  if ( m_floats.end() != found ) { return found->second.get() ; }
  return create_(this, m_floats,name,
                 [&](const std::string& n, Float& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::Double*   Tuples::TupleObj::doubles
( const std::string& name )
{
  auto found = m_doubles.find( name ) ;
  if ( m_doubles.end() != found ) { return found->second.get() ; }
  return create_(this, m_doubles,name,
                 [&](const std::string& n, Double& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::Bool*   Tuples::TupleObj::bools
( const std::string& name )
{
  auto found = m_bools.find( name ) ;
  if( m_bools.end() != found ) { return found->second.get() ; }
  return create_(this, m_bools, name,
                 [&](const std::string& n, Bool& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::Char*   Tuples::TupleObj::chars
( const std::string& name )
{
  auto found = m_chars.find( name ) ;
  if( m_chars.end() != found ) { return found->second.get() ; }
  return create_(this, m_chars, name,
                 [&](const std::string& n, Char& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::Char* Tuples::TupleObj::chars
( const std::string& name ,
  const char         minv ,
  const char         maxv )
{
  auto found = m_chars.find( name ) ;
  if( m_chars.end() != found ) { return found->second.get() ; }
  return create_(this, m_chars, name,
                 [&](const std::string& n, Char& i) 
                 { return this->tuple()->addItem(n,i,minv,maxv); });
}
// ============================================================================
Tuples::TupleObj::UChar* Tuples::TupleObj::uchars
( const std::string& name )
{
  auto found = m_uchars.find( name ) ;
  if( m_uchars.end() != found ) { return found->second.get() ; }
  return create_(this, m_uchars, name,
                 [&](const std::string& n, UChar& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::UChar* Tuples::TupleObj::uchars
( const std::string&  name ,
  const unsigned char minv ,
  const unsigned char maxv )
{
  auto found = m_uchars.find( name ) ;
  if( m_uchars.end() != found ) { return found->second.get() ; }
  return create_(this, m_uchars, name,
                 [&](const std::string& n, UChar& i) 
                 { return this->tuple()->addItem(n,i,minv,maxv); });
}
// ============================================================================
Tuples::TupleObj::Short*   Tuples::TupleObj::shorts
( const std::string& name )
{
  auto found = m_shorts.find( name ) ;
  if( m_shorts.end() != found ) { return found->second.get() ; }
  return create_(this, m_shorts, name,
                 [&](const std::string& n, Short& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::Short* Tuples::TupleObj::shorts
( const std::string& name ,
  const short        minv ,
  const short        maxv )
{
  auto found = m_shorts.find( name ) ;
  if( m_shorts.end() != found ) { return found->second.get(); }
  return create_(this, m_shorts, name,
                 [&](const std::string& n, Short& i) 
                 { return this->tuple()->addItem(n,i,minv,maxv); });
}
// ============================================================================
Tuples::TupleObj::UShort* Tuples::TupleObj::ushorts
( const std::string& name )
{
  auto found = m_ushorts.find( name ) ;
  if( m_ushorts.end() != found ) { return found->second.get() ; }
  return create_(this, m_ushorts, name,
                 [&](const std::string& n, UShort& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::UShort* Tuples::TupleObj::ushorts
( const std::string&   name ,
  const unsigned short minv ,
  const unsigned short maxv )
{
  auto found = m_ushorts.find( name ) ;
  if( m_ushorts.end() != found ) { return found->second.get() ; }
  return create_(this, m_ushorts, name,
                 [&](const std::string& n, UShort& i) 
                 { return this->tuple()->addItem(n,i,minv,maxv); });
}
// ============================================================================
Tuples::TupleObj::Int*   Tuples::TupleObj::ints
( const std::string& name )
{
  auto found = m_ints.find( name ) ;
  if( m_ints.end() != found ) { return found->second.get() ; }
  return create_(this, m_ints, name,
                 [&](const std::string& n, Int& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::Int* Tuples::TupleObj::ints
( const std::string& name ,
  const int          minv ,
  const int          maxv )
{
  auto found = m_ints.find( name ) ;
  if( m_ints.end() != found ) { return found->second.get() ; }
  return create_(this, m_ints, name,
                 [&](const std::string& n, Int& i) 
                 { return this->tuple()->addItem(n,i,minv,maxv); });
}
// ============================================================================
Tuples::TupleObj::UInt*   Tuples::TupleObj::uints
( const std::string& name )
{
  auto found = m_uints.find( name ) ;
  if( m_uints.end() != found ) { return found->second.get() ; }
  return create_(this, m_uints, name,
                 [&](const std::string& n, UInt& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::UInt* Tuples::TupleObj::uints
( const std::string& name ,
  const unsigned int minv ,
  const unsigned int maxv )
{
  auto found = m_uints.find( name ) ;
  if( m_uints.end() != found ) { return found->second.get() ; }
  return create_(this, m_uints, name,
                 [&](const std::string& n, UInt& i) 
                 { return this->tuple()->addItem(n,i,minv,maxv); });
}
// ============================================================================
Tuples::TupleObj::LongLong* Tuples::TupleObj::longlongs
( const std::string& name )
{
  auto found = m_longlongs.find( name ) ;
  if( m_longlongs.end() != found ) { return found->second.get() ; }
  return create_(this, m_longlongs, name,
                 [&](const std::string& n, LongLong& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::LongLong* Tuples::TupleObj::longlongs
( const std::string&       name ,
  const long long minv ,
  const long long maxv )
{
  auto found = m_longlongs.find( name ) ;
  if( m_longlongs.end() != found ) { return found->second.get() ; }
  return create_(this, m_longlongs, name,
                 [&](const std::string& n, LongLong& i) 
                 { return this->tuple()->addItem(n,i,minv,maxv); });
}
// ============================================================================
Tuples::TupleObj::ULongLong* Tuples::TupleObj::ulonglongs
( const std::string& name )
{
  auto found = m_ulonglongs.find( name ) ;
  if( m_ulonglongs.end() != found ) { return found->second.get() ; }
  return create_(this, m_ulonglongs, name,
                 [&](const std::string& n, ULongLong& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
Tuples::TupleObj::ULongLong* Tuples::TupleObj::ulonglongs
( const std::string&       name ,
  const unsigned long long minv ,
  const unsigned long long maxv )
{
  auto found = m_ulonglongs.find( name ) ;
  if( m_ulonglongs.end() != found ) { return found->second.get() ; }
  return create_(this, m_ulonglongs, name,
                 [&](const std::string& n, ULongLong& i) 
                 { return this->tuple()->addItem(n,i,minv,maxv); });
}
// ============================================================================
Tuples::TupleObj::Address* Tuples::TupleObj::addresses
( const std::string& name )
{
  auto found = m_addresses.find( name ) ;
  if( m_addresses.end() != found ) { return found->second.get() ; }
  return create_(this, m_addresses, name,
                 [&](const std::string& n, Address& i) 
                 { return this->tuple()->addItem(n,i); });
}
// ============================================================================
// retrieve (book on demand) array-items for ntuple
// ============================================================================
Tuples::TupleObj::FArray* Tuples::TupleObj::fArray
( const std::string&      name  ,
  Tuples::TupleObj::Int* length )
{
  // existing array ?
  auto found = m_farrays.find( name ) ;
  if( m_farrays.end() != found ) { return found->second.get() ; }
  return create_(this, m_farrays, name,
                 [&](const std::string& n, FArray& i) 
                 { return this->tuple()->addIndexedItem(n,*length,i); });
}
// ============================================================================
// retrieve (book on demand) array-items for ntuple (fixed)
// ============================================================================
Tuples::TupleObj::FArray* Tuples::TupleObj::fArray
( const std::string&              name ,
  const Tuples::TupleObj::MIndex& rows )
{
  // existing array ?
  auto found = m_arraysf.find( name ) ;
  if( m_arraysf.end() != found ) { return found->second.get() ; }
  return create_(this, m_arraysf, name,
                 [&](const std::string& n, FArray& i) 
                 { return this->tuple()->addItem(n,rows,i); });
}
// ============================================================================
// retrieve (book on demand) matrix-items for ntuple
// ============================================================================
Tuples::TupleObj::FMatrix*
Tuples::TupleObj::fMatrix
( const std::string&              name   ,
  Tuples::TupleObj::Int*          length ,
  const Tuples::TupleObj::MIndex& cols   )
{
  // existing array ?
  auto found = m_fmatrices.find( name ) ;
  if( m_fmatrices.end() != found ) { return found->second.get() ; }
  return create_(this, m_fmatrices, name,
                 [&](const std::string& n, FMatrix& i) 
                 { return this->tuple()->addIndexedItem(n,*length,cols,i); });
}
// ============================================================================
// retrieve (book on demand) matrix-items for ntuple (fixed)
// ============================================================================
Tuples::TupleObj::FMatrix*
Tuples::TupleObj::fMatrix
( const std::string&              name   ,
  const Tuples::TupleObj::MIndex& rows   ,
  const Tuples::TupleObj::MIndex& cols   )
{
  // existing array ?
  auto found = m_matricesf.find( name ) ;
  if( m_matricesf.end() != found ) { return found->second.get() ; }
  return create_(this, m_matricesf, name,
                 [&](const std::string& n, FMatrix& i) 
                 { return this->tuple()->addItem(n,rows,cols,i); });
}
// ============================================================================
// The END
// ============================================================================
