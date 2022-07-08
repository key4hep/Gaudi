/***********************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

#if __cplusplus >= 201703
#  include <string_view>
#else
#  include <experimental/string_view>
namespace std {
  using experimental::string_view;
}
#endif

namespace Gaudi::Details {
  /** std::string wrapper for static strings where identical values actually share the memory.
   *
   *  This class is useful when dealing with several duplicated strings so that the actual
   *  value is kept in memory only once and all the SharedString instances point to it.
   *
   *  SharedString is automatically comparable and convertible to std::string.
   */
  class SharedString final {
  public:
    /// Create a new SharedString checking if the value is already in the shared storage
    /// otherwise adding it.
    SharedString( std::string_view s = {} ) : m_s{ SharedString::get( s ) } {}

    operator std::string() const { return *m_s; }
    operator std::string_view() const { return *m_s; }

    bool operator==( std::string_view other ) const { return *m_s == other; }
    bool operator==( const SharedString& other ) const { return m_s == other.m_s; }

    template <typename T>
    bool operator==( const T& other ) const {
      return *m_s == other;
    }

  private:
    const std::string* m_s;

    static const std::string* get( std::string_view s ) {
      if ( s.empty() ) { return {}; }
      return &*( storage.emplace( s ).first );
    }

    static std::unordered_set<std::string> storage;
  };

  /** Helper to record a property identifier as a sequence of SharedString instances.
   *
   *  A PropertyId instance initilized from a string like `SomeName.OtherName.AnotherLevel.PropertyName` will
   *  use internally a vector of 4 SharedString instances so that the storage for the various components
   *  can be shared with other PropertyId instances.
   *
   *  To be afficiently used as key in an `std::unordered_map`, PropertyId caches the hash computed from the
   *  string it was constructed from.
   */
  class PropertyId final {
  public:
    PropertyId( const std::string& s ) : PropertyId( std::string_view{ s } ) {}
    PropertyId( std::string_view s ) {
      m_hash = std::hash<std::string_view>()( s );
      if ( !s.empty() ) {
        m_chunks.reserve( std::count( begin( s ), end( s ), '.' ) + 1 );
        while ( true ) {
          if ( auto pos = s.find( '.' ); pos != std::string_view::npos ) {
            m_chunks.emplace_back( s.substr( 0, pos ) );
            s.remove_prefix( pos + 1 );
          } else {
            m_chunks.emplace_back( s );
            break;
          }
        }
      }
    }

    std::string str() const {
      auto        it = m_chunks.begin();
      std::string s{ *it++ };
      while ( it != m_chunks.end() ) {
        s += '.';
        s += *it++;
      }
      return s;
    }

    operator std::string() const { return str(); }

    std::size_t hash() const noexcept { return m_hash; }

  private:
    std::vector<SharedString> m_chunks;
    std::size_t               m_hash;
    friend bool               operator==( const PropertyId& lhs, const PropertyId& rhs );
  };
  inline bool operator==( const PropertyId& lhs, const PropertyId& rhs ) { return lhs.m_chunks == rhs.m_chunks; }
} // namespace Gaudi::Details

template <>
struct std::hash<Gaudi::Details::PropertyId> {
  std::size_t operator()( Gaudi::Details::PropertyId const& s ) const noexcept { return s.hash(); }
};
