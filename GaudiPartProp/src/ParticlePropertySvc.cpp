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
// STD&STL
// ============================================================================
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/trim.hpp"
#include <cctype>
#include <fstream>
namespace ba = boost::algorithm;
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IFileAccess.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ParticleProperty.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/System.h"
// ============================================================================
//#include "GaudiKernel/ToStream.h"
// ============================================================================
// Local
// ============================================================================
#include "ParticlePropertySvc.h"
// ============================================================================
namespace Gaudi {
  /** Instantiation of a static factory class used by clients to create
   *  instances of this service
   */
  DECLARE_COMPONENT( ParticlePropertySvc )
  // ============================================================================
  /** @file
   *
   * class ParticlePropertySvc Class
   *
   * Implement a ParticlePropertySvc
   *  @author     : I. Last
   * Modify by  : G.Corti 09/11/1999 to read file from LHCBDBASE and
   *             introduce find/eraseByStdHepID methods
   * Modified by Vanya BELYAEV to allow reading of additional files
   * to add/replace/modify the existing particle properties
   */
  // ============================================================================
  ParticlePropertySvc::ParticlePropertySvc( const std::string& name, ISvcLocator* svc ) : base_class( name, svc ) {
    /// @todo: remove reference to LHCb-specific environment variable
    // Redefine the default name:
    if ( System::getEnv( "PARAMFILESROOT", m_filename.value() ) ) { m_filename.value() += "/data/ParticleTable.txt"; }
  }
  // ============================================================================
  /// initialize the service and setProperties
  // ============================================================================
  StatusCode ParticlePropertySvc::initialize() {
    StatusCode sc = Service::initialize();
    if ( sc.isFailure() ) { return sc; }

    sc = setProperties();
    if ( sc.isFailure() ) {
      error() << " Could not set the properties " << endmsg;
      return sc;
    }

    m_fileAccess = service( "VFSSvc" );
    if ( !m_fileAccess ) {
      error() << " Cannot retrieve the VFS service " << endmsg;
      return StatusCode::FAILURE;
    }

    sc = parse();
    if ( sc.isFailure() ) {
      error() << " Could not parse the file " << endmsg;
      return sc;
    }
    if ( !m_particles.empty() ) {
      sc = addParticles();
      if ( sc.isFailure() ) {
        error() << " Could not treat particles! " << endmsg;
        return sc;
      }
    }
    debug() << "ParticleProperties parsed successfully" << endmsg;

    debug() << "Access properties" << endmsg;
    // For debugging purposes print out the size of the internal maps
    // particle name as key: all particles in database are present here
    debug() << "NameMap size =" << m_namemap.size() << endmsg;
    // Geant3 ID as key: all particles in database are present here
    debug() << "GeantID Map size =" << m_idmap.size() << endmsg;
    // StdHep ID as key: some particles have no valid StdHep ID
    debug() << "StdHepID Map size =" << m_stdhepidmap.size() << endmsg;
    // Pythia ID as key: some particles are not defined in Pythia
    debug() << "PythiaID Map size =" << m_pythiaidmap.size() << endmsg;

    if ( !m_replaced.empty() ) {
      info() << "Properties have been redefined for " << m_replaced.size()
             << " particles : " << Gaudi::Utils::toString( m_replaced ) << endmsg;
    }

    return StatusCode::SUCCESS;
  }
  // =============================================================================
  /// finalize
  // =============================================================================
  StatusCode ParticlePropertySvc::finalize() {
    if ( !m_other.empty() ) {
      info() << "Additional Properties have been read from files: " << Gaudi::Utils::toString( m_other ) << endmsg;
    }

    if ( !m_replaced.empty() ) {
      always() << "Properties have been redefined for " << m_replaced.size()
               << " particles : " << Gaudi::Utils::toString( m_replaced ) << endmsg;
    }

    m_fileAccess.reset();

    /// finalize the base class
    return Service::finalize();
  }
  // =============================================================================
  /// Create a particle property and insert it in the maps
  // =============================================================================
  StatusCode ParticlePropertySvc::push_back( const std::string& particle, int geantId, int jetsetId, double charge,
                                             double mass, double tlife, const std::string& evtName, int pythiaId,
                                             double maxWidth ) {
    //
    auto i = m_owned.insert( std::make_unique<ParticleProperty>( particle, geantId, jetsetId, charge, mass, tlife,
                                                                 evtName, pythiaId, maxWidth ) );
    //
    return i.second ? push_back( i.first->get() ) : StatusCode::FAILURE;
  }
  // =============================================================================
  /// Add a particle property.
  // =============================================================================
  StatusCode ParticlePropertySvc::push_back( ParticleProperty* pp ) {
    if ( !pp ) { return StatusCode::FAILURE; }
    //
    { // try to add into Geant(3)ID map
      const int ID = pp->geantID();
      // is this already in the map?
      auto ifind = m_idmap.find( ID );
      if ( m_idmap.end() != ifind && 0 != m_idmap[ID] ) {
        diff( ifind->second, pp );
        m_replaced.insert( m_idmap[ID]->particle() );
      }
      // put it into the map
      m_idmap[ID] = pp;
    }
    //
    { // try to add into Name map
      const std::string& particle = pp->particle();
      // is this already in the map?
      auto ifind = m_namemap.find( particle );
      if ( ifind != m_namemap.end() && ifind->second ) {
        diff( ifind->second, pp );
        m_replaced.insert( ifind->second->particle() );
      }
      // put it into the map
      m_namemap[particle] = pp;
    }
    //
    // add to StdHep map only if StdHep ID different from zero and if
    // not Cerenkov (StdHep ID = gamma)
    if ( 0 != pp->jetsetID() && "Tcherenkov" != pp->particle() ) { // try to add into StdHepID map
      const int ID = pp->jetsetID();
      // is this already in the map?
      auto ifind = m_stdhepidmap.find( ID );
      if ( m_stdhepidmap.end() != ifind && ifind->second ) {
        diff( ifind->second, pp );
        m_replaced.insert( ifind->second->particle() );
      }
      // put it into the map
      m_stdhepidmap[ID] = pp;
    }
    //
    // add to Pythia map only if Pythia ID is different from
    // zero ( StdHep id is always different from zero in this case )
    if ( 0 != pp->pythiaID() && 0 != pp->jetsetID() && "Tcherenkov" != pp->particle() ) { // try to add into PythiaID
                                                                                          // map
      const int ID = pp->pythiaID();
      // is this already in the map?
      auto ifind = m_pythiaidmap.find( ID );
      if ( m_pythiaidmap.end() != ifind && ifind->second ) {
        diff( ifind->second, pp );
        m_replaced.insert( ifind->second->particle() );
      }
      // put it into the map
      m_pythiaidmap[ID] = pp;
    }
    //
    return rebuild();
  }
  // =============================================================================
  /// Erase a property from all maps
  // =============================================================================
  namespace {
    template <class MAP>
    void _remove_( MAP& m, const ParticleProperty* pp ) {
      auto i = std::find_if( m.begin(), m.end(), [&]( typename MAP::const_reference i ) { return i.second == pp; } );
      if ( i != m.end() ) { m.erase( i ); }
    }
  } // namespace
  // ============================================================================
  StatusCode ParticlePropertySvc::erase( const ParticleProperty* pp ) {
    if ( !pp ) { return StatusCode::FAILURE; }

    _remove_( m_idmap, pp );
    _remove_( m_namemap, pp );
    _remove_( m_stdhepidmap, pp );
    _remove_( m_pythiaidmap, pp );
    //
    return rebuild();
  }
  // ============================================================================
  /// Parses the file and fill all the maps
  // ============================================================================
  StatusCode ParticlePropertySvc::parse() {

    // parse "the main" file
    StatusCode sc = parse( m_filename );
    if ( sc.isFailure() ) { return sc; }

    // parse "other" files
    for ( auto& file : m_other ) {
      sc = parse( file );
      if ( sc.isFailure() ) { return sc; }
    }

    // Now check that the file format was consistent with what parser
    // expected
    if ( m_namemap.empty() ) {
      error() << "Format of input file inconsistent with what expected"
              << " - Check you are using ParticleData.txt" << endmsg;
      return StatusCode::FAILURE;
    }

    return sc;
  }
  // ============================================================================
  StatusCode ParticlePropertySvc::parse( const std::string& file ) {
    auto infile = ( m_fileAccess ? m_fileAccess->open( file ) : nullptr );
    if ( !infile ) {
      error() << "Unable to open properties file : " << file << endmsg;
      return StatusCode::FAILURE;
    }

    StatusCode sc = StatusCode::SUCCESS;
    info() << "Opened particle properties file : " << file << endmsg;

    std::vector<std::string> tokens;
    tokens.reserve( 9 );
    std::string line;
    while ( std::getline( *infile, line ) ) {
      // parse each line of the file (comment lines begin with # in the cdf
      // file,
      if ( line.front() == '#' ) continue;

      tokens.clear();
      ba::trim_left_if( line, ba::is_space() );
      ba::split( tokens, line, ba::is_space(), boost::token_compress_on );
      if ( tokens.size() != 9 ) continue;

      auto gid = std::stoi( tokens[1] );
      auto jid = std::stoi( tokens[2] );
      // Change the particles that do not correspond to a pdg number
      if ( jid == 0 ) jid = 10000000 * gid;

      // add a particle property
      sc = push_back( tokens[0], gid, jid, std::stod( tokens[3] ), std::stod( tokens[4] ) * Gaudi::Units::GeV,
                      std::stod( tokens[5] ) * Gaudi::Units::s, tokens[6], std::stoi( tokens[7] ),
                      std::stod( tokens[8] ) * Gaudi::Units::GeV );

      if ( sc.isFailure() ) {
        error() << "Error from ParticlePropertySvc::push_back for particle='" << tokens[0] << "'" << endmsg;
      }
    }
    return StatusCode::SUCCESS;
  }
  // ============================================================================
  /** Helper (protected) function to find an antiparticle for the given
   *  particle ID (StdHepID)
   *  @param pp pointer to particle property
   *  @return pointer to antiparticle
   */
  // ============================================================================
  const ParticleProperty* ParticlePropertySvc::anti( const ParticleProperty* pp ) const {
    if ( !pp ) { return nullptr; }
    const int ID     = pp->pdgID();
    const int antiID = -1 * ID;
    for ( const auto& ap : m_vectpp ) {
      if ( ap && antiID == ap->pdgID() ) { return ap; } // RETURN
    };
    //
    return pp; // RETURN
  }
  // ============================================================================
  /** helper (protected) function to set the valid
   *  particle<-->antiparticle relations
   *  @return status code
   */
  // ============================================================================
  StatusCode ParticlePropertySvc::setAntiParticles() {
    // initialize particle<-->antiParticle relations
    for ( auto& pp : m_vectpp ) {
      if ( !pp ) { continue; } // CONTINUE
      const ParticleProperty* ap = anti( pp );
      if ( ap ) { pp->setAntiParticle( ap ); }
    }
    return StatusCode::SUCCESS;
  }
  // ============================================================================
  /// rebuild "the linear container" from the map
  // ============================================================================
  namespace {
    /// load mapped values from maps into set
    template <typename MAP, typename SET>
    void _load_( MAP& m, SET& result ) {
      for ( auto i = m.begin(); m.end() != i; ++i ) { result.insert( i->second ); }
    }
  } // namespace
  // ============================================================================
  StatusCode ParticlePropertySvc::rebuild() {
    std::set<mapped_type> local;
    m_vectpp.clear();
    m_vectpp.reserve( m_idmap.size() + 100 );
    // load information from maps into the set
    _load_( m_idmap, local );
    _load_( m_namemap, local );
    _load_( m_stdhepidmap, local );
    _load_( m_pythiaidmap, local );
    // load information from set to the linear container vector
    std::copy( std::begin( local ), std::end( local ), std::back_inserter( m_vectpp ) );
    return setAntiParticles();
  }
  // ============================================================================
  // treat additional particles
  // ============================================================================
  StatusCode ParticlePropertySvc::addParticles() {
    // loop over all "explicit" particles
    for ( const auto& item : m_particles ) {
      std::istringstream input( item );
      // get the name
      std::string p_name;
      int         p_geant;
      int         p_jetset;
      double      p_charge;
      double      p_mass;
      double      p_ltime;
      std::string p_evtgen;
      int         p_pythia;
      double      p_maxwid;
      if ( input >> p_name >> p_geant >> p_jetset >> p_charge >> p_mass >> p_ltime >> p_evtgen >> p_pythia >>
           p_maxwid ) {
        always() << " Add/Modify the particle: "
                 << " name='" << p_name << "'"
                 << " geant=" << p_geant << " jetset=" << p_jetset << " charge=" << p_charge << " mass=" << p_mass
                 << " ltime=" << p_ltime << " evtgen='" << p_evtgen << "'"
                 << " pythia=" << p_pythia << " maxwid=" << p_maxwid << endmsg;
        //
        StatusCode sc = push_back( p_name, p_geant, p_jetset, p_charge, p_mass * Gaudi::Units::GeV,
                                   p_ltime * Gaudi::Units::s, p_evtgen, p_pythia, p_maxwid * Gaudi::Units::GeV );
        if ( sc.isFailure() ) { return sc; } // RETURN
      } else {
        error() << " could not parse '" << item << "'" << endmsg;
        return StatusCode::FAILURE; // RETURN
      }
    }
    //
    return StatusCode::SUCCESS;
  }
// ============================================================================
#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//   The comparison are meant
#  pragma warning( push )
#  pragma warning( disable : 1572 )
#endif
  bool ParticlePropertySvc::diff( const ParticleProperty* o, const ParticleProperty* n, const MSG::Level l ) const {
    //
    if ( o == n ) { return false; }
    //
    auto& log = msgStream();
    log << l;
    if ( !o || !n ) {
      log << MSG::WARNING << " ParticleProperty* point to NULL" << endmsg;
      return true; // RETURN
    }
    //
    bool result = false;
    if ( o->particle() != n->particle() ) {
      result = true;
      log << " Name:'" << o->particle() << "'/'" << n->particle() << "'";
    }
    if ( o->geantID() != n->geantID() ) {
      result = true;
      log << " G3ID:" << o->geantID() << "/" << n->geantID() << "'";
    }
    if ( o->pdgID() != n->pdgID() ) {
      result = true;
      log << " PDGID:" << o->pdgID() << "/" << n->pdgID() << "'";
    }
    if ( o->pythiaID() != n->pythiaID() ) {
      result = true;
      log << " PYID:" << o->pythiaID() << "/" << n->pythiaID() << "'";
    }
    if ( o->charge() != n->charge() ) {
      result = true;
      log << " Q:" << o->charge() << "/" << n->charge() << "'";
    }
    if ( o->mass() != n->mass() ) {
      result = true;
      log << " M:" << o->mass() << "/" << n->mass() << "'";
    }
    if ( o->lifetime() != n->lifetime() ) {
      result = true;
      log << " T:" << o->lifetime() << "/" << n->lifetime() << "'";
    }
    if ( o->evtGenName() != n->evtGenName() ) {
      result = true;
      log << " EvtGen:" << o->evtGenName() << "/" << n->evtGenName() << "'";
    }
    if ( o->maxWidth() != n->maxWidth() ) {
      result = true;
      log << " WMAX:" << o->maxWidth() << "/" << n->maxWidth() << "'";
    }
    if ( result ) { log << endmsg; }
    //
    return result;
  }
} // namespace Gaudi
#ifdef __ICC
// re-enable icc remark #1572
#  pragma warning( pop )
#endif
// ============================================================================
// The END
// ============================================================================
