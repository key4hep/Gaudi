/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifdef _WIN32
// Disable a warning in Boost program_options headers:
// inconsistent linkage in program_options/variables_map.hpp
#  pragma warning( disable : 4273 )

// Avoid conflicts between windows and the message service.
#  define NOMSG
#  define NOGDI
#endif

#ifdef __ICC
// disable icc warning #279: controlling expression is constant
// ... a lot of noise produced by the boost/filesystem/operations.hpp
#  pragma warning( disable : 279 )
#endif

#include <Gaudi/Algorithm.h>
#include <Gaudi/Auditor.h>
#include <Gaudi/IAuditor.h>
#include <Gaudi/PluginService.h>
#include <GaudiKernel/AlgTool.h>
#include <GaudiKernel/Bootstrap.h>
#include <GaudiKernel/DataHandle.h>
#include <GaudiKernel/DataHandleProperty.h>
#include <GaudiKernel/GaudiHandle.h>
#include <GaudiKernel/HashMap.h>
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/System.h>
#include <GaudiKernel/Time.h>
#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include <exception>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <type_traits>
#include <vector>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

#define LOG_ERROR BOOST_LOG_TRIVIAL( error )
#define LOG_WARNING BOOST_LOG_TRIVIAL( warning )
#define LOG_INFO BOOST_LOG_TRIVIAL( info )

using namespace std;
using Gaudi::Details::PropertyBase;

class IConverter;

// useful typedefs
typedef std::vector<std::string> Strings_t;
typedef std::vector<fs::path>    LibPathNames_t;

namespace {

  std::string quote( std::string_view sv ) {
    std::ostringstream s;
    s << std::quoted( sv, '\'' );
    return s.str();
  }

  const std::string py_tab = "    ";

  /// Regular expression to validate the property names.
  /// @see http://docs.python.org/reference/lexical_analysis.html#identifiers
  const boost::regex pythonIdentifier( "^[a-zA-Z_][a-zA-Z0-9_]*$" );

  //-----------------------------------------------------------------------------
  enum class component_t {
    Module,
    DefaultName,
    Algorithm,
    AlgTool,
    Auditor,
    Service,
    ApplicationMgr,
    IInterface,
    Converter,
    DataObject,
    Unknown
  };

  enum class conf_t {
    CONF, // legacy configurables
    CONF2 // GaudiConfig2 configurables
  };

  const std::map<std::string, component_t> allowedFactories{
      { typeid( Gaudi::Algorithm::Factory::FactoryType ).name(), component_t::Algorithm },
      { typeid( Service::Factory::FactoryType ).name(), component_t::Service },
      { typeid( AlgTool::Factory::FactoryType ).name(), component_t::AlgTool },
      // factories for new Auditors
      { typeid( Gaudi::Auditor::Factory::FactoryType ).name(), component_t::Auditor },
  };

  const std::string& toString( component_t type ) {
    static const std::array<std::string, 11> names = { "Module",    "DefaultName", "Algorithm",      "AlgTool",
                                                       "Auditor",   "Service",     "ApplicationMgr", "IInterface",
                                                       "Converter", "DataObject",  "Unknown" };
    return names.at( static_cast<std::underlying_type_t<component_t>>( type ) );
  }
  std::ostream& operator<<( std::ostream& os, component_t type ) { return os << toString( type ); }

  std::set<std::string> ignored_interfaces{
      { "IInterface", "IProperty", "INamedInterface", "IAlgorithm", "IAlgTool", "IService", "IAuditor" } };

  //-----------------------------------------------------------------------------
  /// Translate a valid C++ typename into a valid python one
  std::string pythonizeName( const std::string& name ) {
    static const string in( "<>&*,: ()." );
    static const string out( "__rp__s___" );
    auto                r = boost::algorithm::replace_all_copy( name, ", ", "," );
    for ( auto& c : r ) {
      auto rep = in.find( c );
      if ( rep != string::npos ) c = out[rep];
    }
    return r;
  }
  //-----------------------------------------------------------------------------
  template <typename T>
  std::type_index typeIndex() {
    return std::type_index{ typeid( T ) };
  }
  //-----------------------------------------------------------------------------
  inline std::string libNativeName( const std::string& libName ) {
#if defined( _WIN32 )
    return libName + ".dll";
#elif defined( __linux ) || defined( __APPLE__ )
    return "lib" + libName + ".so";
#else
    // variant of the GIGO design pattern
    return libName;
#endif
  }
} // namespace

class configGenerator {
  /// name of the package we are processing
  string m_pkgName;

  /// absolute path to the directory where genconf will store auto-generated
  /// files (Configurables and ConfigurableDb)
  string m_outputDirName;

  /// buffer of auto-generated configurables
  stringstream m_pyBuf;

  /// switch to decide if the generated configurables need
  /// to import GaudiHandles (ie: if one of the components has a XyzHandle<T>)
  bool m_importGaudiHandles = false;
  bool m_importDataHandles  = false;

  /// Types of configurables to generate
  std::set<conf_t> m_confTypes;

  /// buffer of generated configurables informations for the "Db" file
  /// The "Db" file is holding informations about the generated configurables
  /// This file is later one used by the PropertyProxy.py to locate
  /// Configurables and know their default values, host module,...
  stringstream m_dbBuf;

  /// buffer of generated GaudiConfig2 configurables
  stringstream m_db2Buf;

  /// Configurable customization. Contains customization for:
  ///  - Name of the module where configurable base classes are defined
  ///  - Name of the configurable base class for the Algorithm component
  ///  - Name of the configurable base class for the AlgTool component
  ///  - Name of the configurable base class for the Service component
  std::map<component_t, std::string> m_configurable;

public:
  configGenerator( const string& pkgName, const string& outputDirName )
      : m_pkgName( pkgName ), m_outputDirName( outputDirName ) {}

  /// main entry point of this class:
  ///  - iterate over all the modules (ie: library names)
  ///  - for each module extract component informations
  ///  - eventually generate the header/body/trailer python file and "Db" file
  int genConfig( const Strings_t& modules, const string& userModule );

  /// customize configurable types to generate
  void setConfigurableTypes( const std::set<conf_t>& types ) { m_confTypes = types; }

  /// customize the Module name where configurable base classes are defined
  void setConfigurableModule( const std::string& moduleName ) { m_configurable[component_t::Module] = moduleName; }

  /// customize the default name for configurable instances
  void setConfigurableDefaultName( const std::string& defaultName ) {
    m_configurable[component_t::DefaultName] = defaultName;
  }

  /// customize the configurable base class for Algorithm component
  void setConfigurableAlgorithm( const std::string& cfgAlgorithm ) {
    m_configurable[component_t::Algorithm] = cfgAlgorithm;
  }

  /// customize the configurable base class for AlgTool component
  void setConfigurableAlgTool( const std::string& cfgAlgTool ) { m_configurable[component_t::AlgTool] = cfgAlgTool; }

  /// customize the configurable base class for AlgTool component
  void setConfigurableAuditor( const std::string& cfgAuditor ) { m_configurable[component_t::Auditor] = cfgAuditor; }

  /// customize the configurable base class for Service component
  void setConfigurableService( const std::string& cfgService ) {
    m_configurable[component_t::Service]        = cfgService;
    m_configurable[component_t::ApplicationMgr] = cfgService;
  }

private:
  bool genComponent( const std::string& libName, const std::string& componentName, component_t componentType,
                     const vector<PropertyBase*>&                                properties,
                     const Gaudi::PluginService::Details::Registry::FactoryInfo& info );

  bool genComponent2( const std::string& componentName, component_t componentType,
                      const vector<PropertyBase*>& properties, const std::vector<std::string>& interfaces,
                      const Gaudi::PluginService::Details::Registry::FactoryInfo& info );

  void genImport( std::ostream& s, std::string_view frmt, std::string indent );
  void genHeader( std::ostream& pyOut, std::ostream& dbOut );
  void genBody( std::ostream& pyOut, std::ostream& dbOut ) {
    pyOut << m_pyBuf.str() << flush;
    dbOut << m_dbBuf.str() << flush;
  }
  void genTrailer( std::ostream& pyOut, std::ostream& dbOut );

  /// handle the "marshalling" of Properties
  void pythonizeValue( const PropertyBase* prop, string& pvalue, string& ptype );
};

int createAppMgr();

void init_logging( boost::log::trivial::severity_level level ) {
  namespace logging  = boost::log;
  namespace keywords = boost::log::keywords;
  namespace expr     = boost::log::expressions;

  logging::add_console_log( std::cout, keywords::format =
                                           ( expr::stream << "[" << std::setw( 7 ) << std::left
                                                          << logging::trivial::severity << "] " << expr::smessage ) );

  logging::core::get()->set_filter( logging::trivial::severity >= level );
}

//--- Command main program-----------------------------------------------------
int main( int argc, char** argv )
//-----------------------------------------------------------------------------
{
  init_logging( ( System::isEnvSet( "VERBOSE" ) && !System::getEnv( "VERBOSE" ).empty() )
                    ? boost::log::trivial::info
                    : boost::log::trivial::warning );

  fs::path    pwd = fs::initial_path();
  fs::path    out;
  Strings_t   libs;
  std::string pkgName;
  std::string userModule;

  // declare a group of options that will be allowed only on command line
  po::options_description generic( "Generic options" );
  generic.add_options()( "help,h", "produce this help message" )(
      "package-name,p", po::value<string>(), "name of the package for which we create the configurables file" )(
      "input-libraries,i", po::value<string>(), "libraries to extract the component configurables from" )(
      "input-cfg,c", po::value<string>(),
      "path to the cfg file holding the description of the Configurable base "
      "classes, the python module holding the Configurable definitions, etc..." )(
      "output-dir,o", po::value<string>()->default_value( "../genConfDir" ),
      "output directory for genconf files." )( "debug-level,d", po::value<int>()->default_value( 0 ), "debug level" )(
      "load-library,l", po::value<Strings_t>()->composing(), "preloading library" )(
      "user-module,m", po::value<string>(), "user-defined module to be imported by the genConf-generated one" )(
      "no-init", "do not generate the (empty) __init__.py" )(
      "type", po::value<string>()->default_value( "conf,conf2" ), "comma-separate types of configurables to generate" );

  // declare a group of options that will be allowed both on command line
  // _and_ in configuration file
  po::options_description config( "Configuration" );
  config.add_options()( "configurable-module", po::value<string>()->default_value( "AthenaCommon" ),
                        "Name of the module holding the configurable classes" )(
      "configurable-default-name", po::value<string>()->default_value( "Configurable.DefaultName" ),
      "Default name for the configurable instance" )( "configurable-algorithm",
                                                      po::value<string>()->default_value( "ConfigurableAlgorithm" ),
                                                      "Name of the configurable base class for Algorithm components" )(
      "configurable-algtool", po::value<string>()->default_value( "ConfigurableAlgTool" ),
      "Name of the configurable base class for AlgTool components" )(
      "configurable-auditor", po::value<string>()->default_value( "ConfigurableAuditor" ),
      "Name of the configurable base class for Auditor components" )(
      "configurable-service", po::value<string>()->default_value( "ConfigurableService" ),
      "Name of the configurable base class for Service components" );

  po::options_description cmdline_options;
  cmdline_options.add( generic ).add( config );

  po::options_description config_file_options;
  config_file_options.add( config );

  po::options_description visible( "Allowed options" );
  visible.add( generic ).add( config );

  po::variables_map vm;

  try {
    po::store( po::command_line_parser( argc, argv ).options( cmdline_options ).run(), vm );

    po::notify( vm );

    // try to read configuration from the optionally given configuration file
    if ( vm.contains( "input-cfg" ) ) {
      string cfgFileName = vm["input-cfg"].as<string>();
      cfgFileName        = fs::system_complete( fs::path( cfgFileName ) ).string();
      std::ifstream ifs( cfgFileName );
      po::store( parse_config_file( ifs, config_file_options ), vm );
    }

    po::notify( vm );
  } catch ( po::error& err ) {
    LOG_ERROR << "error detected while parsing command options: " << err.what();
    return EXIT_FAILURE;
  }

  //--- Process command options -----------------------------------------------
  if ( vm.contains( "help" ) ) {
    cout << visible << endl;
    return EXIT_FAILURE;
  }

  if ( vm.contains( "package-name" ) ) {
    pkgName = vm["package-name"].as<string>();
  } else {
    LOG_ERROR << "'package-name' required";
    cout << visible << endl;
    return EXIT_FAILURE;
  }

  if ( vm.contains( "user-module" ) ) {
    userModule = vm["user-module"].as<string>();
    LOG_INFO << "INFO: will import user module " << userModule;
  }

  if ( vm.contains( "input-libraries" ) ) {
    // re-shape the input arguments:
    //  - removes spurious spaces,
    //  - split into tokens.
    std::string tmp = vm["input-libraries"].as<std::string>();
    boost::trim( tmp );
    boost::split( libs, tmp, boost::is_any_of( " " ), boost::token_compress_on );
  } else {
    LOG_ERROR << "input component library(ies) required";
    cout << visible << endl;
    return EXIT_FAILURE;
  }

  if ( vm.contains( "output-dir" ) ) { out = fs::system_complete( fs::path( vm["output-dir"].as<string>() ) ); }

  if ( vm.contains( "debug-level" ) ) { Gaudi::PluginService::SetDebug( vm["debug-level"].as<int>() ); }

  if ( vm.contains( "load-library" ) ) {
    for ( const auto& lLib : vm["load-library"].as<Strings_t>() ) {
      // load done through Gaudi helper class
      System::ImageHandle tmp; // we ignore the library handle
      unsigned long       err = System::loadDynamicLib( lLib, &tmp );
      if ( err != 1 ) LOG_WARNING << "failed to load: " << lLib;
    }
  }

  std::set<conf_t> confTypes;
  if ( vm.contains( "type" ) ) {
    for ( const std::string& type : boost::tokenizer{ vm["type"].as<std::string>(), boost::char_separator{ "," } } ) {
      if ( type == "conf" ) {
        confTypes.insert( conf_t::CONF );
      } else if ( type == "conf2" ) {
        confTypes.insert( conf_t::CONF2 );
      } else {
        LOG_ERROR << "unknown configurable type: " << type;
        cout << visible << endl;
        return EXIT_FAILURE;
      }
    }
  }

  if ( !fs::exists( out ) ) {
    try {
      fs::create_directory( out );
    } catch ( fs::filesystem_error& err ) {
      LOG_ERROR << "error creating directory: " << err.what();
      return EXIT_FAILURE;
    }
  }

  {
    std::ostringstream msg;
    msg << ":::::: libraries : [ ";
    std::copy( libs.begin(), libs.end(), std::ostream_iterator<std::string>( msg, " " ) );
    msg << "] ::::::";
    LOG_INFO << msg.str();
  }

  configGenerator py( pkgName, out.string() );
  py.setConfigurableModule( vm["configurable-module"].as<string>() );
  py.setConfigurableTypes( confTypes );
  py.setConfigurableDefaultName( vm["configurable-default-name"].as<string>() );
  py.setConfigurableAlgorithm( vm["configurable-algorithm"].as<string>() );
  py.setConfigurableAlgTool( vm["configurable-algtool"].as<string>() );
  py.setConfigurableAuditor( vm["configurable-auditor"].as<string>() );
  py.setConfigurableService( vm["configurable-service"].as<string>() );

  int sc = EXIT_FAILURE;
  try {
    sc = py.genConfig( libs, userModule );
  } catch ( exception& e ) {
    cout << "ERROR: Could not generate Configurable(s) !\n"
         << "ERROR: Got exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  if ( EXIT_SUCCESS == sc && !vm.contains( "no-init" ) ) {
    // create an empty __init__.py file in the output dir
    std::fstream initPy( ( out / fs::path( "__init__.py" ) ).string(), std::ios_base::out | std::ios_base::trunc );
    initPy << "## Hook for " << pkgName << " genConf module\n" << flush;
  }

  {
    std::ostringstream msg;
    msg << ":::::: libraries : [ ";
    std::copy( libs.begin(), libs.end(), std::ostream_iterator<std::string>( msg, " " ) );
    msg << "] :::::: [DONE]";
    LOG_INFO << msg.str();
  }
  return sc;
}

//-----------------------------------------------------------------------------
int configGenerator::genConfig( const Strings_t& libs, const string& userModule )
//-----------------------------------------------------------------------------
{
  const auto endLib = libs.end();

  static const std::string gaudiSvc = "GaudiCoreSvc";
  const bool               isGaudiSvc =
      std::find_if( libs.begin(), endLib, []( const auto& s ) {
        return s.find( gaudiSvc ) != std::string::npos; // libs can be <name> or path/to/lib<name>.so
      } ) != endLib;

  //--- Instantiate ApplicationMgr --------------------------------------------
  if ( !isGaudiSvc && createAppMgr() ) {
    cout << "ERROR: ApplicationMgr can not be created. Check environment" << endl;
    return EXIT_FAILURE;
  }

  //--- Iterate over component factories --------------------------------------
  using Gaudi::PluginService::Details::Registry;
  const Registry& registry = Registry::instance();

  auto bkgNames = registry.loadedFactoryNames();

  ISvcLocator* svcLoc   = Gaudi::svcLocator();
  IInterface*  dummySvc = new Service( "DummySvc", svcLoc );
  dummySvc->addRef();

  bool allGood = true;

  // iterate over all the requested libraries
  for ( const auto& iLib : libs ) {
    std::string lib = fs::path( iLib ).stem().string();
    if ( lib.compare( 0, 3, "lib" ) == 0 ) {
      lib = lib.substr( 3 ); // For *NIX remove "lib"
    }
    LOG_INFO << ":::: processing library: " << iLib << "...";

    // reset state
    m_importGaudiHandles = false;
    m_importDataHandles  = false;
    m_pyBuf.str( "" );
    m_dbBuf.str( "" );
    m_db2Buf.str( "" );

    //--- Load component library ----------------------------------------------
    System::ImageHandle handle;
    unsigned long       err = System::loadDynamicLib( iLib, &handle );
    if ( err != 1 ) {
      LOG_ERROR << System::getLastErrorString();
      allGood = false;
      continue;
    }

    const auto& factories = registry.factories();
    for ( const auto& factoryName : registry.loadedFactoryNames() ) {
      if ( bkgNames.find( factoryName ) != bkgNames.end() ) {
        if ( Gaudi::PluginService::Details::logger().level() <= 1 ) {
          LOG_INFO << "\t==> skipping [" << factoryName << "]...";
        }
        continue;
      }
      auto entry = factories.find( factoryName );
      if ( entry == end( factories ) ) {
        LOG_ERROR << "inconsistency in component factories list: I cannot find anymore " << factoryName;
        continue;
      }
      const auto& info = entry->second;
      if ( !info.is_set() ) continue;

      // do not generate configurables for the Reflex-compatible aliases
      if ( !info.getprop( "ReflexName" ).empty() ) continue;

      // Atlas contributed code (patch #1247)
      // Skip the generation of configurables if the component does not come
      // from the same library we are processing (i.e. we found a symbol that
      // is coming from a library loaded by the linker).
      if ( libNativeName( lib ) != info.library ) {
        LOG_WARNING << "library [" << lib << "] exposes factory [" << factoryName << "] which is declared in ["
                    << info.library << "] !!";
        continue;
      }

      component_t type = component_t::Unknown;
      {
        const auto ft = allowedFactories.find( info.factory.type().name() );
        if ( ft != allowedFactories.end() ) {
          type = ft->second;
        } else if ( factoryName == "ApplicationMgr" ) {
          type = component_t::ApplicationMgr;
        } else
          continue;
      }

      // handle possible problems with templated components
      std::string name = boost::trim_copy( factoryName );

      const auto className = info.getprop( "ClassName" );
      LOG_INFO << " - component: " << className << " (" << ( className != name ? ( name + ": " ) : std::string() )
               << type << ")";

      string             cname = "DefaultName";
      SmartIF<IProperty> prop;
      try {
        switch ( type ) {
        case component_t::Algorithm:
          prop = SmartIF<IAlgorithm>( Gaudi::Algorithm::Factory::create( factoryName, cname, svcLoc ).release() );
          break;
        case component_t::Service:
          prop = SmartIF<IService>( Service::Factory::create( factoryName, cname, svcLoc ).release() );
          break;
        case component_t::AlgTool:
          prop =
              SmartIF<IAlgTool>( AlgTool::Factory::create( factoryName, cname, toString( type ), dummySvc ).release() );
          // FIXME: AlgTool base class increase artificially by 1 the refcount.
          prop->release();
          break;
        case component_t::Auditor:
          prop = SmartIF<Gaudi::IAuditor>( Gaudi::Auditor::Factory::create( factoryName, cname, svcLoc ).release() );
          break;
        case component_t::ApplicationMgr:
          prop = SmartIF<ISvcLocator>( svcLoc );
          break;
        default:
          continue; // unknown
        }
      } catch ( exception& e ) {
        LOG_ERROR << "Error instantiating " << name << " from " << iLib;
        LOG_ERROR << "Got exception: " << e.what();
        allGood = false;
        continue;
      } catch ( ... ) {
        LOG_ERROR << "Error instantiating " << name << " from " << iLib;
        allGood = false;
        continue;
      }
      if ( prop ) {
        if ( m_confTypes.contains( conf_t::CONF ) && !genComponent( lib, name, type, prop->getProperties(), info ) ) {
          allGood = false;
        }
        if ( m_confTypes.contains( conf_t::CONF2 ) &&
             !genComponent2( name, type, prop->getProperties(), prop->getInterfaceNames(), info ) ) {
          allGood = false;
        }
        prop.reset();
      } else {
        LOG_ERROR << "could not cast IInterface* object to an IProperty* !";
        LOG_ERROR << "NO Configurable will be generated for [" << name << "] !";
        allGood = false;
      }
    } //> end loop over factories

    ///
    /// write-out files for this library
    ///
    if ( m_confTypes.contains( conf_t::CONF ) ) {
      const std::string pyName = ( fs::path( m_outputDirName ) / fs::path( lib + "Conf.py" ) ).string();
      const std::string dbName = ( fs::path( m_outputDirName ) / fs::path( lib + ".confdb" ) ).string();

      std::fstream py( pyName, std::ios_base::out | std::ios_base::trunc );
      std::fstream db( dbName, std::ios_base::out | std::ios_base::trunc );

      genHeader( py, db );
      if ( !userModule.empty() ) py << "from " << userModule << " import *" << endl;
      genBody( py, db );
      genTrailer( py, db );
    }
    if ( m_confTypes.contains( conf_t::CONF2 ) ) {
      const std::string db2Name = ( fs::path( m_outputDirName ) / fs::path( lib + ".confdb2_part" ) ).string();
      std::fstream      db2( db2Name, std::ios_base::out | std::ios_base::trunc );
      db2 << "{\n" << m_db2Buf.str() << "}\n";
    }

  } //> end loop over libraries

  dummySvc->release();
  dummySvc = 0;

  return allGood ? EXIT_SUCCESS : EXIT_FAILURE;
}

void configGenerator::genImport( std::ostream& s, std::string_view frmt, std::string indent = "" ) {

  std::string::size_type pos = 0, nxtpos = 0;
  std::string            mod;

  while ( std::string::npos != pos ) {
    // find end of module name
    nxtpos = m_configurable[component_t::Module].find_first_of( ',', pos );

    // Prepare import string
    mod = m_configurable[component_t::Module].substr( pos, nxtpos - pos );
    std::ostringstream import;
    import << fmt::format( fmt::runtime( frmt ), mod );

    // append a normal import or a try/except enclosed one depending
    // on availability of a fall-back module (next in the list)
    if ( std::string::npos == nxtpos ) {
      // last possible module
      s << indent << import.str() << "\n" << flush;
      pos = std::string::npos;
    } else {
      // we have a fallback for this
      s << indent << "try:\n" << indent << py_tab << import.str() << "\n" << indent << "except ImportError:\n" << flush;
      pos = nxtpos + 1;
    }
    // increase indentation level for next iteration
    indent += py_tab;
  }
}

//-----------------------------------------------------------------------------
void configGenerator::genHeader( std::ostream& py, std::ostream& db )
//-----------------------------------------------------------------------------
{
  // python file part
  std::string now = Gaudi::Time::current().format( true );
  py << "#" << now //<< "\n"
     << "\"\"\"Automatically generated. DO NOT EDIT please\"\"\"\n";

  if ( m_importGaudiHandles ) { py << "from GaudiKernel.GaudiHandles import *\n"; }

  if ( m_importDataHandles ) { py << "from GaudiKernel.DataHandle import DataHandle\n"; }

  genImport( py, "from {}.Configurable import *" );

  // db file part
  db << "##  -*- ascii -*-  \n"
     << "# db file automatically generated by genconf on: " << now << "\n"
     << flush;
}
//-----------------------------------------------------------------------------
void configGenerator::genTrailer( std::ostream& /*py*/, std::ostream& db )
//-----------------------------------------------------------------------------
{
  // db file part
  db << "## " << m_pkgName << "\n" << std::flush;
}

//-----------------------------------------------------------------------------
bool configGenerator::genComponent( const std::string& libName, const std::string& componentName,
                                    component_t componentType, const vector<PropertyBase*>& properties,
                                    const Gaudi::PluginService::Details::Registry::FactoryInfo& info )
//-----------------------------------------------------------------------------
{
  auto       cname    = pythonizeName( componentName );
  const auto decl_loc = info.getprop( "declaration_location" );

  std::vector<std::pair<std::string, std::string>> propDoc;
  propDoc.reserve( properties.size() );

  m_pyBuf << "\nclass " << cname << "( " << m_configurable[componentType] << " ) :\n";
  m_pyBuf << "  __slots__ = { \n";
  for ( const auto& prop : properties ) {
    const string& pname = prop->name();
    // Validate property name (it must be a valid Python identifier)
    if ( !boost::regex_match( pname, pythonIdentifier ) ) {
      LOG_ERROR << "invalid property name \"" << pname << "\" in component " << cname << " (invalid Python identifier)"
                << std::endl;
      // try to make the buffer at least more or less valid python code.
      m_pyBuf << " #ERROR-invalid identifier '" << pname << "'\n"
              << "  }\n";
      return false;
    }

    string pvalue, ptype;
    pythonizeValue( prop, pvalue, ptype );
    m_pyBuf << "    '" << pname << "' : " << pvalue << ",\n";

    if ( prop->documentation() != "none" ) {
      propDoc.emplace_back( pname, prop->documentation() + " [" + prop->ownerTypeName() + "]" );
    }
  }
  m_pyBuf << "  }\n";
  m_pyBuf << "  _propertyDocDct = { \n";
  for ( const auto& prop : propDoc ) {
    m_pyBuf << std::setw( 5 ) << "'" << prop.first << "' : "
            << "\"\"\" " << prop.second << " \"\"\",\n";
  }
  m_pyBuf << "  }\n";

  if ( !decl_loc.empty() ) { m_pyBuf << "  __declaration_location__ = '" << decl_loc << "'\n"; }
  m_pyBuf << "  def __init__(self, name = " << m_configurable[component_t::DefaultName] << ", **kwargs):\n"
          << "      super(" << cname << ", self).__init__(name)\n"
          << "      for n,v in kwargs.items():\n"
          << "         setattr(self, n, v)\n"
          << "  def getDlls( self ):\n"
          << "      return '" << libName << "'\n"
          << "  def getType( self ):\n"
          << "      return '" << componentName << "'\n"
          << "  pass # class " << cname << "\n"
          << flush;

  // name of the auto-generated module
  const string pyName  = ( fs::path( m_outputDirName ) / fs::path( libName + "Conf.py" ) ).string();
  const string modName = fs::path( pyName ).filename().stem().string();

  // now the db part
  m_dbBuf << m_pkgName << "." << modName << " " << libName << " " << cname << "\n" << flush;

  return true;
}

//-----------------------------------------------------------------------------
bool configGenerator::genComponent2( const std::string& componentName, component_t componentType,
                                     const vector<PropertyBase*>& properties, const vector<std::string>& interfaces,
                                     const Gaudi::PluginService::Details::Registry::FactoryInfo& info )
//-----------------------------------------------------------------------------
{
  m_db2Buf << "    '" << componentName << "': {\n";
  m_db2Buf << "        '__component_type__': '";
  switch ( componentType ) {
  case component_t::Algorithm:
    m_db2Buf << "Algorithm";
    break;
  case component_t::AlgTool:
    m_db2Buf << "AlgTool";
    break;
  case component_t::ApplicationMgr: // FALLTROUGH
  case component_t::Service:
    m_db2Buf << "Service";
    break;
  case component_t::Auditor:
    m_db2Buf << "Auditor";
    break;
  default:
    m_db2Buf << "Unknown";
  }

  const auto decl_loc = info.getprop( "declaration_location" );
  if ( !decl_loc.empty() ) { m_db2Buf << "',\n        '__declaration_location__': '" << decl_loc; }

  m_db2Buf << "',\n        '__interfaces__': (";
  for ( const auto& intf : std::set<std::string>{ begin( interfaces ), end( interfaces ) } ) {
    if ( ignored_interfaces.find( intf ) == end( ignored_interfaces ) ) { m_db2Buf << '\'' << intf << "', "; }
  }
  m_db2Buf << "),\n        'properties': {\n";

  bool success = true;
  for ( const auto& prop : properties ) {
    const string& pname = prop->name();
    // Validate property name (it must be a valid Python identifier)
    if ( !boost::regex_match( pname, pythonIdentifier ) ) {
      LOG_ERROR << "invalid property name \"" << pname << "\" in component " << componentName
                << " (invalid Python identifier)" << std::endl;
      m_db2Buf << " #ERROR-invalid identifier '" << pname << "'\n";
      success = false;
      break;
    }

    string pvalue, ptype;
    pythonizeValue( prop, pvalue, ptype );

    m_db2Buf << "            '" << pname << "': ('" << ptype << "', " << pvalue << ", '''" << prop->documentation()
             << " [" << prop->ownerTypeName() << "]'''";
    auto sem = prop->semantics();
    if ( !sem.empty() ) { m_db2Buf << ", '" << sem << '\''; }
    m_db2Buf << "),\n";
  }

  m_db2Buf << "        },\n    },\n";

  return success;
}

//-----------------------------------------------------------------------------
void configGenerator::pythonizeValue( const PropertyBase* p, string& pvalue, string& ptype )
//-----------------------------------------------------------------------------
{
  const std::string     cvalue = p->toString();
  const std::type_index ti     = std::type_index( *p->type_info() );
  ptype                        = System::typeinfoName( *p->type_info() );

  if ( ti == typeIndex<bool>() ) {
    pvalue = ( cvalue == "0" || cvalue == "False" || cvalue == "false" ) ? "False" : "True";
  } else if ( ti == typeIndex<char>() || ti == typeIndex<signed char>() || ti == typeIndex<unsigned char>() ||
              ti == typeIndex<short>() || ti == typeIndex<unsigned short>() || ti == typeIndex<int>() ||
              ti == typeIndex<unsigned int>() || ti == typeIndex<long>() || ti == typeIndex<unsigned long>() ||
              ti == typeIndex<long long>() || ti == typeIndex<unsigned long long>() ) {
    pvalue = cvalue;
  } else if ( ti == typeIndex<float>() || ti == typeIndex<double>() ) {
    // forces python to handle this as a float: put a dot in there...
    pvalue = boost::to_lower_copy( cvalue );
    if ( std::string::npos != pvalue.find( "nan" ) ) {
      pvalue = "float('nan')";
    } else if ( std::string::npos == pvalue.find( "." ) && std::string::npos == pvalue.find( "e" ) ) {
      pvalue = cvalue + ".0";
    }
  } else if ( ti == typeIndex<string>() ) {
    pvalue = quote( cvalue );
  } else if ( ti == typeIndex<GaudiHandleBase>() ) {
    const GaudiHandleProperty& hdl  = dynamic_cast<const GaudiHandleProperty&>( *p );
    const GaudiHandleBase&     base = hdl.value();

    pvalue               = base.pythonRepr();
    ptype                = base.pythonPropertyClassName();
    m_importGaudiHandles = true;
  } else if ( ti == typeIndex<GaudiHandleArrayBase>() ) {
    const GaudiHandleArrayProperty& hdl  = dynamic_cast<const GaudiHandleArrayProperty&>( *p );
    const GaudiHandleArrayBase&     base = hdl.value();

    pvalue               = base.pythonRepr();
    ptype                = base.pythonPropertyClassName();
    m_importGaudiHandles = true;
  } else if ( auto hdl = dynamic_cast<const DataHandleProperty*>( p ); hdl ) {
    // dynamic_cast to support also classes derived from DataHandleProperty
    const Gaudi::DataHandle& base = hdl->value();

    pvalue              = base.pythonRepr();
    m_importDataHandles = true;
  } else {
    std::ostringstream v_str;
    v_str.setf( std::ios::showpoint ); // to correctly display floats
    p->toStream( v_str );
    pvalue = v_str.str();
  }
}

//-----------------------------------------------------------------------------
int createAppMgr()
//-----------------------------------------------------------------------------
{
  IInterface*        iface = Gaudi::createApplicationMgr();
  SmartIF<IAppMgrUI> appUI( iface );
  auto               propMgr = appUI.as<IProperty>();
  if ( !propMgr || !appUI ) return EXIT_FAILURE;

  propMgr->setProperty( "JobOptionsType", "NONE" ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ ); // No job
                                                                                                            // options
  propMgr->setProperty( "AppName", "" ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ ); // No initial printout
                                                                                                 // message
  propMgr->setProperty( "OutputLevel", 7 ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ ); // No other printout
                                                                                                    // messages
  appUI->configure().ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
  auto msgSvc = SmartIF<IMessageSvc>{ iface }.as<IProperty>();
  msgSvc->setPropertyRepr( "setWarning", "['DefaultName', 'PropertyHolder']" )
      .ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
  msgSvc->setProperty( "Format", "%T %0W%M" ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
  return EXIT_SUCCESS;
}
