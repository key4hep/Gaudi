#ifndef GAUDIAUD_PROCSTATS_H
#define GAUDIAUD_PROCSTATS_H

// Class: ProcStats
// Description:  Keeps statistics on memory usage
// Author: Jim Kowalkowski (FNAL), modified by M. Shapiro (LBNL)

#include <string>
#include <vector>
#if defined( __linux__ ) or defined( __APPLE__ )
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif // __linux__ or __APPLE__

struct procInfo {
  procInfo() : vsize( 0 ), rss( 0 ) {}
  procInfo( double sz, double rss_sz ) : vsize( sz ), rss( rss_sz ) {}

  bool operator==( const procInfo& p ) const {
#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
#  pragma warning( push )
#  pragma warning( disable : 1572 )
#endif

    return vsize == p.vsize && rss == p.rss;

#ifdef __ICC
// re-enable icc remark #1572
#  pragma warning( pop )
#endif
  }

  // see proc(4) man pages for units and a description
  double vsize; // in MB (used to be in pages?)
  double rss;   // in MB (used to be in pages?)
};

class ProcStats {
public:
  static ProcStats* instance();

  bool   fetch( procInfo& fill_me );
  double pageSize() const { return pg_size; }

private:
  ProcStats();

  struct cleanup {
    cleanup() {}
    ~cleanup();
  };

  friend struct cleanup;

  class unique_fd {
    int m_fd;
    unique_fd( const unique_fd& ) = delete;
    unique_fd& operator=( const unique_fd& ) = delete;

  public:
    unique_fd( int fd = -1 ) : m_fd( fd ) {}
    unique_fd( unique_fd&& other ) {
      m_fd       = other.m_fd;
      other.m_fd = -1;
    }
    ~unique_fd() {
      if ( m_fd != -1 ) ::close( m_fd );
    }

    explicit operator bool() const { return m_fd != -1; }
    template <typename... Args>
    unique_fd& open( Args&&... args ) {
      m_fd = ::open( std::forward<Args>( args )... );
      return *this;
    }
#define unique_fd_forward( fun )                                                                                       \
  template <typename... Args>                                                                                          \
  auto fun( Args&&... args ) const->decltype( ::fun( m_fd, std::forward<Args>( args )... ) ) {                         \
    return ::fun( m_fd, std::forward<Args>( args )... );                                                               \
  }
    unique_fd_forward( lseek ) unique_fd_forward( read ) unique_fd_forward( write ) unique_fd_forward( fcntl )
        unique_fd_forward( fsync ) unique_fd_forward( fchown ) unique_fd_forward( stat )
#undef unique_fd_forward
            int close() {
      auto r = ::close( m_fd );
      m_fd   = -1;
      return r;
    }
  };

  unique_fd   fd;
  double      pg_size;
  procInfo    curr;
  std::string fname;
  char        buf[500];
  bool        valid;

  static ProcStats* inst;
};

#endif
