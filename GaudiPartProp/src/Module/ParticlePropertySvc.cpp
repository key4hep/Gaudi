/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Decays/CC.h>
#include <Gaudi/Decays/Symbols.h>
#include <Gaudi/Interfaces/IParticlePropertySvc.h>
#include <Gaudi/ParticleID.h>
#include <Gaudi/ParticleProperty.h>
#include <GaudiKernel/IFileAccess.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/PhysicalConstants.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/VectorMap.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <set>

/** @file
 *  Simple implementation of class Gaudi::ParticlePropertySvc
 *  @author      : I. Last
 *  - Modified by  G.Corti 09/11/1999 to read file from LHCBDBASE and
 *                 introduce find/eraseByStdHepID methods
 *  - Modifies by Vanya BELYAEV to allow readig of additional files to
 *                add/replace/modify the existing particle properties
 */
namespace {
  std::string defaultFilename() { // the main file with particle properties
    auto* root = getenv( "PARAMFILESROOT" );
    return root ? std::string{ root } + "/data/ParticleTable.txt" : std::string{ "./ParticleTable.txt" };
  }
} // namespace
namespace Gaudi {
  /** @class ParticlePropertySvc ParticlePropertySvc.cpp
   *
   *  This service provides access to particle properties.
   *  The settable property of this service is the
   *  ParticlePropertiesFile, an ASCII file containing the
   *  list of properties for the particles.
   *  The default file is:
   *      "$PARAMFILESROOT/data/ParticleTable.txt"
   *      or if $PARAMFILESROOT is not defined
   *      "ParticleTable.txt"
   *
   *  @author Iain Last
   *  @author Gloria Corti
   *  @author Vanya BELYAEV
   *  @date 2006-09-22
   *
   *  1) Modified to add possibility to redefine properties of existing
   *  particles and to read addiitonal files.
   *  New propeety "OtherFiles" (default is empty vector) is introduces.
   *  Service parsed additional files after the main one.
   *
   *    @code
   *
   *    // read the additional files with particle properties
   *    ParticlePropertySvc.OtherFiles += {
   *       "$SOMELOCATION1/file1.txt" ,
   *       "$SOMELOCATION2/file2.txt"
   *    } ;
   *
   *    @endcode
   *
   *  2) Add possibility to modify only certain partiles through the
   *  new property "Particles"  (default is enpty list),
   *  Each line is interpreted as a line in particle data table, e.g.
   *
   *   @code
   *
   *   // redefine the properties of H_20 and H_30 particles:
   *   ParticlePropertySvc.Particles = {
   *       "H_20 88 35 0.0 120.0 9.4e-26 Higgs'0 35 0.0e+00" ,
   *       "H_30 89 36 0.0  40.0 1.0e-12      A0 36 0.0e+00"
   *     } ;
   *
   *   @endcode
   *
   *  The replaces/modified particles are reported.
   */
  struct ParticlePropertySvc final : public extends<Service, Gaudi::Interfaces::IParticlePropertySvc> {
    /** get the begin-iterator for the container of particle properties
     *  It is assumed that the container is properly ordered
     *  @return begin-iterator for the container of particle properties
     */
    iterator begin() const override { return m_vector.begin(); }
    /** get the end-iterator for the container of particle properties
     *  It is assumed that the container is properly ordered
     *  @return end-iterator for the container of particle properties
     */
    iterator end() const override { return m_vector.end(); }
    /// get the container size.
    size_t size() const override { return m_vector.size(); }
    /** Retrieve an object by name:
     *
     *  @code
     *
     *   Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *   const std::string& name = ... ;
     *
     *   const Gaudi::ParticleProperty*  pp = svc -> find ( name ) ;
     *
     *  @endcode
     *  @param name the particle name
     *  @return pointer to particle property object
     */
    const Gaudi::ParticleProperty* find( const std::string& name ) const override { return m_nameMap( name ); }
    /** Retrieve an object by PID:
     *
     *  @code
     *
     *   Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *   const Gaudi::ParticleID& pid = ... ;
     *
     *   const Gaudi::ParticleProperty*  pp = svc -> find ( pid ) ;
     *
     *  @endcode
     *  @param name the particle name
     *  @return pointer to particle property object
     */
    const Gaudi::ParticleProperty* find( const Gaudi::ParticleID& pid ) const override { return m_pidMap( pid ); }

    /** make the charge conjugation for the string/decay descriptor
     *
     *  @code
     *
     *   std::string decay = "B0 -> pi+ pi-" ;
     *
     *   Gaudi::IParticleProeprtySvc* svc = ... ;
     *
     *   std::string cc = svc -> cc ( decay ) ;
     *
     *  @endcode
     *
     *  @param decay the decay descriptor
     *  @return the charge conjugation for the decay descriptor
     */
    std::string cc( const std::string& decay ) const override;

    /// Initialize the service.
    StatusCode initialize() override;

    /** Standard Constructor.
     *  @param  name   service instance name
     *  @param  pSvc   pointer to service locator `
     */
    ParticlePropertySvc( const std::string& name, // the service instance name
                         ISvcLocator*       pSvc )      // the Service Locator
        : base_class( name, pSvc ) {}

    /** the action  in the case of interactive manipulation with properties:
     *   - no action if the internal data is not yet build
     *   - else rebuild the internal data
     *  Such action will allow more flexible interactive configuration
     *  of the service
     *  @param p the updated property
     */
    void updateHandler( Gaudi::Details::PropertyBase& p );

    /** the action  in the case of interactive manipulation with properties:
     *  of the service
     *  @param p the updated property
     */
    void updateDump( Gaudi::Details::PropertyBase& p );
    /** the action  in the case of interactive manipulation with properties:
     *  of the service
     *  @param p the updated property
     */
    void updateCC( Gaudi::Details::PropertyBase& p );
    /// dump the table of particle properties
    void dump();

    /** rebuild the Particle Property Data
     *   - clear existing containers
     *   - parse main file
     *   - parse additional files
     *   - parse the specific options
     *   - set particle<->antiparticle links
     *   @return status code
     */
    StatusCode rebuild();

    /** parse the file
     *  @param file the file name to be parsed
     *  @return status code
     */
    StatusCode parse( const std::string& file );
    /** parse the line
     *  @param line the line to be parsed
     *  @return status code
     */
    StatusCode parseLine( const std::string& line );

    /** add new particle (or redefine the existing one)
     *  @param pname  the particle  name
     *  @param pid    the particle ID
     *  @param charge the particle charge
     *  @param mass   the particle mass
     *  @param tlife  the particle tlife
     *  @param maxWidth the particle max-width
     *  @param evtgen the name for EvtGen
     *  @param pythia the ID for pythia generator
     */
    StatusCode addParticle( const std::string& pname, const Gaudi::ParticleID& pid, const double charge,
                            const double mass, const double tlife, const double maxWidth, const std::string& evtgen,
                            const int pythia );

    /** set properly particle<-->antiparticle relations
     *  @return status code
     */
    StatusCode setAntiParticles();

    /// check the difference of two properties and fill corresponding sets
    bool diff( const Gaudi::ParticleProperty& n, const Gaudi::ParticleProperty& o );

    /// the actual storage of all properties
    typedef std::set<std::unique_ptr<Gaudi::ParticleProperty>> Set;
    /// the actual type of map: { "name" : "property" }
    typedef GaudiUtils::VectorMap<std::string, const Gaudi::ParticleProperty*> NameMap;
    /// the actual type of map: { "pid"  : "property" }
    typedef GaudiUtils::VectorMap<Gaudi::ParticleID, const Gaudi::ParticleProperty*> PidMap;
    typedef Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties              Vector;
    /// the actual storage of all particle properties
    Set m_set;
    /// "visible" data (the ordered container)
    Vector m_vector;
    /// Map:   { "name" : "property" }
    NameMap m_nameMap;
    /// Map:   { "pid"  : "property" }
    PidMap m_pidMap;
    /// dump the table?
    Gaudi::Property<bool> m_dump{ this, "Dump", false, &Gaudi::ParticlePropertySvc::updateDump,
                                  "Dump all properties in a table format" };

    /// the actual type for the list of files
    typedef std::vector<std::string> Files;
    /// the actual type for the list of particle properties (strings)
    typedef std::vector<std::string> Particles;
    /// the main file with particle properties
    Gaudi::Property<std::string> m_filename{ this, "ParticlePropertiesFile", defaultFilename(),
                                             &Gaudi::ParticlePropertySvc::updateHandler,
                                             "The name of 'main' particle properties file" };
    /// additional files
    Gaudi::Property<Files> m_other{ this,
                                    "OtherFiles",
                                    {},
                                    &Gaudi::ParticlePropertySvc::updateHandler,
                                    "The (optional) list of additional files with the particle data" };
    /// properties to be redefined explicitly
    Gaudi::Property<Particles> m_particles{ this,
                                            "Particles",
                                            {},
                                            &Gaudi::ParticlePropertySvc::updateHandler,
                                            "The (optional) list of special particle properties" };

    /// the CC-map
    mutable Decays::CC::MapCC m_ccMap;
    /// CC-map for properties
    Gaudi::Property<std::map<std::string, std::string>> m_ccmap_{
        this, "ChargeConjugations", Decays::Symbols::instance().cc(), &Gaudi::ParticlePropertySvc::updateCC,
        "The map of charge-conjugation & protected symbols" };

    typedef std::set<std::string>       NameSet;
    typedef std::set<Gaudi::ParticleID> PidSet;

    NameSet m_by_charge;
    NameSet m_by_mass;
    NameSet m_by_tlife;
    NameSet m_by_width;
    NameSet m_by_evtgen;
    NameSet m_by_pythia;

    NameSet m_replaced_names;
    PidSet  m_replaced_pids;
    NameSet m_no_anti;

    Vector m_modified;
  };
} // namespace Gaudi

StatusCode Gaudi::ParticlePropertySvc::initialize() {
  // 1) initialize the base
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) { return sc; }
  // 2) create the log
  MsgStream log( msgSvc(), name() );
  // 3) rebuild everything
  sc = rebuild();
  log << MSG::INFO << "Initialising Gaudi ParticlePropertySvc" << endmsg;
  if ( sc.isFailure() ) {
    log << MSG::ERROR << " Unable to initialize the internal structures " << endmsg;
    return sc;
  }

  m_by_charge.clear();
  m_by_mass.clear();
  m_by_tlife.clear();
  m_by_width.clear();
  m_by_evtgen.clear();
  m_by_pythia.clear();
  m_replaced_names.clear();
  m_replaced_pids.clear();
  m_no_anti.clear();

  m_ccMap.clear();

  if ( m_dump.value() || msgLevel( MSG::DEBUG ) ) { dump(); }

  return StatusCode::SUCCESS;
}

/* rebuild the Particle Property Data
 *   - clear existing containers
 *   - parse main file
 *   - parse additional files
 *   - parse the specific options
 *   - set particle<->antiparticle links
 *   @return status code
 */
StatusCode Gaudi::ParticlePropertySvc::rebuild() {
  // clear all existing containers
  m_nameMap.clear();
  m_pidMap.clear();
  m_vector.clear();
  m_ccMap.clear();

  // parse the main file
  StatusCode sc = parse( m_filename.value() );
  if ( sc.isFailure() ) { return sc; }
  // parse the additional files
  for ( const auto& file : m_other.value() ) {
    if ( sc = parse( file ); sc.isFailure() ) { return sc; }
  }
  // parse the options/lines
  m_modified.clear();
  for ( const auto& line : m_particles.value() ) {
    if ( sc = parseLine( line ); sc.isFailure() ) { return sc; }
  }
  // sort the vector
  std::stable_sort( m_vector.begin(), m_vector.end(), Gaudi::ParticleProperty::Compare() );
  // set particle<-->antiparticle links
  sc = setAntiParticles();
  if ( sc.isFailure() ) { return sc; }
  // some debug printout
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << " All:   " << m_vector.size() << " By Name: " << m_nameMap.size() << " By PID: " << m_pidMap.size()
            << " Total: " << m_set.size() << endmsg;
  }

  if ( !m_modified.empty() ) {
    auto& log = always();
    log << " New/updated particles (from \"Particles\" property)" << std::endl;
    Gaudi::ParticleProperties::printAsTable( m_modified, log, this );
    log << endmsg;
    m_modified.clear();
  }

  if ( !m_by_charge.empty() ) {
    info() << " Charge   has beed redefined for " << Gaudi::Utils::toString( m_by_charge ) << endmsg;
  }
  if ( !m_by_mass.empty() ) {
    info() << " Mass     has beed redefined for " << Gaudi::Utils::toString( m_by_mass ) << endmsg;
  }
  if ( !m_by_tlife.empty() ) {
    info() << " Lifetime has beed redefined for " << Gaudi::Utils::toString( m_by_tlife ) << endmsg;
  }
  if ( !m_by_width.empty() ) {
    info() << " MaxWidth has beed redefined for " << Gaudi::Utils::toString( m_by_width ) << endmsg;
  }
  if ( !m_by_evtgen.empty() ) {
    info() << " EvtGenID has beed redefined for " << Gaudi::Utils::toString( m_by_evtgen ) << endmsg;
  }
  if ( !m_by_pythia.empty() ) {
    info() << " PythiaID has beed redefined for " << Gaudi::Utils::toString( m_by_pythia ) << endmsg;
  }
  if ( !m_replaced_names.empty() ) {
    info() << " Replaced names : " << Gaudi::Utils::toString( m_replaced_names ) << endmsg;
  }
  if ( !m_replaced_pids.empty() ) {
    info() << " Replaced PIDs  : " << Gaudi::Utils::toString( m_replaced_pids ) << endmsg;
  }
  if ( !m_no_anti.empty() ) { info() << " No anti particle : " << Gaudi::Utils::toString( m_no_anti ) << endmsg; }

  return StatusCode::SUCCESS;
}

/*  the action  in the case of interactive manipulation with properties:
 *   - no action if the internal data is not yet build
 *   - else rebuild the internal data
 *  Such action will allow more flexible interactive configuration
 *  of the service
 */
void Gaudi::ParticlePropertySvc::updateHandler( Gaudi::Details::PropertyBase& p ) {
  if ( FSMState() < Gaudi::StateMachine::INITIALIZED ) { return; }

  info() << "Property triggers the update of internal Particle Property Data : " << p << endmsg;
  // rebuild the internal data
  StatusCode sc = rebuild();
  if ( sc.isFailure() ) {
    throw GaudiException( "Can't rebuild Particle Properties Data", "*ParticlePropertySvc*", sc );
  }
  // clear CC-map
  m_ccMap.clear();
}

//  the action  in the case of redefinition of "ChargeConjugates"
void Gaudi::ParticlePropertySvc::updateCC( Gaudi::Details::PropertyBase& ) { m_ccMap.clear(); }

/* the action  in the case of interactive manipulation with properties:
 *  of the service
 *  @param p the updated property
 */
void Gaudi::ParticlePropertySvc::updateDump( Gaudi::Details::PropertyBase& ) {
  if ( FSMState() < Gaudi::StateMachine::INITIALIZED ) { return; }
  dump();
}

StatusCode Gaudi::ParticlePropertySvc::parse( const std::string& file ) {
  auto fileAccess = service<IFileAccess>( "VFSSvc" );
  if ( !fileAccess ) {
    error() << "Unable to locate IFileAccess('VFSSvc') service" << endmsg;
    return StatusCode::FAILURE;
  }
  // "open" the file
  auto infile = fileAccess->open( file );
  if ( !infile.get() ) {
    error() << "Unable to open file '" << file << "'" << endmsg;
    return StatusCode::FAILURE;
  }
  info() << "Opened particle properties file : " << file << endmsg;
  bool active = false;
  // read the file line-by-line
  while ( *infile ) {
    std::string line;
    std::getline( *infile, line );
    // skip empty lines:
    if ( line.empty() ) { continue; }
    // comment lines start with '#'
    if ( line[0] == '#' ) { continue; }
    //
    if ( !active ) {
      if ( "PARTICLE" == boost::to_upper_copy( boost::trim_copy( line ) ) ) {
        active = true;
        continue;
      }
    } else {
      if ( "END PARTICLE" == boost::to_upper_copy( boost::trim_copy( line ) ) ) {
        active = false;
        continue;
      }
    }

    if ( !active ) { continue; } // skip the lines if not active
    // parse the line
    if ( auto sc = parseLine( line ); sc.isFailure() ) {
      error() << "Unable to parse the file '" << file << "'" << endmsg;
      return sc;
    }
  }

  return StatusCode::SUCCESS;
}

/*  parse the line
 *  the format of the line is defined by old SICB CDF
 *  @param line the line to be parsed
 *  @return status code
 */
StatusCode Gaudi::ParticlePropertySvc::parseLine( const std::string& line ) {
  // get the input stream  from the line :
  std::istringstream input( line );
  // get the name
  std::string p_name;
  int         p_geant; // obsolete, to be ignored
  int         p_pdg;
  double      p_charge;
  double      p_mass;
  double      p_ltime;
  std::string p_evtgen;
  int         p_pythia;
  double      p_maxwid;
  // parse the line
  if ( input >> p_name >> p_geant >> p_pdg >> p_charge >> p_mass >> p_ltime >> p_evtgen >> p_pythia >> p_maxwid ) {
    // Negative lifetime means the width in GeV-units
    if ( 0 > p_ltime ) {
      p_ltime = Gaudi::Units::hbar_Planck / std::abs( p_ltime * Gaudi::Units::GeV ) / Gaudi::Units::s;
    }

    StatusCode sc = addParticle( p_name, Gaudi::ParticleID( p_pdg ), p_charge,
                                 p_mass * Gaudi::Units::GeV,   // rescale from CDF units
                                 p_ltime * Gaudi::Units::s,    // rescale from CDF units
                                 p_maxwid * Gaudi::Units::GeV, // rescale from CDF units
                                 p_evtgen, p_pythia );
    if ( sc.isFailure() ) { return sc; }
  } else {
    MsgStream log( msgSvc(), name() );
    log << MSG::ERROR << " could not parse the line: '" << line << "'" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode Gaudi::ParticlePropertySvc::addParticle( const std::string& pname, const Gaudi::ParticleID& pid,
                                                    const double charge, const double mass, const double tlife,
                                                    const double maxWidth, const std::string& evtgen,
                                                    const int pythia ) {
  // create the local object
  const Gaudi::ParticleProperty pp( pname, pid, charge, mass, tlife, maxWidth, evtgen, pythia );

  // 1) find the object with same name & pid in set:
  auto it = std::find_if( m_set.begin(), m_set.end(), [&]( const std::unique_ptr<Gaudi::ParticleProperty>& s ) {
    return s->name() == pp.name() && s->pid() == pp.pid();
  } );
  // 2) object is found, redefine it!
  Gaudi::ParticleProperty* newp = nullptr;
  if ( m_set.end() != it ) { newp = it->get(); }
  // new property ?
  if ( !newp ) {
    it   = m_set.insert( std::make_unique<Gaudi::ParticleProperty>( pp ) ).first;
    newp = it->get();
  } else if ( diff( *newp, pp ) ) {
    *newp = pp; // NB: redefine the properties
  }

  // insert into name map
  {
    auto i1 = m_nameMap.find( newp->name() );
    if ( m_nameMap.end() != i1 && i1->second != newp ) { m_replaced_names.insert( newp->name() ); }
    m_nameMap.update( newp->name(), newp );
  }
  // insert into PID map
  {
    auto i2 = m_pidMap.find( newp->pid() );
    if ( m_pidMap.end() != i2 && i2->second != newp ) { m_replaced_pids.insert( newp->pid() ); }
    m_pidMap.update( newp->pid(), newp );
  }
  // insert into vector
  if ( m_vector.end() == std::find( m_vector.begin(), m_vector.end(), newp ) ) { m_vector.push_back( newp ); }

  m_modified.push_back( newp );

  return StatusCode::SUCCESS;
}

StatusCode Gaudi::ParticlePropertySvc::setAntiParticles() {
  for ( const Gaudi::ParticleProperty* _pp : m_vector ) {
    Gaudi::ParticleProperty* pp = const_cast<Gaudi::ParticleProperty*>( _pp );
    pp->setAntiParticle( nullptr );
    // get the ID for antiParticle
    Gaudi::ParticleID panti( -pp->particleID().pid() );
    //
    const Gaudi::ParticleProperty* anti = m_pidMap( panti );
    //
    if ( !anti && 0 < pp->particleID().pid() && !pp->particleID().isNucleus() ) { anti = _pp; }
    //
    pp->setAntiParticle( anti );
    if ( pp->antiParticle() ) {
      if ( msgLevel( MSG::VERBOSE ) )
        verbose() << "Antiparticle for \n" << ( *pp ) << " is set to be    \n" << ( *( pp->antiParticle() ) ) << endmsg;
    }
    if ( ( !pp->antiParticle() ) && ( 0 == pp->pid().extraBits() ) ) { m_no_anti.insert( pp->name() ); }
  }
  return StatusCode::SUCCESS;
}

namespace {
  inline bool different( const double a, const double b, const double p = 1.e-8 ) {
    return std::abs( a - b ) > ( std::abs( a ) + std::abs( b ) ) * std::abs( p );
  }
  inline bool different( const std::string& a, const std::string& b ) { return a != b; }
} // namespace

bool Gaudi::ParticlePropertySvc::diff( const Gaudi::ParticleProperty& n, const Gaudi::ParticleProperty& o ) {
  bool d = false;

  if ( different( n.charge(), o.charge() ) ) {
    m_by_charge.insert( n.name() );
    d = true;
  }
  if ( different( n.mass(), o.mass() ) ) {
    m_by_mass.insert( n.name() );
    d = true;
  }
  if ( different( n.lifetime(), o.lifetime() ) ) {
    m_by_tlife.insert( n.name() );
    d = true;
  }
  if ( different( n.maxWidth(), o.maxWidth() ) ) {
    m_by_width.insert( n.name() );
    d = true;
  }
  if ( different( n.evtGen(), o.evtGen() ) ) {
    m_by_evtgen.insert( n.name() );
    d = true;
  }
  if ( different( n.pythia(), o.pythia() ) ) {
    m_by_pythia.insert( n.name() );
    d = true;
  }

  if ( d ) {
    MsgStream log( msgSvc(), name() );
    if ( log.level() <= MSG::DEBUG )
      log << MSG::DEBUG << " Change the properties of '" << n.name() << "'/" << n.pid().pid() << std::endl
          << " New: " << n << std::endl
          << " Old: " << o << endmsg;
  }
  return d;
}

void Gaudi::ParticlePropertySvc::dump() {
  auto& log = always();
  log << " The Table of Particle Properties " << std::endl;
  Gaudi::ParticleProperties::printAsTable( m_vector, log, this );
  log << endmsg;
}

/* make the charge conjugation for the string/decay descriptor
 *
 *  @code
 *
 *   std::string decay = "B0 -> pi+ pi-" ;
 *
 *   Gaudi::IParticlePropertySvc* svc = ... ;
 *
 *   std::string cc = svc -> cc ( decay ) ;
 *
 *  @endcode
 *
 *  @param decay the decay descriptor
 *  @return the charge conjugation for the decay descriptor
 */
std::string Gaudi::ParticlePropertySvc::cc( const std::string& decay ) const {
  // build the map if not done yet
  if ( m_ccMap.empty() ) {
    // get the particles from the service
    for ( const auto& pp : m_vector ) {
      if ( !pp ) { continue; }
      const Gaudi::ParticleProperty* anti = pp->antiParticle();
      if ( !anti ) { continue; }
      m_ccMap[pp->particle()] = anti->particle();
    }
    // get the particles from the options
    for ( const auto& ic : m_ccmap_.value() ) {
      m_ccMap[ic.first]  = ic.second;
      m_ccMap[ic.second] = ic.first;
    }
    if ( msgLevel( MSG::DEBUG ) ) {
      Gaudi::Utils::toStream( m_ccMap, ( debug() << " CC-map is \n" ).stream() );
      debug() << endmsg;
    }
  }
  // use the map
  return Decays::CC::cc( decay, m_ccMap );
}

DECLARE_COMPONENT( Gaudi::ParticlePropertySvc )
