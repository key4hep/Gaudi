// $Id: TupleObj.cpp,v 1.8 2007/09/28 11:47:29 marcocle Exp $
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
namespace Tuples
{
  namespace Local
  {
    class Counter
    {
    public:
      // constructor
      Counter ( const std::string& msg = " Misbalance ")
        : m_map     ()
        , m_message ( msg )
      {}
      // destructor
      ~Counter() { report() ; m_map.clear() ;}
      // make the increment
      long increment ( const std::string& object ) { return ++m_map[object] ; }
      // make the decrement
      long decrement ( const std::string& object ) { return --m_map[object] ; }
      // current count
      long counts    ( const std::string& object ) { return   m_map[object] ; }
      // make a report
      void report() const
      {
        for ( Map::const_iterator entry = m_map.begin() ;
              m_map.end() != entry ; ++entry )
        {
          if( 0 == entry->second ) { continue ; }
          std::cout << "Tuples::TupleObj WARNING "          << m_message
                    << "'" << entry->first << "' Counts = " << entry->second
                    << std::endl ;
        }
      };

    private:
      typedef std::map<std::string,long> Map;
      Map         m_map     ;
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
( const std::string&    name  ,
  NTuple::Tuple*        tuple ,
  const CLID&           clid  ,
  const Tuples::Type    type  )
//
  : m_name     ( name )
  , m_tuple    ( tuple )
  , m_clid     ( clid )
  , m_type     ( type )
// for error handling
  , m_refCount ( 0 )
// columns
  , m_bools     ()
  , m_chars     ()
  , m_uchars    ()
  , m_shorts    ()
  , m_ushorts   ()
  , m_ints      ()
  , m_uints     ()
  , m_longlongs ()
  , m_ulonglongs()
  , m_floats    ()
  , m_doubles   ()
  , m_addresses ()
  , m_farrays   ()
  , m_arraysf   ()
  , m_fmatrices ()
  , m_matricesf ()
//
  , m_items     ()
{
  // make counts
  Tuples::Local::s_InstanceCounter.increment ( m_name ) ;
}
// ============================================================================
// destructor
// ============================================================================
Tuples::TupleObj::~TupleObj()
{
  {// delete 'bool' columns
    for( Bools::iterator it = m_bools.begin() ;
         m_bools.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_bools.clear() ;
  }
  {// delete 'char' columns
    for( Chars::iterator it = m_chars.begin() ;
         m_chars.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_chars.clear() ;
  }
  {// delete 'unsigned char' columns
    for( UChars::iterator it = m_uchars.begin() ;
         m_uchars.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_uchars.clear() ;
  }
  {// delete 'short' columns
    for( Shorts::iterator it = m_shorts.begin() ;
         m_shorts.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_shorts.clear() ;
  }
  {// delete 'unsigned short' columns
    for( UShorts::iterator it = m_ushorts.begin() ;
         m_ushorts.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_ushorts.clear() ;
  }
  {// delete 'int' columns
    for( Ints::iterator it = m_ints.begin() ;
         m_ints.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_ints.clear() ;
  }
  {// delete 'unsigned int' columns
    for( UInts::iterator it = m_uints.begin() ;
         m_uints.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_uints.clear() ;
  }
  {// delete 'longlong' columns
    for( LongLongs::iterator it = m_longlongs.begin() ;
         m_longlongs.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_longlongs.clear() ;
  }
  {// delete 'ulonglong' columns
    for( ULongLongs::iterator it = m_ulonglongs.begin() ;
         m_ulonglongs.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_ulonglongs.clear() ;
  }
  {// delete 'float' columns
    for( Floats::iterator it = m_floats.begin() ;
         m_floats.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_floats.clear() ;
  }
  {// delete 'double' columns
    for( Doubles::iterator it = m_doubles.begin() ;
         m_doubles.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_doubles.clear() ;
  }
  {// delete 'fArray' columns
    for( FArrays::iterator it = m_farrays.begin() ;
         m_farrays.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_farrays.clear() ;
  }
  {// delete 'fArray' columns
    for( FArrays::iterator it = m_arraysf.begin() ;
         m_arraysf.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_arraysf.clear() ;
  }
  { // destroy and clean all "addresses"
    for( Addresses::iterator it = m_addresses.begin() ;
         m_addresses.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_addresses.clear();
  }
  { // destroy and clean all "matrices"
    for( FMatrices::iterator it = m_fmatrices.begin() ;
         m_fmatrices.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_fmatrices.clear();
  }
  { // destroy and clean all "matrices" (fixed)
    for( FMatrices::iterator it = m_matricesf.begin() ;
         m_matricesf.end() != it ; ++it )
    { if( 0 != it->second ) { delete it->second ; } }
    m_matricesf.clear();
  }

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
    std::string::const_iterator it1 = value.begin() ;
    std::string::const_iterator it2 = value.begin() ;
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
  for( Tokens::const_iterator token = tokens.begin() ;
       tokens.end() != token && status.isSuccess() ; ++token )
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
  if ( 0 == address )
  { return Error ( "column('" + name +
                   "') IOpaqueAddress* is NULL!" , InvalidObject ) ; }
  Address* item = addresses( name );
  if ( 0 == item      ) { return InvalidItem      ; }
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
  Floats::iterator found = m_floats.find( name ) ;
  if ( m_floats.end() != found ) { return found->second ; }
  Float* item = new Float() ;
  m_floats[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item );
  if ( sc.isFailure() )
  { Error ( "floats ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "F" ) )
  { Error ( "floats ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::Double*   Tuples::TupleObj::doubles
( const std::string& name )
{
  Doubles::iterator found = m_doubles.find( name ) ;
  if ( m_doubles.end() != found ) { return found->second ; }
  Double* item = new Double() ;
  m_doubles[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item );
  if ( sc.isFailure() )
  { Error ( "doubles ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "D" ) )
  { Error ( "doubles ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::Bool*   Tuples::TupleObj::bools
( const std::string& name )
{
  Bools::iterator found = m_bools.find( name ) ;
  if( m_bools.end() != found ) { return found->second ; }
  Bool* item = new Bool() ;
  m_bools[ name ] = item ;
  StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "bools ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "bools ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::Char*   Tuples::TupleObj::chars
( const std::string& name )
{
  Chars::iterator found = m_chars.find( name ) ;
  if( m_chars.end() != found ) { return found->second ; }
  Char* item = new Char() ;
  m_chars[ name ] = item ;
  StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "chars ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "chars ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::Char* Tuples::TupleObj::chars
( const std::string& name ,
  const char         minv ,
  const char         maxv )
{
  Chars::iterator found = m_chars.find( name ) ;
  if( m_chars.end() != found ) { return found->second ; }
  Char* item = new Char() ;
  m_chars[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item , minv , maxv );
  if( sc.isFailure() )
  { Error ( "chars ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "chars ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::UChar* Tuples::TupleObj::uchars
( const std::string& name )
{
  UChars::iterator found = m_uchars.find( name ) ;
  if( m_uchars.end() != found ) { return found->second ; }
  UChar* item = new UChar() ;
  m_uchars[ name ] = item ;
  StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "uchars ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "uchars ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::UChar* Tuples::TupleObj::uchars
( const std::string&  name ,
  const unsigned char minv ,
  const unsigned char maxv )
{
  UChars::iterator found = m_uchars.find( name ) ;
  if( m_uchars.end() != found ) { return found->second ; }
  UChar* item = new UChar() ;
  m_uchars[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item , minv , maxv );
  if( sc.isFailure() )
  { Error ( "uchars ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "uchars ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::Short*   Tuples::TupleObj::shorts
( const std::string& name )
{
  Shorts::iterator found = m_shorts.find( name ) ;
  if( m_shorts.end() != found ) { return found->second ; }
  Short* item = new Short() ;
  m_shorts[ name ] = item ;
  StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "shorts ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "shorts ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::Short* Tuples::TupleObj::shorts
( const std::string& name ,
  const short        minv ,
  const short        maxv )
{
  Shorts::iterator found = m_shorts.find( name ) ;
  if( m_shorts.end() != found ) { return found->second ; }
  Short* item = new Short() ;
  m_shorts[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item , minv , maxv );
  if( sc.isFailure() )
  { Error ( "shorts ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "shorts ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::UShort* Tuples::TupleObj::ushorts
( const std::string& name )
{
  UShorts::iterator found = m_ushorts.find( name ) ;
  if( m_ushorts.end() != found ) { return found->second ; }
  UShort* item = new UShort() ;
  m_ushorts[ name ] = item ;
  StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "ushorts ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "ushorts ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::UShort* Tuples::TupleObj::ushorts
( const std::string&   name ,
  const unsigned short minv ,
  const unsigned short maxv )
{
  UShorts::iterator found = m_ushorts.find( name ) ;
  if( m_ushorts.end() != found ) { return found->second ; }
  UShort* item = new UShort() ;
  m_ushorts[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item , minv , maxv );
  if( sc.isFailure() )
  { Error ( "ushorts ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "ushorts ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::Int*   Tuples::TupleObj::ints
( const std::string& name )
{
  Ints::iterator found = m_ints.find( name ) ;
  if( m_ints.end() != found ) { return found->second ; }
  Int* item = new Int() ;
  m_ints[ name ] = item ;
  StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "ints ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "ints ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::Int* Tuples::TupleObj::ints
( const std::string& name ,
  const int          minv ,
  const int          maxv )
{
  Ints::iterator found = m_ints.find( name ) ;
  if( m_ints.end() != found ) { return found->second ; }
  Int* item = new Int() ;
  m_ints[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item , minv , maxv );
  if( sc.isFailure() )
  { Error ( "ints ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "ints ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::UInt*   Tuples::TupleObj::uints
( const std::string& name )
{
  UInts::iterator found = m_uints.find( name ) ;
  if( m_uints.end() != found ) { return found->second ; }
  UInt* item = new UInt() ;
  m_uints[ name ] = item ;
  StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "uints ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "uints ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::UInt* Tuples::TupleObj::uints
( const std::string& name ,
  const unsigned int minv ,
  const unsigned int maxv )
{
  UInts::iterator found = m_uints.find( name ) ;
  if( m_uints.end() != found ) { return found->second ; }
  UInt* item = new UInt() ;
  m_uints[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item , minv , maxv );
  if( sc.isFailure() )
  { Error ( "uints ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "I" ) )
  { Error ( "uints ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::LongLong* Tuples::TupleObj::longlongs
( const std::string& name )
{
  LongLongs::iterator found = m_longlongs.find( name ) ;
  if( m_longlongs.end() != found ) { return found->second ; }
  LongLong* item = new LongLong() ;
  m_longlongs[ name ] = item ;
  StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "ints ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "ULL" ) )
  { Error ( "ints ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::LongLong* Tuples::TupleObj::longlongs
( const std::string&       name ,
  const long long minv ,
  const long long maxv )
{
  LongLongs::iterator found = m_longlongs.find( name ) ;
  if( m_longlongs.end() != found ) { return found->second ; }
  LongLong* item = new LongLong() ;
  m_longlongs[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item , minv , maxv );
  if( sc.isFailure() )
  { Error ( "longlongs ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "ULL" ) )
  { Error ( "longlongs ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::ULongLong* Tuples::TupleObj::ulonglongs
( const std::string& name )
{
  ULongLongs::iterator found = m_ulonglongs.find( name ) ;
  if( m_ulonglongs.end() != found ) { return found->second ; }
  ULongLong* item = new ULongLong() ;
  m_ulonglongs[ name ] = item ;
  StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "ulonglongs ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "ULL" ) )
  { Error ( "ulonglongs ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::ULongLong* Tuples::TupleObj::ulonglongs
( const std::string&       name ,
  const unsigned long long minv ,
  const unsigned long long maxv )
{
  ULongLongs::iterator found = m_ulonglongs.find( name ) ;
  if( m_ulonglongs.end() != found ) { return found->second ; }
  ULongLong* item = new ULongLong() ;
  m_ulonglongs[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item , minv , maxv );
  if( sc.isFailure() )
  { Error ( "ulonglongs ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "ULL" ) )
  { Error ( "ulonglongs ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
Tuples::TupleObj::Address* Tuples::TupleObj::addresses
( const std::string& name )
{
  Addresses::iterator found = m_addresses.find( name ) ;
  if( m_addresses.end() != found ) { return found->second ; }
  Address* item = new Address() ;
  m_addresses[ name ] = item ;
  const StatusCode sc = tuple()->addItem( name , *item );
  if( sc.isFailure() )
  { Error ( "addresses ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "IOpaqueAddress*" ) )
  { Error ( "addresses ('" + name + "'): item is not unique"     ) ; }
  return item ;
}
// ============================================================================
// retrieve (book on demand) array-items for ntuple
// ============================================================================
Tuples::TupleObj::FArray* Tuples::TupleObj::fArray
( const std::string&      name  ,
  Tuples::TupleObj::Int* length )
{
  // existing array ?
  FArrays::iterator found = m_farrays.find( name ) ;
  if( m_farrays.end() != found ) { return found->second ; }
  // create new array
  FArray* array = new FArray () ;
  m_farrays[ name] =      array    ;
  const StatusCode sc = tuple() -> addIndexedItem( name , *length , *array) ;
  if( sc.isFailure() )
  { Error ( "farray ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "FArray" ) )
  { Error ( "farray ('" + name + "'): item is not unique"     ) ; }
  return array ;
}
// ============================================================================
// retrieve (book on demand) array-items for ntuple (fixed)
// ============================================================================
Tuples::TupleObj::FArray* Tuples::TupleObj::fArray
( const std::string&              name ,
  const Tuples::TupleObj::MIndex& rows )
{
  // existing array ?
  FArrays::iterator found = m_arraysf.find( name ) ;
  if( m_arraysf.end() != found ) { return found->second ; }
  // create new array
  FArray* array = new FArray () ;
  m_arraysf[ name] =      array    ;
  const StatusCode sc = tuple() -> addItem ( name , rows , *array) ;
  if( sc.isFailure() )
  { Error ( "array ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "FArray" ) )
  { Error ( "array ('" + name + "'): item is not unique"     ) ; }
  return array ;
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
  FMatrices::iterator found = m_fmatrices.find( name ) ;
  if( m_fmatrices.end() != found ) { return found->second ; }
  // create new array
  FMatrix* matrix = new FMatrix () ;
  m_fmatrices[ name] =  matrix   ;
  const StatusCode sc =
    tuple() -> addIndexedItem( name , *length , cols , *matrix ) ;
  if( sc.isFailure() )
  { Error ( "fmatrix ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "FMatrix" ) )
  { Error ( "fmatrix ('" + name + "'): item is not unique"     ) ; }
  return matrix ;
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
  FMatrices::iterator found = m_matricesf.find( name ) ;
  if( m_matricesf.end() != found ) { return found->second ; }
  // create new array
  FMatrix* matrix = new FMatrix () ;
  m_matricesf[ name] =  matrix   ;
  const StatusCode sc =
    tuple() -> addItem( name , rows , cols , *matrix ) ;
  if( sc.isFailure() )
  { Error ( "matrix ('" + name + "'): item is not added",  sc ) ; }
  if ( !addItem ( name , "FMatrix" ) )
  { Error ( "matrix ('" + name + "'): item is not unique"     ) ; }
  return matrix ;
}
// ============================================================================
// The END
// ============================================================================
