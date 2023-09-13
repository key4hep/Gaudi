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

#pragma once

// Class: ProcStats
// Description:  Keeps statistics on memory usage
// Author: Jim Kowalkowski (FNAL), modified by M. Shapiro (LBNL)

#include <mutex>
#include <string>
#include <vector>

#if defined( __linux__ ) or defined( __APPLE__ )
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif // __linux__ or __APPLE__

struct procInfo {
  procInfo() = default;
  procInfo( double sz, double rss_sz ) : vsize( sz ), rss( rss_sz ) {}

  bool operator==( const procInfo& p ) const {
#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
#  pragma warning( push )
#  pragma warning( disable : 1572 )
#endif

    return ( vsize == p.vsize && rss == p.rss );

#ifdef __ICC
// re-enable icc remark #1572
#  pragma warning( pop )
#endif
  }

  // see proc(4) man pages for units and a description
  double vsize{ 0 }; // in MB (used to be in pages?)
  double rss{ 0 };   // in MB (used to be in pages?)
};

class ProcStats {

public:
  ProcStats() { open_ufd(); }

private:
  void open_ufd();

public:
  static ProcStats* instance();
  bool              fetch( procInfo& fill_me );
  auto              pageSize() const noexcept { return m_pg_size; }

private:
  class unique_fd {

  private:
    int m_fd{ -1 };

  private:
    unique_fd( const unique_fd& )            = delete;
    unique_fd& operator=( const unique_fd& ) = delete;

  public:
    unique_fd( const int fd = -1 ) : m_fd( fd ) {}
    unique_fd( unique_fd&& other ) {
      m_fd       = other.m_fd;
      other.m_fd = -1;
    }
    ~unique_fd() { close(); }

  public:
    explicit operator bool() const { return m_fd != -1; }
    template <typename... Args>
    unique_fd& open( Args&&... args ) {
      m_fd = ::open( std::forward<Args>( args )... );
      return *this;
    }
    int close() {
      int r = 0;
      if ( m_fd != -1 ) {
        r    = ::close( m_fd );
        m_fd = -1;
      }
      return r;
    }

  public:
#define unique_fd_forward( fun )                                                                                       \
  template <typename... Args>                                                                                          \
  auto fun( Args&&... args ) const {                                                                                   \
    return ::fun( m_fd, std::forward<Args>( args )... );                                                               \
  }
    // clang-format off
    unique_fd_forward( lseek )
    unique_fd_forward( read )
    unique_fd_forward( write )
    unique_fd_forward( fcntl )
    unique_fd_forward( fsync )
    unique_fd_forward( fchown )
    unique_fd_forward( stat )
    // clang-format on
#undef unique_fd_forward
  };

private:
  unique_fd  m_ufd;
  double     m_pg_size{ 0 };
  procInfo   m_curr;
  bool       m_valid{ false };
  std::mutex m_mutex;
};
