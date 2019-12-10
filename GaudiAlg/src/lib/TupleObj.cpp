/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <cstdarg>
#include <map>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/GaudiException.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/TupleObj.h"
#include "GaudiAlg/Tuples.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/integer_traits.hpp"
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

  template <typename T>
  struct tuple_type_;

  template <>
  struct tuple_type_<typename Tuples::TupleObj::Float> {
    static constexpr const char* fmt = "F";
    static constexpr const char* typ = "floats";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::Double> {
    static constexpr const char* fmt = "D";
    static constexpr const char* typ = "doubles";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::Bool> {
    static constexpr const char* fmt = "I";
    static constexpr const char* typ = "bools";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::Char> {
    static constexpr const char* fmt = "I";
    static constexpr const char* typ = "chars";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::UChar> {
    static constexpr const char* fmt = "I";
    static constexpr const char* typ = "uchars";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::Short> {
    static constexpr const char* fmt = "I";
    static constexpr const char* typ = "shorts";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::UShort> {
    static constexpr const char* fmt = "I";
    static constexpr const char* typ = "ushorts";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::Int> {
    static constexpr const char* fmt = "I";
    static constexpr const char* typ = "ints";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::UInt> {
    static constexpr const char* fmt = "I";
    static constexpr const char* typ = "uints";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::LongLong> {
    static constexpr const char* fmt = "ULL";
    static constexpr const char* typ = "longlongs";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::ULongLong> {
    static constexpr const char* fmt = "ULL";
    static constexpr const char* typ = "ulonglongs";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::Address> {
    static constexpr const char* fmt = "IOpaqueAddress*";
    static constexpr const char* typ = "addresses";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::FArray> {
    static constexpr const char* fmt = "FArray";
    static constexpr const char* typ = "farray";
  };
  template <>
  struct tuple_type_<typename Tuples::TupleObj::FMatrix> {
    static constexpr const char* fmt = "FMatrix";
    static constexpr const char* typ = "fmatrix";
  };

  // helper functions to simplify things...
  template <typename C, typename AddItem>
  typename C::mapped_type::pointer create_( Tuples::TupleObj* parent, C& container, const std::string& name,
                                            AddItem addItem ) {
    using element_t = typename C::mapped_type::element_type;
    using map_t     = struct tuple_type_<element_t>;
    auto item       = container.emplace( name, std::make_unique<element_t>() );
    if ( !item.second ) { parent->Error( std::string{map_t::typ} + " ('" + name + "'): item is not inserted" ); }
    StatusCode sc = addItem( name, *( item.first->second ) );
    if ( sc.isFailure() ) { parent->Error( std::string{map_t::typ} + " ('" + name + "'): item is not added", sc ); }
    if ( !parent->addItem( name, map_t::fmt ) ) {
      parent->Error( std::string{map_t::typ} + " ('" + name + "'): item is not unique" );
    }
    return item.first->second.get();
  }

  template <typename C, typename... ExtraArgs>
  typename C::mapped_type::pointer find_or_create( Tuples::TupleObj* parent, const std::string& name, C& map,
                                                   ExtraArgs&&... ea ) {
    using pointer   = typename C::mapped_type::pointer;
    using reference = std::add_lvalue_reference_t<std::remove_pointer_t<pointer>>;
    auto found      = map.find( name );
    return found != map.end() ? found->second.get()
                              : create_( parent, map, name, [&]( const std::string& n, reference i ) {
                                  return parent->tuple()->addItem( n, i, std::forward<ExtraArgs>( ea )... );
                                } );
  }

  template <typename Container, typename UT, typename... ExtraArgs>
  StatusCode column_( Tuples::TupleObj* parent, Container& container, const std::string& name, UT&& value,
                      ExtraArgs&&... ea ) {
    if ( parent->invalid() ) return Tuples::ErrorCodes::InvalidTuple;
    auto item = find_or_create( parent, name, container, std::forward<ExtraArgs>( ea )... );
    if ( !item ) return Tuples::ErrorCodes::InvalidColumn;
    *item = std::forward<UT>( value );
    return StatusCode::SUCCESS;
  }

  struct TuplesCategory : StatusCode::Category {
    const char* name() const override { return "Tuples"; }

    bool isRecoverable( StatusCode::code_t ) const override { return false; }

    std::string message( StatusCode::code_t code ) const override {
      switch ( static_cast<Tuples::ErrorCodes>( code ) ) {
      case Tuples::ErrorCodes::InvalidTuple:
        return "InvalidTuple";
      case Tuples::ErrorCodes::InvalidColumn:
        return "InvalidColumn";
      case Tuples::ErrorCodes::InvalidOperation:
        return "InvalidOperation";
      case Tuples::ErrorCodes::InvalidObject:
        return "InvalidObject";
      case Tuples::ErrorCodes::InvalidItem:
        return "InvalidItem";
      case Tuples::ErrorCodes::TruncateValue:
        return "TruncateValue";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };
} // namespace

STATUSCODE_ENUM_IMPL( Tuples::ErrorCodes, TuplesCategory )

namespace Tuples {
  namespace Local {
    class Counter final {
    public:
      // constructor
      Counter( std::string msg = " Misbalance " ) : m_message( std::move( msg ) ) {}
      // destructor
      ~Counter() { report(); }
      // make the increment
      long increment( const std::string& object ) { return ++m_map[object]; }
      // make the decrement
      long decrement( const std::string& object ) { return --m_map[object]; }
      // current count
      long counts( const std::string& object ) const { return m_map.at( object ); }
      // make a report
      void report() const {
        for ( auto& entry : m_map ) {
          if ( entry.second != 0 )
            std::cout << "Tuples::TupleObj WARNING " << m_message << "'" << entry.first << "' Counts = " << entry.second
                      << std::endl;
        }
      };

    private:
      std::map<std::string, long> m_map;
      std::string                 m_message;
    };

    /** @var s_InstanceCounter
     *  The instance counter for all 'TupleObj' based classes
     *  @author Vanya BELYAEV Ivan.Belyaev@Ivan.Belyaev@itep.ru
     *  @date   2004-01-19
     */
    static Counter s_InstanceCounter{" Create/Destroy      (mis)balance "};
  } // namespace Local
} // namespace Tuples
// ============================================================================
// Standard constructor
// ============================================================================
Tuples::TupleObj::TupleObj( std::string name, NTuple::Tuple* tuple, const CLID& clid, const Tuples::Type type )
    //
    : m_name( std::move( name ) ), m_tuple( tuple ), m_clid( clid ), m_type( type ) {
  // make counts
  Tuples::Local::s_InstanceCounter.increment( m_name );
}
// ============================================================================
// destructor
// ============================================================================
Tuples::TupleObj::~TupleObj() {
  // make counts
  Tuples::Local::s_InstanceCounter.decrement( m_name );
}
// ============================================================================
// write a record to NTuple
// ============================================================================
StatusCode Tuples::TupleObj::write() {
  if ( invalid() ) return ErrorCodes::InvalidTuple;
  return tuple()->write();
}
// ============================================================================
namespace {
  /** Very simple tokenizer for TupleObj
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date   2002-07-14
   */
  std::vector<std::string> tokenize( std::string_view value, std::string_view separators = " " ) {
    std::vector<std::string> tokens;
    auto                     it1 = value.begin();
    auto                     it2 = value.begin();
    while ( value.end() != it1 && value.end() != it2 ) {
      it2 = std::find_first_of( it1, value.end(), separators.begin(), separators.end() );
      if ( it2 != it1 ) {
        tokens.emplace_back( value, it1 - value.begin(), it2 - it1 );
        it1 = it2;
      } else {
        ++it1;
      }
    }
    return tokens;
  }
} // namespace
// ============================================================================
StatusCode Tuples::TupleObj::fill( const char* format... ) {
  // check the underlying tuple
  if ( invalid() ) return ErrorCodes::InvalidTuple;
  // decode format string into tokens
  auto tokens = tokenize( format, " ,;" );
  /// decode arguments
  va_list valist;
  va_start( valist, format );
  // loop over all tokens
  StatusCode status = StatusCode::SUCCESS;
  for ( auto token = tokens.cbegin(); tokens.cend() != token && status.isSuccess(); ++token ) {
    double val = va_arg( valist, double );
    status     = column( *token, val );
    if ( status.isFailure() ) Error( "fill(): Can not add column '" + *token + "' " );
  }
  // mandatory !!!
  va_end( valist );
  //
  return status;
}

// ============================================================================
// put IOpaqueAddress in NTuple (has sense only for Event tag collection Ntuples)
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, IOpaqueAddress* address ) {
  if ( !evtColType() ) return ErrorCodes::InvalidOperation;
  if ( !address ) return Error( "column('" + name + "') IOpaqueAddress* is NULL!", ErrorCodes::InvalidObject );
  return column_( this, m_addresses, name, address );
}

// ============================================================================
// put IOpaqueAddress in NTuple (has sense only for Event tag collection Ntuples)
// ============================================================================
StatusCode Tuples::TupleObj::column( IOpaqueAddress* address ) { return column( "Address", address ); }

// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, float value ) {
  return column_( this, m_floats, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, double value ) {
  return column_( this, m_doubles, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, char value ) {
  return column_( this, m_chars, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, char value, char minv, char maxv ) {
  return column_( this, m_chars, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, unsigned char value ) {
  return column_( this, m_uchars, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, unsigned char value, unsigned char minv,
                                     unsigned char maxv ) {
  return column_( this, m_uchars, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, short value ) {
  return column_( this, m_shorts, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, const short value, const short minv, const short maxv ) {
  return column_( this, m_shorts, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, const unsigned short value ) {
  return column_( this, m_ushorts, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, unsigned short value, unsigned short minv,
                                     unsigned short maxv ) {
  return column_( this, m_ushorts, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, int value ) {
  return column_( this, m_ints, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, int value, int minv, int maxv ) {
  return column_( this, m_ints, name, value, minv, maxv );
}
// ============================================================================
Tuples::TupleObj::Int* Tuples::TupleObj::ints( const std::string& name, int minv, int maxv ) {
  return find_or_create( this, name, m_ints, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, unsigned int value ) {
  return column_( this, m_uints, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, unsigned int value, unsigned int minv,
                                     unsigned int maxv ) {
  return column_( this, m_uints, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, const long value ) {
  Warning( "'long' has different sizes on 32/64 bit systems. Casting '" + name + "' to 'long long'",
           StatusCode::SUCCESS )
      .ignore();
  return column( name, static_cast<long long>( value ) );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, const long value, const long minv, const long maxv ) {
  Warning( "'long' has different sizes on 32/64 bit systems. Casting '" + name + "' to 'long long'",
           StatusCode::SUCCESS )
      .ignore();
  return column( name, static_cast<long long>( value ), static_cast<long long>( minv ),
                 static_cast<long long>( maxv ) );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, const unsigned long value ) {
  Warning( "'unsigned long' has different sizes on 32/64 bit systems. Casting '" + name + "' to 'unsigned long long'",
           StatusCode::SUCCESS )
      .ignore();
  return column( name, static_cast<unsigned long long>( value ) );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, const unsigned long value, const unsigned long minv,
                                     const unsigned long maxv ) {
  Warning( "'unsigned long' has different sizes on 32/64 bit systems. Casting '" + name + "' to 'unsigned long long'",
           StatusCode::SUCCESS )
      .ignore();
  return column( name, static_cast<unsigned long long>( value ), static_cast<unsigned long long>( minv ),
                 static_cast<unsigned long long>( maxv ) );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, const long long value ) {
  return column_( this, m_longlongs, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, long long value, long long minv, long long maxv ) {
  return column_( this, m_longlongs, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, unsigned long long value ) {
  return column_( this, m_ulonglongs, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, unsigned long long value, unsigned long long minv,
                                     unsigned long long maxv ) {
  return column_( this, m_ulonglongs, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( const std::string& name, bool value ) {
  return column_( this, m_bools, name, value );
}
// ============================================================================
// retrieve (book on demand) array-items for ntuple
// ============================================================================
Tuples::TupleObj::FArray* Tuples::TupleObj::fArray( const std::string& name, Tuples::TupleObj::Int* length ) {
  // existing array ?
  auto found = m_farrays.find( name );
  if ( m_farrays.end() != found ) return found->second.get();
  return create_( this, m_farrays, name,
                  [&]( const std::string& n, FArray& i ) { return this->tuple()->addIndexedItem( n, *length, i ); } );
}
// ============================================================================
// retrieve (book on demand) array-items for ntuple (fixed)
// ============================================================================
Tuples::TupleObj::FArray* Tuples::TupleObj::fArray( const std::string& name, const Tuples::TupleObj::MIndex& rows ) {
  // existing array ?
  auto found = m_arraysf.find( name );
  if ( m_arraysf.end() != found ) return found->second.get();
  return create_( this, m_arraysf, name,
                  [&]( const std::string& n, FArray& i ) { return this->tuple()->addItem( n, rows, i ); } );
}
// ============================================================================
// retrieve (book on demand) matrix-items for ntuple
// ============================================================================
Tuples::TupleObj::FMatrix* Tuples::TupleObj::fMatrix( const std::string& name, Tuples::TupleObj::Int* length,
                                                      const Tuples::TupleObj::MIndex& cols ) {
  // existing array ?
  auto found = m_fmatrices.find( name );
  if ( m_fmatrices.end() != found ) return found->second.get();
  return create_( this, m_fmatrices, name, [&]( const std::string& n, FMatrix& i ) {
    return this->tuple()->addIndexedItem( n, *length, cols, i );
  } );
}
// ============================================================================
// retrieve (book on demand) matrix-items for ntuple (fixed)
// ============================================================================
Tuples::TupleObj::FMatrix* Tuples::TupleObj::fMatrix( const std::string& name, const Tuples::TupleObj::MIndex& rows,
                                                      const Tuples::TupleObj::MIndex& cols ) {
  // existing array ?
  auto found = m_matricesf.find( name );
  if ( m_matricesf.end() != found ) return found->second.get();
  return create_( this, m_matricesf, name,
                  [&]( const std::string& n, FMatrix& i ) { return this->tuple()->addItem( n, rows, cols, i ); } );
}
// ============================================================================
// The END
// ============================================================================
