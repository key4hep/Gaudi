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
#ifndef _GAUDI_PLUGIN_SERVICE_DETAILS_V1_H_
#define _GAUDI_PLUGIN_SERVICE_DETAILS_V1_H_

/// @author Marco Clemencic <marco.clemencic@cern.ch>

#include <Gaudi/Details/PluginServiceCommon.h>

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>

#include <mutex>

namespace Gaudi {
  namespace PluginService {
    // cppcheck-suppress unknownMacro
    GAUDI_PLUGIN_SERVICE_V1_INLINE namespace v1 {
      namespace Details {
        /// Class providing default factory functions.
        ///
        /// The template argument T is the class to be created, while the methods
        /// template argument S is the specific factory signature.
        template <class T>
        class Factory {
        public:
          template <typename S, typename... Args>
          static typename S::ReturnType create( Args&&... args ) {
            return new T( std::forward<Args>( args )... );
          }
        };

        /// Function used to load a specific factory function.
        /// @return the pointer to the factory function.
        GAUDIPS_API
        void* getCreator( const std::string& id, const std::string& type );

        /// Convoluted implementation of getCreator with an embedded
        /// reinterpret_cast, used to avoid the warning
        /// <pre>
        /// warning: ISO C++ forbids casting between pointer-to-function and pointer-to-object
        /// </pre>
        /// It is an ugly trick but works.<br/>
        /// See:
        /// <ul>
        ///  <li>http://www.trilithium.com/johan/2004/12/problem-with-dlsym/</li>
        ///  <li>http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#573</li>
        ///  <li>http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#195</li>
        /// </ul>
        template <typename F>
        inline F getCreator( const std::string& id ) {
          union {
            void* src;
            F     dst;
          } p2p;
          p2p.src = getCreator( id, typeid( F ).name() );
          return p2p.dst;
        }

        /// Return a canonical name for type_info object (implementation borrowed
        ///  from GaudiKernel/System).
        GAUDIPS_API
        std::string demangle( const std::type_info& id );

        /// Return a canonical name for the template argument.
        template <typename T>
        inline std::string demangle() {
          return demangle( typeid( T ) );
        }

        /// In-memory database of the loaded factories.
        class GAUDIPS_API Registry {
        public:
          typedef std::string KeyType;

          /// Type used for the properties implementation.
          typedef std::map<KeyType, std::string> Properties;

          struct FactoryInfo {
            FactoryInfo( std::string lib, void* p = nullptr, std::string t = "", std::string rt = "",
                         std::string cn = "", Properties props = Properties() )
                : library( std::move( lib ) )
                , ptr( p )
                , type( std::move( t ) )
                , rtype( std::move( rt ) )
                , className( std::move( cn ) )
                , properties( std::move( props ) ) {}

            std::string library;
            void*       ptr;
            std::string type;
            std::string rtype;
            std::string className;
            Properties  properties;

            FactoryInfo& addProperty( const KeyType& k, std::string v ) {
              properties[k] = std::move( v );
              return *this;
            }
          };

          /// Type used for the database implementation.
          typedef std::map<KeyType, FactoryInfo> FactoryMap;

          /// Retrieve the singleton instance of Registry.
          static Registry& instance();

          /// Add a factory to the database.
          template <typename F, typename T, typename I>
          inline FactoryInfo& add( const I& id, typename F::FuncType ptr ) {
            union {
              typename F::FuncType src;
              void*                dst;
            } p2p;
            p2p.src = ptr;
            std::ostringstream o;
            o << id;
            return add( o.str(), p2p.dst, typeid( typename F::FuncType ).name(),
                        typeid( typename F::ReturnType ).name(), demangle<T>() );
          }

          /// Retrieve the factory for the given id.
          void* get( const std::string& id, const std::string& type ) const;

          /// Retrieve the FactoryInfo object for an id.
          const FactoryInfo& getInfo( const std::string& id ) const;

          /// Add a property to an already existing FactoryInfo object (via its id.)
          Registry& addProperty( const std::string& id, const std::string& k, const std::string& v );

          /// Return a list of all the known and loaded factories
          std::set<KeyType> loadedFactoryNames() const;

          /// Return the known factories (loading the list if not yet done).
          inline const FactoryMap& factories() const {
            if ( !m_initialized ) const_cast<Registry*>( this )->initialize();
            return m_factories;
          }

        private:
          /// Private constructor for the singleton pattern.
          /// At construction time, the internal database of known factories is
          /// filled with the name of the libraries containing them, using the
          /// ".components" files in the LD_LIBRARY_PATH.
          Registry();

          /// Private copy constructor for the singleton pattern.
          Registry( const Registry& ) : m_initialized( false ) {}

          /// Add a factory to the database.
          FactoryInfo& add( const std::string& id, void* factory, const std::string& type, const std::string& rtype,
                            const std::string& className, const Properties& props = Properties() );

          /// Return the known factories (loading the list if not yet done).
          inline FactoryMap& factories() {
            if ( !m_initialized ) initialize();
            return m_factories;
          }

          /// Initialize the registry loading the list of factories from the
          /// .component files in the library search path.
          void initialize();

          /// Flag recording if the registry has been initialized or not.
          bool m_initialized;

          /// Internal storage for factories.
          FactoryMap m_factories;

          /// Mutex used to control concurrent access to the internal data.
          mutable std::recursive_mutex m_mutex;
        };

        /// Simple logging class, just to provide a default implementation.
        class GAUDIPS_API Logger {
        public:
          enum Level { Debug = 0, Info = 1, Warning = 2, Error = 3 };
          Logger( Level level = Warning ) : m_level( level ) {}
          virtual ~Logger() {}
          inline Level level() const { return m_level; }
          inline void  setLevel( Level level ) { m_level = level; }
          inline void  info( const std::string& msg ) { report( Info, msg ); }
          inline void  debug( const std::string& msg ) { report( Debug, msg ); }
          inline void  warning( const std::string& msg ) { report( Warning, msg ); }
          inline void  error( const std::string& msg ) { report( Error, msg ); }

        private:
          virtual void report( Level lvl, const std::string& msg );
          Level        m_level;
        };

        /// Return the current logger instance.
        GAUDIPS_API Logger& logger();
        /// Set the logger instance to use.
        /// It must be a new instance and the ownership is passed to the function.
        GAUDIPS_API void setLogger( Logger* logger );
      } // namespace Details

      /// Backward compatibility with Reflex.
      GAUDIPS_API void SetDebug( int debugLevel );
      /// Backward compatibility with Reflex.
      GAUDIPS_API int Debug();
    }
  } // namespace PluginService
} // namespace Gaudi

#define _PS_V1_INTERNAL_FACTORY_REGISTER_CNAME( name, serial ) _register_##_##serial

#define _PS_V1_INTERNAL_DECLARE_FACTORY_WITH_CREATOR( type, typecreator, id, factory, serial )                         \
  namespace {                                                                                                          \
    class _PS_V1_INTERNAL_FACTORY_REGISTER_CNAME( type, serial ) {                                                     \
    public:                                                                                                            \
      typedef factory      s_t;                                                                                        \
      typedef typecreator  f_t;                                                                                        \
      static s_t::FuncType creator() { return &f_t::create<s_t>; }                                                     \
      _PS_V1_INTERNAL_FACTORY_REGISTER_CNAME( type, serial )() {                                                       \
        using ::Gaudi::PluginService::v1::Details::Registry;                                                           \
        Registry::instance().add<s_t, type>( id, creator() );                                                          \
      }                                                                                                                \
    } _PS_V1_INTERNAL_FACTORY_REGISTER_CNAME( s_##type, serial );                                                      \
  }

#define _PS_V1_INTERNAL_DECLARE_FACTORY( type, id, factory, serial )                                                   \
  _PS_V1_INTERNAL_DECLARE_FACTORY_WITH_CREATOR( type, ::Gaudi::PluginService::v1::Details::Factory<type>, id, factory, \
                                                serial )

#endif //_GAUDI_PLUGIN_SERVICE_DETAILS_H_
