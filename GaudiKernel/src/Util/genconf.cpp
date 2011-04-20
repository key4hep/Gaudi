//$Id: genconf.cpp,v 1.35 2008/10/15 21:51:24 marcocle Exp $	//

#ifdef _WIN32
// Disable a warning in Boost program_options headers:
// inconsistent linkage in program_options/variables_map.hpp
#pragma warning ( disable : 4273 )

// Avoid conflicts between windows and the message service.
#define NOMSG
#define NOGDI
#endif

#ifdef __ICC
// disable icc warning #279: controlling expression is constant
// ... a lot of noise produced by the boost/filesystem/operations.hpp
#pragma warning(disable:279)
// Avoid icc remark #193: zero used for undefined preprocessing identifier "_MSC_VER"
#if !defined(_WIN32) && !defined(_MSC_VER)
#define _MSC_VER 0
#endif
#endif

// Include files----------------------------------------------------------------
#include "boost/program_options.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/exception.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/algorithm/string/case_conv.hpp"
#include "boost/format.hpp"
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "GaudiKernel/System.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IAuditor.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/GaudiHandle.h"

#include "GaudiKernel/Time.h"

#include "Reflex/PluginService.h"
#include "Reflex/Reflex.h"
#include "Reflex/SharedLibrary.h"

#include "RVersion.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <set>
#include <vector>

#include "GaudiDBM.h"

#include "DsoUtils.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

using namespace std;
using namespace ROOT::Reflex;

// useful typedefs
typedef std::vector<std::string> Strings_t;
typedef std::vector<fs::path>    LibPathNames_t;

namespace {
  const std::string py_tab = "    ";
}

class configGenerator
{
  /// name of the package we are processing
  string  m_pkgName;

  /// absolute path to the directory where genconf will store auto-generated
  /// files (Configurables and ConfigurableDb)
  string m_outputDirName;

  /// buffer of auto-generated configurables
  stringstream m_pyBuf;

  stringstream m_newConfBuf;
  set<string> m_propertyTypes;
  set<string> m_headers;

  /// switch to decide if the generated configurables need
  /// to import GaudiHandles (ie: if one of the components has a XyzHandle<T>)
  bool    m_importGaudiHandles;

  /// buffer of generated configurables informations for the "Db" file
  /// The "Db" file is holding informations about the generated configurables
  /// This file is later one used by the PropertyProxy.py to locate
  /// Configurables and know their default values, host module,...
  stringstream m_dbBuf;

  /// Configurable customization. Contains customization for:
  ///  - Name of the module where configurable base classes are defined
  ///  - Name of the configurable base class for the Algorithm component
  ///  - Name of the configurable base class for the AlgTool component
  ///  - Name of the configurable base class for the Service component
  GaudiUtils::HashMap<std::string, std::string> m_configurable;

  /// gdbm file with the database of configurables.
  Gaudi::DBM m_confDB;

public:
  configGenerator( const string& pkgName,
		   const string& outputDirName,
		   const string& confDB ) :
    m_pkgName           ( pkgName ),
    m_outputDirName     ( outputDirName ),
    m_pyBuf             ( ),
    m_newConfBuf        ( ),
    m_importGaudiHandles( false ),
    m_dbBuf             ( ),
    m_configurable      ( ),
    m_confDB            (confDB, Gaudi::DBM::WriteOrCreate)
  {}
  /// main entry point of this class:
  ///  - iterate over all the modules (ie: library names)
  ///  - for each module extract component informations
  ///  - eventually generate the header/body/trailer python file and "Db" file
  int genConfig( const Strings_t& modules );

  /// customize the Module name where configurable base classes are defined
  void setConfigurableModule( const std::string& moduleName )
  {
    m_configurable[ "Module" ] = moduleName;
  }

  /// customize the default name for configurable instances
  void setConfigurableDefaultName( const std::string& defaultName )
  {
    m_configurable[ "DefaultName" ] = defaultName;
  }

  /// customize the configurable base class for Algorithm component
  void setConfigurableAlgorithm( const std::string& cfgAlgorithm )
  {
    m_configurable[ "Algorithm" ] = cfgAlgorithm;
  }

  /// customize the configurable base class for AlgTool component
  void setConfigurableAlgTool( const std::string& cfgAlgTool )
  {
    m_configurable[ "AlgTool" ] = cfgAlgTool;
  }

  /// customize the configurable base class for AlgTool component
  void setConfigurableAuditor( const std::string& cfgAuditor )
  {
    m_configurable[ "Auditor" ] = cfgAuditor;
  }

  /// customize the configurable base class for Service component
  void setConfigurableService( const std::string& cfgService )
  {
    m_configurable[ "Service" ] = cfgService;
    m_configurable[ "ApplicationMgr" ] = cfgService;
  }
  void setValidatorHeaders(const Strings_t& hdrs) {
    m_headers.insert(hdrs.begin(), hdrs.end());
  }

private:
  void genComponent( const std::string& libName,
		     const std::string& componentName,
		     const std::string& componentType,
		     const vector<Property*>& properties );
  void genImport( std::ostream& s, const boost::format& frmt,std::string indent);
  void genHeader( std::ostream& pyOut, std::ostream& dbOut );
  void genBody( std::ostream& pyOut,
		std::ostream& dbOut ) {
    pyOut << m_pyBuf.str() << flush;
    dbOut << m_dbBuf.str() << flush;
  }

  void genValidatorSource(const string &dir, const string &lib) {
    {
      fstream data((fs::path(dir) / fs::path(lib + "_validators.cpp")).string().c_str(),
          ios_base::out|ios_base::trunc);
      data << "// --- Auto generated file: DO NOT EDIT ---\n\n"
              "// This needs to be first to avoid warnings\n"
              "#include <boost/python.hpp>\n"
              "// Headers required by the validators\n"
              "#include <GaudiKernel/Property.h>\n";
      for (set<string>::iterator t = m_headers.begin();
          t != m_headers.end(); ++t) {
        data << "#include <" << *t << ">\n";
      }
      data << "// Common code for the validators (implementation of \"check\")\n"
              "//   must be after the headers\n"
              "#define PYCONF_VALIDATOR_MODULE\n"
              "#include <GaudiKernel/PyConfValidators.h>\n"
              "// Code of the Python module\n"
           << "BOOST_PYTHON_MODULE(" << lib << "_validators)\n{\n"
              "using namespace boost::python;\n";

      for (set<string>::iterator t = m_propertyTypes.begin();
          t != m_propertyTypes.end(); ++t) {
        const string valKey = "validator->" + *t;
        if (m_confDB.exists(valKey) && (m_confDB.fetch(valKey) != lib))
          continue; // skip the property type if already present in another validator library

        data << "def(\"" << *t << "\",\n"
                "    check";
        string::size_type p = t->find_first_of('<');
        if (p != string::npos) {
          data << t->substr(p);
        } // Special cases
        else if (*t == "GaudiHandleProperty") { // equivalent to a string
          data << "<std::string,NullVerifier<std::string> >";
        }
        else if (*t == "GaudiHandleArrayProperty") { // equivalent to vector<string>
          data << "<std::vector<std::string,std::allocator<std::string> >,NullVerifier<std::vector<std::string,std::allocator<std::string> > > >";
        }
        else { // If unknown, use the catch-all string type
          data << "<std::string,NullVerifier<std::string> >";
        }
        data << ");\n";

        // mark this property type as present for this library
        m_confDB.store(valKey, lib, true);
      }
      data << "}\n";

    }
  }

  void genTrailer( std::ostream& pyOut,
		   std::ostream& dbOut );

  /// handle the "marshalling" of Properties
  void pythonizeValue( const Property* prop,
		       string& pvalue,
		       string& ptype );

  /// Translates a valid C++ typename into a valid python one
  void pythonizeName( string& name );
};

int createAppMgr();

//--- Command main program-----------------------------------------------------
int main ( int argc, char** argv )
//-----------------------------------------------------------------------------
{
  fs::path::default_name_check(fs::native);
  fs::path pwd = fs::initial_path();
  fs::path out;
  Strings_t libs;
  std::string pkgName;

  // declare a group of options that will be allowed only on command line
  po::options_description generic("Generic options");
  generic.add_options()
    ("help,h",
     "produce this help message")
    ("package-name,p",
     po::value<string>(),
     "name of the package for which we create the configurables file")
    ("input-libraries,i",
     po::value<string>(),
     "libraries to extract the component configurables from")
    ("input-cfg,c",
     po::value<string>(),
     "path to the cfg file holding the description of the Configurable base "
     "classes, the python module holding the Configurable definitions, etc...")
    ("output-dir,o",
     po::value<string>()->default_value("../genConf"),
     "output directory for genconf files.")
    ("debug-level,d",
     po::value<int>()->default_value(0),
     "debug level")
    ("load-library,l",
     po::value< Strings_t >()->composing(),
     "preloading library")
    ("validator-include,I",
     po::value<Strings_t>(),
     "extra headers to be added to the validator C++/Python module")
    ("configurables-db",
     po::value<string>()->default_value("../genConf/_configurables.db"),
     "gdbm file containing the database of configurables")
    ;

  // declare a group of options that will be allowed both on command line
  // _and_ in configuration file
  po::options_description config("Configuration");
  config.add_options()
    ("configurable-module",
     po::value<string>()->default_value("AthenaCommon"),
     "Name of the module holding the configurable classes")
    ("configurable-default-name",
     po::value<string>()->default_value("Configurable.DefaultName"),
     "Default name for the configurable instance")
    ("configurable-algorithm",
     po::value<string>()->default_value("ConfigurableAlgorithm"),
     "Name of the configurable base class for Algorithm components")
    ("configurable-algtool",
     po::value<string>()->default_value("ConfigurableAlgTool"),
     "Name of the configurable base class for AlgTool components")
    ("configurable-auditor",
     po::value<string>()->default_value("ConfigurableAuditor"),
     "Name of the configurable base class for Auditor components")
    ("configurable-service",
     po::value<string>()->default_value("ConfigurableService"),
     "Name of the configurable base class for Service components")
    ;

  po::options_description cmdline_options;
  cmdline_options.add(generic).add(config);

  po::options_description config_file_options;
  config_file_options.add(config);

  po::options_description visible("Allowed options");
  visible.add(generic).add(config);

  po::variables_map vm;

  try {
    po::store( po::command_line_parser(argc, argv).
	       options(cmdline_options).run(),
	       vm );

    po::notify(vm);

    // try to read configuration from the optionally given configuration file
    if( vm.count("input-cfg") ) {
      string cfgFileName = vm["input-cfg"].as<string>();
      cfgFileName = fs::complete( fs::path( cfgFileName,
					    fs::native ) ).string();
      std::ifstream ifs( cfgFileName.c_str() );
      po::store( parse_config_file( ifs, config_file_options ), vm );
    }

    po::notify(vm);
  }
  catch ( po::error& err ) {
    cout << "ERR0R: error detected while parsing command options: "<< err.what() << endl;
    return EXIT_FAILURE;
  }

  //--- Process command options -----------------------------------------------
  if( vm.count("help")) {
    cout << visible << endl;
    return EXIT_FAILURE;
  }

  if( vm.count("package-name") ) {
    pkgName = vm["package-name"].as<string>();
  }
  else {
    cout << "ERROR: 'package-name' required" << endl;
    cout << visible << endl;
    return EXIT_FAILURE;
  }

  if( vm.count("input-libraries") ) {
    // re-shape the input arguments:
    //  - removes spurious spaces,
    //  - split into tokens.
    Strings_t inputLibs;
    {
      string tmp = vm["input-libraries"].as<string>();
      boost::trim(tmp);
      boost::split( inputLibs, tmp,
		    boost::is_any_of(" "),
		    boost::token_compress_on );
    }

    //
    libs.reserve( inputLibs.size() );
    for ( Strings_t::const_iterator iLib = inputLibs.begin();
	  iLib != inputLibs.end();
	  ++iLib ) {
      fs::path libPath = fs::path( *iLib, fs::native );
      std::string lib = libPath.leaf().substr(0, libPath.leaf().find('.') );
      if ( 0 == lib.find("lib") ) {
        lib = lib.substr(3); // For *NIX remove "lib"
      }
      // remove duplicates
      if ( !lib.empty() &&
          std::find( libs.begin(), libs.end(), lib ) == libs.end() ) {
        libs.push_back( lib );
      }
    } //> end loop over input-libraries
    if ( libs.empty() ) {
      cout << "ERROR: input component library(ies) required !\n"
           << "ERROR: 'input-libraries' argument was ["
           << vm["input-libraries"].as<string>()
           << "]"
           << endl;
      return EXIT_FAILURE;
    }
  }
  else {
    cout << "ERROR: input component library(ies) required" << endl;
    cout << visible << endl;
    return EXIT_FAILURE;
  }

  if( vm.count("output-dir") ) {
    out = fs::complete( fs::path( vm["output-dir"].as<string>(),
                                  fs::native ) );
  }

  if ( vm.count("debug-level") ) {
    PluginService::SetDebug( vm["debug-level"].as<int>() );
  }

  if ( vm.count("load-library") ) {
    Strings_t lLib_list = vm["load-library"].as< Strings_t >();
    for (Strings_t::const_iterator lLib=lLib_list.begin();
	 lLib != lLib_list.end();
	 ++lLib) {
      // load done through ROOT::Reflex helper class
      SharedLibrary tmplib(*lLib) ;
      tmplib.Load() ;
    }
  }


  if ( !fs::exists( out ) ) {
    try {
      fs::create_directory(out);
    }
    catch ( fs::filesystem_error err ) {
      cout << "ERR0R: error creating directory: "<< err.what() << endl;
      return EXIT_FAILURE;
    }
  }

  cout << ":::::: libraries : [ ";
  copy( libs.begin(), libs.end(), ostream_iterator<string>(cout, " ") );
  cout << "] ::::::" << endl;

  configGenerator py( pkgName, out.string(),
                      vm["configurables-db"].as<string>() );
  py.setConfigurableModule     (vm["configurable-module"].as<string>());
  py.setConfigurableDefaultName(vm["configurable-default-name"].as<string>());
  py.setConfigurableAlgorithm  (vm["configurable-algorithm"].as<string>());
  py.setConfigurableAlgTool    (vm["configurable-algtool"].as<string>());
  py.setConfigurableAuditor    (vm["configurable-auditor"].as<string>());
  py.setConfigurableService    (vm["configurable-service"].as<string>());
  if (vm.count("validator-include"))
    py.setValidatorHeaders(vm["validator-include"].as<Strings_t>());

  int sc = EXIT_FAILURE;
  try {
    sc = py.genConfig( libs );
  }
  catch ( exception& e ) {
    cout << "ERROR: Could not generate Configurable(s) !\n"
         << "ERROR: Got exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  if ( EXIT_SUCCESS == sc ) {
    // create an empty __init__.py file in the output dir
    fstream initPy( ( out / fs::path( "__init__.py" ) ).string().c_str(),
        std::ios_base::out|std::ios_base::trunc );
    initPy << "## Hook for " << pkgName << " genConf module\n" << flush;
  }

  cout << ":::::: libraries : [ ";
  copy( libs.begin(), libs.end(), ostream_iterator<string>(cout, " ") );
  cout << "] :::::: [DONE]" << endl;

  return sc;
}

/// Given a Reflex::Member object, return the id for the configurable (name or id, if it is a string).
/// non-string ids are used for the persistency (DataObjects)
inline std::string getId(const Member & m) {
      return (m.Properties().HasProperty("id") && (m.Properties().PropertyValue("id").TypeInfo() == typeid(std::string))) ?
             m.Properties().PropertyAsString("id") :
             m.Properties().PropertyAsString("name") ;
}

template <class T>
IProperty *makeInstance(const Member &member, const vector<void*> &args)
{
  Object dummy;
  T* obj;
#if ROOT_VERSION_CODE < ROOT_VERSION(5,21,6)
  obj = static_cast<T*>(member.Invoke(dummy,args).Address());
#else
  member.Invoke(dummy,obj,args);
#endif
  return dynamic_cast<IProperty*>(obj);
}


//-----------------------------------------------------------------------------
int configGenerator::genConfig( const Strings_t& libs )
//-----------------------------------------------------------------------------
{
  //--- Disable checking StatusCode -------------------------------------------
  StatusCode::disableChecking();

  const Strings_t::const_iterator endLib = libs.end();

  const std::string gaudiSvc = "GaudiSvc";
  const bool isGaudiSvc = ( std::find( libs.begin(), endLib, gaudiSvc ) != endLib );

  //--- Instantiate ApplicationMgr --------------------------------------------
  if ( !isGaudiSvc && createAppMgr() ) {
    cout << "ERROR: ApplicationMgr can not be created. Check environment" << endl;
    return EXIT_FAILURE;
  }

  //--- Iterate over component factories --------------------------------------
  Scope factories = Scope::ByName(PLUGINSVC_FACTORY_NS);
  if ( !factories ) {
    cout << "ERROR: No PluginSvc factory namespace could be found" << endl;
    return EXIT_FAILURE;
  }

  ISvcLocator* svcLoc = Gaudi::svcLocator();
  IInterface*  dummySvc = new Service( "DummySvc", svcLoc );
  dummySvc->addRef();

  bool allGood = true;

  // iterate over all the requested libraries
  for ( Strings_t::const_iterator iLib=libs.begin(); iLib != endLib; ++iLib ) {

    std::cout << ":::: processing library: " << *iLib << "..." << std::endl;

    // reset state
    m_importGaudiHandles = false;
    m_pyBuf.str("");
    m_newConfBuf.str("");
    m_propertyTypes.clear();
    m_dbBuf.str("");

    // Scan the pluginSvc namespace and store the "background" of already
    // alive components, so we can extract our signal later on
    set<string> bkgNames;
    if ( !isGaudiSvc ) {
      for ( Member_Iterator it = factories.FunctionMember_Begin();
            it != factories.FunctionMember_End(); ++it ) {
        string ident = getId(*it);
        if ( PluginService::Debug() > 0 ) {
          cout << "::: " << ident << endl;
        }
        bkgNames.insert( ident );
      }
    }
    const set<string>::const_iterator bkgNamesEnd = bkgNames.end();

    //--- Load component library ----------------------------------------------
    System::ImageHandle handle;
    unsigned long err = System::loadDynamicLib( *iLib, &handle );
    if ( err != 1 ) {
      cout << "ERROR: " << System::getLastErrorString() << endl;
      allGood = false;
      continue;
    }

    const std::string compLibKey(*iLib + "->components");
    {
      // Remove the components belonging to this library from the database
      std::string components = m_confDB.fetch(compLibKey);
      typedef boost::char_separator<char> sep;
      typedef boost::tokenizer<sep> tokenizer;
      tokenizer tokens(components, sep(";"));
      BOOST_FOREACH(std::string t, tokens)
      {
        m_confDB.remove(t);
      }
    }
    // keep trace of the components in the current library, to store them in the
    stringstream componentsInLib;
    bool firstComponentInLib = true;
    for ( Member_Iterator it = factories.FunctionMember_Begin();
          it != factories.FunctionMember_End();
          ++it ) {
      const string ident = getId(*it);
      if ( bkgNames.find(ident) != bkgNamesEnd ) {
        if ( PluginService::Debug() > 0 ) {
          cout << "\t==> skipping [" << ident << "]..." << endl;
        }
        continue;
      }

      // Atlas contributed code (patch #1247)
      // Skip the generation of configurables if the component does not come
      // from the same library we are processing (i.e. we found a symbol that
      // is coming from a library loaded by the linker).
      // Windows implementation is empty.
      if ( !DsoUtils::inDso( *it, DsoUtils::libNativeName(*iLib) ) ) {
        cout << "WARNING: library [" << *iLib << "] requested factory "
        << "from another library ["
        << DsoUtils::dsoName(*it) << "]"
        << " ==> [" << ident << "] !!"
        << endl;
        continue;
      }

      const string rtype = it->TypeOf().ReturnType().Name();
      string type;
      bool known = true;
      if      ( ident == "ApplicationMgr" ) type = "ApplicationMgr";
      else if ( rtype == "IInterface*" )    type = "IInterface";
      else if ( rtype == "IAlgorithm*" )    type = "Algorithm";
      else if ( rtype == "IService*" )      type = "Service";
      else if ( rtype == "IAlgTool*" )      type = "AlgTool";
      else if ( rtype == "IAuditor*" )      type = "Auditor";
      else if ( rtype == "IConverter*" )    type = "Converter";
      else if ( rtype == "DataObject*" )    type = "DataObject";
      else                                  type = "Unknown", known = false;
      string name = ident;
      // handle possible problems with templated components
      boost::trim(name);

      cout << " - component: " << name << endl;

      if ( type == "IInterface" ) {
        /// not enough information...
        /// skip it
        continue;
      }

      if ( type == "Converter" || type == "DataObject" ) {
        /// no Properties, so don't bother create Configurables...
        continue;
      }

      //if ( type == "ApplicationMgr" ) {
      ///  @FIXME: no Configurable for this component. yet...
      ///  continue;
      //}

      if ( !known ) {
        cout << "WARNING: Unknown (return) type [" << rtype << "] !!\n"
             << "WARNING: component [" << ident << "] is skipped !"
             << endl;
        allGood = false;
        continue;
      }

      string cname = "DefaultName";
      vector<void*>  args;
      args.reserve( 3 );
      if ( type == "AlgTool" ) {
        args.resize( 3 );
        args[0] = &cname;
        args[1] = &type;
        args[2] = dummySvc;
      }
      else {
        args.resize( 2 );
        args[0] = &cname;
        args[1] = svcLoc;
      }
      IProperty* prop = 0;
      try {
        if ( type == "Algorithm" ) {
          prop = makeInstance<IAlgorithm>(*it,args);
        }
        else if ( type == "Service") {
          prop = makeInstance<IService>(*it,args);
        }
        else if ( type == "AlgTool") {
          prop = makeInstance<IAlgTool>(*it,args);
        }
        else if ( type == "Auditor") {
          prop = makeInstance<IAuditor>(*it,args);
        }
        else if ( type == "ApplicationMgr") {
          //svcLoc->queryInterface(IProperty::interfaceID(), pp_cast<void>(&prop));
          svcLoc->queryInterface(IProperty::interfaceID(), (void**)(&prop));
        }
        else {
          prop = makeInstance<IInterface>(*it,args);
        }
      }
      catch ( exception& e ) {
        cout << "ERROR: Error instantiating " << name
             << " from " << *iLib << endl;
        cout << "ERROR: Got exception: " << e.what() << endl;
        allGood = false;
        continue;
      }
      catch ( ... ) {
        cout << "ERROR: Error instantiating " << name
             << " from " << *iLib << endl;
        allGood = false;
        continue;
      }
      if( prop ) {
        genComponent( *iLib, name, type, prop->getProperties() );
        prop->release();
        if (!firstComponentInLib) {
          componentsInLib << ';';
        } else {
          firstComponentInLib = false;
        }
        componentsInLib << name;
      } else {
        cout << "ERROR: could not cast IInterface* object to an IProperty* !\n"
             << "ERROR: return type from PluginSvc is [" << rtype << "]...\n"
             << "ERROR: NO Configurable will be generated for ["
             << name << "] !"
             << endl;
        allGood = false;
      }
    } //> end loop over factories

    /// Store in the database the list of components that are in the library.
    m_confDB.store(compLibKey, componentsInLib.str(), true);

    ///
    /// write-out files for this library
    ///
    const std::string pyName = ( fs::path(m_outputDirName) /
                  fs::path(*iLib+"Conf.py") ).string();
    const std::string dbName = ( fs::path(m_outputDirName) /
                  fs::path(*iLib+"_confDb.py") ).string();

    std::fstream py( pyName.c_str(),
              std::ios_base::out|std::ios_base::trunc );
    std::fstream db( dbName.c_str(),
              std::ios_base::out|std::ios_base::trunc );

    genHeader ( py, db );
    genBody   ( py, db );
    genTrailer( py, db );

    // update the list of headers required by the validators
    {
      const string k(*iLib + "->headers");
      if (!m_headers.empty()) {
        stringstream hdrs;
        for (set<string>::iterator h = m_headers.begin(); h != m_headers.end(); ++h) {
          if (h != m_headers.begin())
            hdrs << ';';
          hdrs << *h;
        }
        m_confDB.store(k, hdrs.str(), true);
      } else {
        if (m_confDB.exists(k)) m_confDB.remove(k);
      }
    }

    genValidatorSource((fs::path("..") / System::getEnv("CMTCONFIG")).string(),
                       *iLib);

  } //> end loop over libraries

  dummySvc->release();
  dummySvc = 0;

  return allGood ? EXIT_SUCCESS : EXIT_FAILURE;
}

void configGenerator::genImport( std::ostream& s,
                                 const boost::format& frmt,
                                 std::string indent = ""){

  std::string::size_type pos = 0, nxtpos = 0;
  std::string mod;

  while ( std::string::npos != pos ){
    // find end of module name
    nxtpos = m_configurable["Module"].find_first_of(',',pos);

    // Prepare import string
    mod = m_configurable["Module"].substr(pos,nxtpos-pos);
    std::ostringstream import;
    import << boost::format(frmt) % mod;

    // append a normal import or a try/except enclosed one depending
    // on availability of a fall-back module (next in the list)
    if ( std::string::npos == nxtpos ) {
      // last possible module
      s << indent << import.str() << "\n" << flush;
      pos = std::string::npos;
    } else {
      // we have a fallback for this
      s << indent << "try:\n"
        << indent << py_tab << import.str() << "\n"
        << indent << "except ImportError:\n"
        << flush;
      pos = nxtpos+1;
    }
    // increase indentation level for next iteration
    indent += py_tab;
  }
}

//-----------------------------------------------------------------------------
void configGenerator::genHeader( std::ostream& py,
				 std::ostream& db )
//-----------------------------------------------------------------------------
{
  // python file part
  std::string now = Gaudi::Time::current().format(true);
  py << "#" << now //<< "\n"
     << "\"\"\"Automatically generated. DO NOT EDIT please\"\"\"\n";
  if ( m_importGaudiHandles ) {
    py << "from GaudiKernel.GaudiHandles import *\n";
  }

  genImport(py,boost::format("from %1%.Configurable import *"));

  // db file part
  db << "##  -*- python -*-  \n"
     << "# db file automatically generated by genconf on: "
     << now; // << "\n";
  db << "## insulates outside world against anything bad that could happen\n"
     << "## also prevents global scope pollution\n"
     << "def _fillCfgDb():\n";
  genImport(db,boost::format("from %1%.ConfigurableDb import CfgDb"),py_tab);

  db << "\n"
     << py_tab << "# get a handle on the repository of Configurables\n"
     << py_tab << "cfgDb = CfgDb()\n"
     << "\n"
     << py_tab << "# populate the repository with informations on Configurables \n"
     << "\n"
     << flush;
}
//-----------------------------------------------------------------------------
void configGenerator::genTrailer( std::ostream& /*py*/,
				  std::ostream& db )
//-----------------------------------------------------------------------------
{
  // db file part
  db << py_tab << "return #_fillCfgDb\n"
     << "# fill cfgDb at module import...\n"
     << "try:\n"
     << py_tab << "_fillCfgDb()\n"
     << py_tab << "#house cleaning...\n"
     << py_tab << "del _fillCfgDb\n"
     << "except Exception,err:\n"
     << py_tab << "print \"Py:ConfigurableDb   ERROR Problem with [%s] content!"
               << "\" % __name__\n"
     << py_tab << "print \"Py:ConfigurableDb   ERROR\",err\n"
     << py_tab << "print \"Py:ConfigurableDb   ERROR   ==> culprit is package ["
               << m_pkgName << "] !\"\n"
     << std::flush;
}

//-----------------------------------------------------------------------------
void configGenerator::genComponent( const std::string& libName,
                                    const std::string& componentName,
                                    const std::string& componentType,
                                    const vector<Property*>& properties )
//-----------------------------------------------------------------------------
{
  string cname = componentName;
  pythonizeName(cname);

  stringstream compDesc;

  typedef GaudiUtils::HashMap<std::string, std::string> PropertyDoc_t;
  PropertyDoc_t propDoc;

  m_pyBuf << "\n";
  m_pyBuf << "class " << cname
          << "( " << m_configurable[componentType] << " ) :"
          << "\n";
  m_pyBuf << "  __slots__ = { \n";


  // The format of a component description is:
  //   [<prop_desc>,...]
  // The class name is the key in the database.
  compDesc << '[';
  for ( vector<Property*>::const_iterator it = properties.begin() ;
        it != properties.end(); ++it ) {
    const string pname  = (*it)->name();
    const string pclass = (*it)->propertyClass();

    string pvalue, ptype;
    pythonizeValue( (*it), pvalue, ptype );

    m_pyBuf << "    # " << pclass << "\n";
    m_pyBuf << "    '" << pname << "' : " << pvalue <<", # " << ptype << "\n";

    if ( (*it)->documentation() != "none" ) {
      propDoc[pname] = (*it)->documentation();
    }

    // Property description:
    //   (name, cppType, default, propClass, doc)
    compDesc << "('" << pname << "'"
             << ",'" << System::typeinfoName(*(*it)->type_info()) << "'"
             << ',' << pvalue
             << ",'" << pclass << "',";
    if ( (*it)->documentation() != "none" ) {
      compDesc << "'''" << (*it)->documentation() << "'''";
    } else {
      compDesc << "None";
    }
    compDesc << "),";
    m_propertyTypes.insert(pclass);
  }
  compDesc << "]";
  m_confDB.store(componentName, compDesc.str());

  m_pyBuf << "  }\n";
  m_pyBuf << "  _propertyDocDct = { \n";
  for ( PropertyDoc_t::const_iterator iProp = propDoc.begin();
        iProp != propDoc.end();
        ++iProp ) {
    m_pyBuf << std::setw(5)
            << "'"       << iProp->first  << "' : "
            << "\"\"\" "  << iProp->second << " \"\"\",\n";
  }
  m_pyBuf << "  }\n";

  m_pyBuf
    << "  def __init__(self, name = " << m_configurable["DefaultName"]
                                      << ", **kwargs):\n"
    << "      super(" << cname << ", self).__init__(name)\n"
    << "      for n,v in kwargs.items():\n"
    << "         setattr(self, n, v)\n"
    << "  def getDlls( self ):\n"
    << "      return '" << libName << "'\n"
    << "  def getType( self ):\n"
    << "      return '" << componentName  << "'\n"
    << "  pass # class " << cname << "\n"
    << flush;

  // name of the auto-generated module
  const string pyName = ( fs::path(m_outputDirName) /
			  fs::path(libName+"Conf.py") ).string();
  const string modName = fs::basename( fs::path( pyName ).leaf() );

  // now the db part
  m_dbBuf
    << py_tab << "cfgDb.add( configurable = '" << cname << "',\n"
    << py_tab << "           package = '" << m_pkgName << "',\n"
    << py_tab << "           module  = '" << m_pkgName << "." << modName << "',\n"
    << py_tab << "           lib     = '" << libName << "' )\n"
    << flush;

}
//-----------------------------------------------------------------------------
void configGenerator::pythonizeName( string& name )
//-----------------------------------------------------------------------------
{
  static string  in("<>&*,: ().");
  static string out("__rp__s___");
  for ( string::iterator i = name.begin(); i != name.end(); ++i ) {
    if ( in.find(*i) != string::npos ) *i = out[in.find(*i)];
  }
}

//-----------------------------------------------------------------------------
void configGenerator::pythonizeValue( const Property* p,
                                      string& pvalue, string& ptype )
//-----------------------------------------------------------------------------
{
  const std::string cvalue = p->toString();
  const type_info& ti = *p->type_info();
  if ( ti == typeid(bool) ) {
    pvalue = ( cvalue == "0" || cvalue == "False" || cvalue == "false" )
           ? "False"
           : "True";
    ptype  = "bool";
  }
  else if ( ti == typeid(char)  || ti == typeid(signed char)
            || ti == typeid(unsigned char)  ||
            ti == typeid(short) || ti == typeid(unsigned short) ||
            ti == typeid(int)   || ti == typeid(unsigned int)   ||
            ti == typeid(long)  || ti == typeid(unsigned long) ) {
    pvalue = cvalue;
    ptype  = "int";
  }
  else if ( ti == typeid(long long) || ti == typeid(unsigned long long) ) {
    pvalue = cvalue + "L";
    ptype  = "long";
  }
  else if ( ti == typeid(float) || ti == typeid(double) ) {
    // forces python to handle this as a float: put a dot in there...
    pvalue = boost::to_lower_copy(cvalue);
    if ( pvalue == "nan" ) {
      pvalue = "float('nan')";
      std::cout << "WARNING: default value for ["
                << p->name() << "] is NaN !!"
                << std::endl;
    } else if ( std::string::npos == pvalue.find(".") &&
                std::string::npos == pvalue.find("e")  ) {
      pvalue = cvalue + ".0";
    }
    ptype  = "float";
  }
  else if ( ti == typeid(string) ) {

    pvalue = "'"+cvalue+"'";
    ptype  = "str";
  }
  else if ( ti == typeid(GaudiHandleBase) ) {
    m_importGaudiHandles = true;
    const GaudiHandleProperty& hdl
      = dynamic_cast<const GaudiHandleProperty&>(*p);
    const GaudiHandleBase&     base = hdl.value();

    pvalue = base.pythonRepr();
    ptype  = "GaudiHandle";
  }
  else if ( ti == typeid(GaudiHandleArrayBase) ) {
    m_importGaudiHandles = true;
    const GaudiHandleArrayProperty& hdl
      = dynamic_cast<const GaudiHandleArrayProperty&>(*p);
    const GaudiHandleArrayBase&     base = hdl.value();

    pvalue = base.pythonRepr();
    ptype  = "GaudiHandleArray";
  }
  else {
    pvalue = cvalue;
    ptype  = "list";
  }
}

//-----------------------------------------------------------------------------
int createAppMgr()
//-----------------------------------------------------------------------------
{
  IInterface* iface = Gaudi::createApplicationMgr();
  SmartIF<IProperty> propMgr ( iface );
  SmartIF<IAppMgrUI> appUI  ( iface );

  if ( propMgr.isValid() && appUI.isValid() ) {
    propMgr->setProperty( "JobOptionsType", "NONE" );  // No job options
    propMgr->setProperty( "AppName", "");              // No initial printout message
    propMgr->setProperty( "OutputLevel", "7");         // No other printout messages
    appUI->configure();
    return EXIT_SUCCESS;
  }
  else {
    return EXIT_FAILURE;
  }
}
