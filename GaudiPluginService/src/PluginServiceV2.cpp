/***********************************************************************************\
* (c) Copyright 2013-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

/// @author Marco Clemencic <marco.clemencic@cern.ch>

#define GAUDI_PLUGIN_SERVICE_V2
#include <Gaudi/PluginService.h>

#include <dirent.h>
#include <dlfcn.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string_view>

#include <cxxabi.h>
#include <sys/stat.h>

#ifdef _GNU_SOURCE
#  include <cstring>
#  include <dlfcn.h>
#endif

#ifdef USE_BOOST_FILESYSTEM
#  include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#  include <filesystem>
namespace fs = std::filesystem;
#endif // USE_BOOST_FILESYSTEM

namespace {
  std::mutex registrySingletonMutex;
}

#include <algorithm>

namespace {
  struct OldStyleCnv {
    std::string name;
    void        operator()( const char c ) {
      switch ( c ) {
      case '<':
      case '>':
      case ',':
      case '(':
      case ')':
      case ':':
      case '.':
        name.push_back( '_' );
        break;
      case '&':
        name.push_back( 'r' );
        break;
      case '*':
        name.push_back( 'p' );
        break;
      case ' ':
        break;
      default:
        name.push_back( c );
        break;
      }
    }
  };
  /// Convert a class name in the string used with the Reflex plugin service
  std::string old_style_name( const std::string& name ) {
    return std::for_each( name.begin(), name.end(), OldStyleCnv() ).name;
  }
} // namespace

namespace Gaudi {
  namespace PluginService {
    GAUDI_PLUGIN_SERVICE_V2_INLINE namespace v2 {
      namespace Details {
        std::string demangle( const std::string& id ) {
          int  status;
          auto realname = std::unique_ptr<char, decltype( free )*>(
              abi::__cxa_demangle( id.c_str(), nullptr, nullptr, &status ), free );
          if ( !realname ) return id;
          return std::regex_replace(
              realname.get(),
              std::regex{ "std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >( (?=>))?" },
              "std::string" );
        }
        std::string demangle( const std::type_info& id ) { return demangle( id.name() ); }

        bool Registry::tryDLOpen( const std::string_view& libName ) const {
          const void* handle = dlopen( libName.data(), RTLD_LAZY | RTLD_GLOBAL );
          if ( !handle ) {
            std::cout << "dlopen failed for " << libName << std::endl;
            logger().warning( "cannot load " + std::string( libName ) );
            if ( char* dlmsg = dlerror() ) { logger().warning( dlmsg ); }
            return false;
          }
          return true;
        }

        Registry& Registry::instance() {
          auto            _guard = std::scoped_lock{ ::registrySingletonMutex };
          static Registry r;
          return r;
        }

        void reportBadAnyCast( const std::type_info& factory_type, const std::string& id ) {
          if ( logger().level() <= Logger::Debug ) {
            std::stringstream msg;
            const auto&       info = Registry::instance().getInfo( id );
            msg << "bad any_cast: requested factory " << id << " of type " << demangle( factory_type ) << ", got ";
            if ( info.is_set() )
              msg << demangle( info.factory.type() ) << " from " << info.library;
            else
              msg << "nothing";
            logger().debug( msg.str() );
          }
        }

        Registry::Properties::mapped_type Registry::FactoryInfo::getprop( const Properties::key_type& name ) const {
          auto p = properties.find( name );
          return ( p != end( properties ) ) ? p->second : Properties::mapped_type{};
        }

        Registry::Registry() {}

        void Registry::initialize() {
          auto _guard = std::scoped_lock{ m_mutex };
#if defined( __APPLE__ )
          const auto envVars = { "GAUDI_PLUGIN_PATH" };
          const char sep     = ':';
#else
          const auto envVars = { "GAUDI_PLUGIN_PATH", "LD_LIBRARY_PATH" };
          const char sep     = ':';
#endif

          std::regex line_format{ "^(?:[[:space:]]*(?:(v[0-9]+)::)?([^:]+):(.*[^[:space:]]))?[[:space:]]*(?:#.*)?$" };
          for ( const auto& envVar : envVars ) {
            std::smatch       m;
            std::stringstream search_path;
            if ( auto ptr = std::getenv( envVar ) ) search_path << ptr;
            logger().debug( std::string( "searching factories in " ) + envVar );

            // std::string_view::size_type start_pos = 0, end_pos = 0;
            std::string dir;
            while ( std::getline( search_path, dir, sep ) ) {
              // correctly handle begin of string or path separator
              logger().debug( " looking into " + dir );
              // look for files called "*.components" in the directory
              if ( !fs::is_directory( dir ) ) { continue; }
              for ( const auto& p : fs::directory_iterator( dir ) ) {
                if ( p.path().extension() != ".components" || !is_regular_file( p.path() ) ) { continue; }
                // read the file
                const auto& fullPath = p.path().string();
                logger().debug( "  reading " + p.path().filename().string() );
                std::ifstream factories{ fullPath };
                std::string   line;
                int           factoriesCount = 0;
                int           lineCount      = 0;
                while ( !factories.eof() ) {
                  ++lineCount;
                  std::getline( factories, line );
                  if ( regex_match( line, m, line_format ) ) {
                    if ( m[1] != "v2" ) { continue; } // ignore non "v2" and "empty" lines
                    const std::string lib{ m[2] };
                    const std::string fact{ m[3] };
                    m_factories.emplace( fact, FactoryInfo{ lib, {}, { { "ClassName", fact } } } );
#ifdef GAUDI_REFLEX_COMPONENT_ALIASES
                    // add an alias for the factory using the Reflex convention
                    std::string old_name = old_style_name( fact );
                    if ( fact != old_name ) {
                      m_factories.emplace(
                          old_name, FactoryInfo{ lib, {}, { { "ReflexName", "true" }, { "ClassName", fact } } } );
                    }
#endif
                    ++factoriesCount;
                  } else {
                    logger().warning( "failed to parse line " + fullPath + ':' + std::to_string( lineCount ) );
                  }
                }
                if ( logger().level() <= Logger::Debug ) {
                  logger().debug( "  found " + std::to_string( factoriesCount ) + " factories" );
                }
              }
            }
          }
        }

        const Registry::FactoryMap& Registry::factories() const {
          std::call_once( m_initialized, &Registry::initialize, const_cast<Registry*>( this ) );
          return m_factories;
        }

        Registry::FactoryMap& Registry::factories() {
          std::call_once( m_initialized, &Registry::initialize, this );
          return m_factories;
        }

        Registry::FactoryInfo& Registry::add( const KeyType& id, FactoryInfo info ) {
          auto        _guard = std::scoped_lock{ m_mutex };
          FactoryMap& facts  = factories();

#ifdef GAUDI_REFLEX_COMPONENT_ALIASES
          // add an alias for the factory using the Reflex convention
          const auto old_name = old_style_name( id );
          if ( id != old_name ) {
            auto new_info = info;

            new_info.properties["ReflexName"] = "true";

            add( old_name, new_info );
          }
#endif

          auto entry = facts.find( id );
          if ( entry == facts.end() ) {
            // this factory was not known yet
            entry = facts.emplace( id, std::move( info ) ).first;
          } else {
            // do not replace an existing factory with a new one
            if ( !entry->second.is_set() ) entry->second = std::move( info );
          }
          return entry->second;
        }

        Registry::FactoryMap::size_type Registry::erase( const KeyType& id ) {
          auto        _guard = std::scoped_lock{ m_mutex };
          FactoryMap& facts  = factories();
          return facts.erase( id );
        }

        const Registry::FactoryInfo& Registry::getInfo( const KeyType& id, const bool load ) const {
          auto                     _guard  = std::scoped_lock{ m_mutex };
          static const FactoryInfo unknown = { "unknown" };
          const FactoryMap&        facts   = factories();
          auto                     f       = facts.find( id );

          if ( f == facts.end() ) { return unknown; }
          if ( !load || f->second.is_set() ) { return f->second; }
          const std::string_view library = f->second.library;

          // dlopen can not look into GAUDI_PLUGIN_PATH so a search is reimplemented here
          // and if not found then we fall back to dlopen without full paths
          // that will look in LD_LIBRARY_PATH
          std::stringstream ss;
          if ( auto ptr = std::getenv( "GAUDI_PLUGIN_PATH" ) ) ss << ptr;
          std::string dir;
          bool        found = false;
          while ( std::getline( ss, dir, ':' ) && !found ) {
            if ( !fs::exists( dir ) ) { continue; }
            if ( is_regular_file( dir / fs::path( library ) ) ) {
              // logger().debug( "found " + dirName.string() + "/" + library.c_str() + " for factory " + id );
              if ( !tryDLOpen( ( dir / fs::path( library ) ).string() ) ) {
                return unknown;
              } else {
                found = true;
                break;
              }
            }
          }
          if ( !found && !tryDLOpen( library ) ) return unknown;
          f = facts.find( id ); // ensure that the iterator is valid
          return f->second;
        }

        Registry& Registry::addProperty( const KeyType& id, const KeyType& k, const std::string& v ) {
          auto        _guard = std::scoped_lock{ m_mutex };
          FactoryMap& facts  = factories();
          auto        f      = facts.find( id );

          if ( f != facts.end() ) f->second.properties[k] = v;
          return *this;
        }

        void Registry::setError( const KeyType& warning ) { m_werror.insert( warning ); }

        void Registry::unsetError( const KeyType& warning ) { m_werror.erase( warning ); }

        std::set<Registry::KeyType> Registry::loadedFactoryNames() const {
          auto              _guard = std::scoped_lock{ m_mutex };
          std::set<KeyType> l;
          for ( const auto& f : factories() ) {
            if ( f.second.is_set() ) l.insert( f.first );
          }
          return l;
        }

        void Logger::report( Level lvl, const std::string& msg ) {
          static const char* levels[] = { "DEBUG  : ", "INFO   : ", "WARNING: ", "ERROR  : " };
          if ( lvl >= level() ) { std::cerr << levels[lvl] << msg << std::endl; }
        }

        static auto s_logger = std::make_unique<Logger>();
        Logger&     logger() { return *s_logger; }
        void        setLogger( Logger* logger ) { s_logger.reset( logger ); }

        // This chunk of code was taken from GaudiKernel (genconf) DsoUtils.h
        std::string getDSONameFor( void* fptr ) {
#if defined _GNU_SOURCE || defined __APPLE__
          Dl_info info;
          if ( dladdr( fptr, &info ) == 0 ) return "";

          auto pos = std::strrchr( info.dli_fname, '/' );
          if ( pos )
            ++pos;
          else
            return info.dli_fname;
          return pos;
#else
          return "";
#endif
        }
      } // namespace Details

      void SetDebug( int debugLevel ) {
        using namespace Details;
        Logger& l = logger();
        if ( debugLevel > 1 )
          l.setLevel( Logger::Debug );
        else if ( debugLevel > 0 )
          l.setLevel( Logger::Info );
        else
          l.setLevel( Logger::Warning );
      }

      int Debug() {
        using namespace Details;
        switch ( logger().level() ) {
        case Logger::Debug:
          return 2;
        case Logger::Info:
          return 1;
        default:
          return 0;
        }
      }
    }
  } // namespace PluginService
} // namespace Gaudi
