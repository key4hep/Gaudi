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
  auto create_( Tuples::TupleObj* parent, C& container, std::string_view name, AddItem addItem ) {
    using map_t     = struct tuple_type_<typename C::element_type>;
    auto [iter, ok] = container.emplace( name );
    if ( !ok ) { parent->Error( fmt::format( "{} ('{}'): item is not inserted", map_t::typ, name ) ).ignore(); }
    StatusCode sc = addItem( std::string{ name }, *iter );
    if ( sc.isFailure() ) {
      parent->Error( fmt::format( "{} ('{}'): item is not added", map_t::typ, name ), sc ).ignore();
    }
    if ( !parent->addItem( std::string{ name }, map_t::fmt ) ) {
      parent->Error( fmt::format( "{} ('{}'): item is not unique", map_t::typ, name ) ).ignore();
    }
    return &*iter;
  }

  template <typename C, typename... ExtraArgs>
  auto find_or_create( Tuples::TupleObj* parent, std::string_view name, C& map, ExtraArgs&&... ea ) {
    auto found = map.find( name );
    return found != map.end() ? &*found : create_( parent, map, name, [&]( std::string_view n, auto& i ) {
      return parent->tuple()->addItem( std::string{ n }, i, std::forward<ExtraArgs>( ea )... );
    } );
  }

  template <typename Container, typename UT, typename... ExtraArgs>
  StatusCode column_( Tuples::TupleObj* parent, Container& container, std::string_view name, UT&& value,
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
      long increment( std::string_view object ) { return ++get( object ); }
      // make the decrement
      long decrement( std::string_view object ) { return --get( object ); }
      // current count
      long counts( std::string_view object ) const { return get( object ); }
      // make a report
      void report() const {
        for ( auto& entry : m_map ) {
          if ( entry.second != 0 )
            std::cout << "Tuples::TupleObj WARNING " << m_message << "'" << entry.first << "' Counts = " << entry.second
                      << std::endl;
        }
      };

    private:
      long& get( std::string_view sv ) {
        auto i = m_map.find( sv );
        if ( i == m_map.end() ) { i = m_map.emplace( std::pair{ std::string{ sv }, 0 } ).first; }
        return i->second;
      }
      long get( std::string_view sv ) const {
        auto i = m_map.find( sv );
        if ( i == m_map.end() ) throw;
        return i->second;
      }
      std::map<std::string, long, std::less<>> m_map;
      std::string                              m_message;
    };

    /** @var s_InstanceCounter
     *  The instance counter for all 'TupleObj' based classes
     *  @author Vanya BELYAEV Ivan.Belyaev@Ivan.Belyaev@itep.ru
     *  @date   2004-01-19
     */
    static Counter s_InstanceCounter{ " Create/Destroy      (mis)balance " };
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

// ============================================================================
// put IOpaqueAddress in NTuple (has sense only for Event tag collection Ntuples)
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, IOpaqueAddress* address ) {
  if ( !evtColType() ) return ErrorCodes::InvalidOperation;
  if ( !address )
    return Error( fmt::format( "column('{}') IOpaqueAddress* is NULL!", name ), ErrorCodes::InvalidObject );
  return column_( this, m_addresses, name, address );
}

// ============================================================================
// put IOpaqueAddress in NTuple (has sense only for Event tag collection Ntuples)
// ============================================================================
StatusCode Tuples::TupleObj::column( IOpaqueAddress* address ) { return column( "Address", address ); }

// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, float value ) {
  return column_( this, m_floats, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, double value ) {
  return column_( this, m_doubles, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, char value ) {
  return column_( this, m_chars, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, char value, char minv, char maxv ) {
  return column_( this, m_chars, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, unsigned char value ) {
  return column_( this, m_uchars, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, unsigned char value, unsigned char minv,
                                     unsigned char maxv ) {
  return column_( this, m_uchars, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, short value ) {
  return column_( this, m_shorts, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, const short value, const short minv, const short maxv ) {
  return column_( this, m_shorts, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, const unsigned short value ) {
  return column_( this, m_ushorts, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, unsigned short value, unsigned short minv,
                                     unsigned short maxv ) {
  return column_( this, m_ushorts, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, int value ) { return column_( this, m_ints, name, value ); }
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, int value, int minv, int maxv ) {
  return column_( this, m_ints, name, value, minv, maxv );
}
// ============================================================================
Tuples::TupleObj::Int* Tuples::TupleObj::ints( std::string_view name, int minv, int maxv ) {
  return find_or_create( this, name, m_ints, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, unsigned int value ) {
  return column_( this, m_uints, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, unsigned int value, unsigned int minv, unsigned int maxv ) {
  return column_( this, m_uints, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, const long value ) {
  Warning( fmt::format( "'long' has different sizes on 32/64 bit systems. Casting '{}' to 'long long'", name ),
           StatusCode::SUCCESS )
      .ignore();
  return column( name, static_cast<long long>( value ) );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, const long value, const long minv, const long maxv ) {
  Warning( fmt::format( "'long' has different sizes on 32/64 bit systems. Casting '{}' to 'long long'", name ),
           StatusCode::SUCCESS )
      .ignore();
  return column( name, static_cast<long long>( value ), static_cast<long long>( minv ),
                 static_cast<long long>( maxv ) );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, const unsigned long value ) {
  Warning( fmt::format(
               "'unsigned long' has different sizes on 32/64 bit systems. Casting '{}' to 'unsigned long long'", name ),
           StatusCode::SUCCESS )
      .ignore();
  return column( name, static_cast<unsigned long long>( value ) );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, const unsigned long value, const unsigned long minv,
                                     const unsigned long maxv ) {
  Warning( fmt::format(
               "'unsigned long' has different sizes on 32/64 bit systems. Casting '{}' to 'unsigned long long'", name ),
           StatusCode::SUCCESS )
      .ignore();
  return column( name, static_cast<unsigned long long>( value ), static_cast<unsigned long long>( minv ),
                 static_cast<unsigned long long>( maxv ) );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, const long long value ) {
  return column_( this, m_longlongs, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, long long value, long long minv, long long maxv ) {
  return column_( this, m_longlongs, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, unsigned long long value ) {
  return column_( this, m_ulonglongs, name, value );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, unsigned long long value, unsigned long long minv,
                                     unsigned long long maxv ) {
  return column_( this, m_ulonglongs, name, value, minv, maxv );
}
// ============================================================================
StatusCode Tuples::TupleObj::column( std::string_view name, bool value ) {
  return column_( this, m_bools, name, value );
}
// ============================================================================
// retrieve (book on demand) array-items for ntuple
// ============================================================================
Tuples::TupleObj::FArray* Tuples::TupleObj::fArray( std::string_view name, Tuples::TupleObj::Int* length ) {
  // existing array ?
  auto found = m_farrays.find( name );
  if ( m_farrays.end() != found ) return &*found;
  return create_( this, m_farrays, name, [&]( std::string_view n, FArray& i ) {
    return this->tuple()->addIndexedItem( std::string{ n }, *length, i );
  } );
}
// ============================================================================
// retrieve (book on demand) array-items for ntuple (fixed)
// ============================================================================
Tuples::TupleObj::FArray* Tuples::TupleObj::fArray( std::string_view name, const Tuples::TupleObj::MIndex& rows ) {
  // existing array ?
  auto found = m_arraysf.find( name );
  if ( m_arraysf.end() != found ) return &*found;
  return create_( this, m_arraysf, name, [&]( std::string_view n, FArray& i ) {
    return this->tuple()->addItem( std::string{ n }, rows, i );
  } );
}
// ============================================================================
// retrieve (book on demand) matrix-items for ntuple
// ============================================================================
Tuples::TupleObj::FMatrix* Tuples::TupleObj::fMatrix( std::string_view name, Tuples::TupleObj::Int* length,
                                                      const Tuples::TupleObj::MIndex& cols ) {
  // existing array ?
  auto found = m_fmatrices.find( name );
  if ( m_fmatrices.end() != found ) return &*found;
  return create_( this, m_fmatrices, name, [&]( std::string_view n, FMatrix& i ) {
    return this->tuple()->addIndexedItem( std::string{ n }, *length, cols, i );
  } );
}
// ============================================================================
// retrieve (book on demand) matrix-items for ntuple (fixed)
// ============================================================================
Tuples::TupleObj::FMatrix* Tuples::TupleObj::fMatrix( std::string_view name, const Tuples::TupleObj::MIndex& rows,
                                                      const Tuples::TupleObj::MIndex& cols ) {
  // existing array ?
  auto found = m_matricesf.find( name );
  if ( m_matricesf.end() != found ) return &*found;
  return create_( this, m_matricesf, name, [&]( std::string_view n, FMatrix& i ) {
    return this->tuple()->addItem( std::string{ n }, rows, cols, i );
  } );
}
// ============================================================================
// The END
// ============================================================================
