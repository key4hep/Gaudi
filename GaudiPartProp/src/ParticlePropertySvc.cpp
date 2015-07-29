// ============================================================================
// Include files
// ============================================================================
// STD&STL
// ============================================================================
#include <cstdlib>
#include <cstring>
#include <fstream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ParticleProperty.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/IFileAccess.h"
#include "GaudiKernel/System.h"
// ============================================================================
//#include "GaudiKernel/ToStream.h"
// ============================================================================
// Local
// ============================================================================
#include "ParticlePropertySvc.h"
// ============================================================================
namespace {
    //@FIXME/@TODO remove once we can assume C++14...
    template <typename T, class... Args>
    std::unique_ptr<T> make_unique_(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
namespace Gaudi {
/** Instantiation of a static factory class used by clients to create
 *  instances of this service
 */
DECLARE_COMPONENT(ParticlePropertySvc)
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
ParticlePropertySvc::ParticlePropertySvc
( const std::string& name ,
  ISvcLocator*       svc  )
  : base_class( name, svc )
{
  /// @todo: remove reference to LHCb-specific environment variable
  // Redefine the default name:
  if( System::getEnv("PARAMFILESROOT", m_filename) )
  {
    m_filename += "/data/ParticleTable.txt";
  }
  //
  declareProperty ( "ParticlePropertiesFile" , m_filename  ) ;
  declareProperty ( "OtherFiles"             , m_other     ) ;
  declareProperty ( "Particles"              , m_particles ) ;
}
// ============================================================================
/// initialize the service and setProperties
// ============================================================================
StatusCode ParticlePropertySvc::initialize()
{
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) { return sc ; }

  MsgStream log( msgSvc() , name() ) ;

  sc = setProperties();
  if ( sc.isFailure() )
  {
    log << MSG::ERROR << " Could not set the properties " << endmsg ;
    return sc ;
  }


  sc = service("VFSSvc",m_fileAccess);
  if ( sc.isFailure() )
  {
    log << MSG::ERROR << " Cannot retrieve the VFS service " << endmsg ;
    return sc ;
  }

  sc = parse();
  if ( sc.isFailure() )
  {
    log << MSG::ERROR << " Could not parse the file " << endmsg ;
    return sc ;
  }
  if ( !m_particles.empty() )
  {
    sc = addParticles () ;
    if ( sc.isFailure() )
    {
      log << MSG::ERROR << " Could not treat particles! " << endmsg ;
      return sc ;
    }
  }
  log << MSG::DEBUG << "ParticleProperties parsed successfully" << endmsg;

  log << MSG::DEBUG << "Access properties" << endmsg;
  // For debugging purposes print out the size of the internal maps
  // particle name as key: all particles in database are present here
  log << MSG::DEBUG << "NameMap size =" << m_namemap.size() << endmsg;
  // Geant3 ID as key: all particles in database are present here
  log << MSG::DEBUG << "GeantID Map size =" << m_idmap.size() << endmsg;
  // StdHep ID as key: some particles have no valid StdHep ID
  log << MSG::DEBUG << "StdHepID Map size =" << m_stdhepidmap.size()
      << endmsg;
  // Pythia ID as key: some particles are not defined in Pythia
  log << MSG::DEBUG << "PythiaID Map size =" << m_pythiaidmap.size()
      << endmsg;

  if ( !m_replaced.empty() )
  {
    log << MSG::INFO
        << "Properties have been redefined for "
        << m_replaced.size() << " particles : "
        << Gaudi::Utils::toString( m_replaced )
        << endmsg ;
  }

  return StatusCode::SUCCESS ;
}
// =============================================================================
/// finalize
// =============================================================================
StatusCode ParticlePropertySvc::finalize()
{
  if ( !m_other.empty() )
  {
    MsgStream log( msgSvc() , name() ) ;
    log << MSG::INFO
        << "Additional Properties have been read from files: "
        << Gaudi::Utils::toString ( m_other )
        << endmsg ;
  }

  if ( !m_replaced.empty() )
  {
    MsgStream log( msgSvc() , name() ) ;
    log << MSG::ALWAYS
        << "Properties have been redefined for "
        << m_replaced.size() << " particles : "
        << Gaudi::Utils::toString( m_replaced )
        << endmsg ;
  }

  if (m_fileAccess) {
    m_fileAccess->release();
    m_fileAccess = nullptr;
  }

  /// finalize the base class
  return Service::finalize () ;
}
// =============================================================================
/// Create a particle property and insert it in the maps
// =============================================================================
StatusCode ParticlePropertySvc::push_back
( const std::string& particle ,
  int                geantId  ,
  int                jetsetId ,
  double             charge   ,
  double             mass     ,
  double             tlife    ,
  const std::string& evtName  ,
  int                pythiaId ,
  double             maxWidth )
{
  //
  auto i = m_owned.insert(  
    make_unique_<ParticleProperty>( particle , geantId  , jetsetId ,
                                    charge   , mass     , tlife    ,
                                    evtName  , pythiaId , maxWidth ) 
  ) ;
  //
  return i.second ? push_back( i.first->get() ) : StatusCode::FAILURE;
}
// =============================================================================
/// Add a particle property.
// =============================================================================
StatusCode ParticlePropertySvc::push_back ( ParticleProperty* pp )
{
  if ( !pp ) { return StatusCode::FAILURE ; }
  //
  { // try to add into Geant(3)ID map
    const int ID = pp->geantID() ;
    // is this already in the map?
    auto ifind = m_idmap.find( ID ) ;
    if ( m_idmap.end() != ifind && 0 != m_idmap[ ID ])
    {
      diff ( ifind->second , pp ) ;
      m_replaced.insert( m_idmap[ ID ]->particle() ) ;
    }
    // put it into the map
    m_idmap[ ID ] = pp ;
  }
  //
  { // try to add into Name map
    const std::string& particle = pp->particle() ;
    // is this already in the map?
    auto ifind = m_namemap.find( particle ) ;
    if ( m_namemap.end() != ifind && 0 != m_namemap[ particle ] )
    {
      diff ( ifind->second , pp ) ;
      m_replaced.insert( m_namemap[ particle ]->particle() ) ;
    }
    // put it into the map
    m_namemap[ particle ] = pp ;
  }
  //
  // add to StdHep map only if StdHep ID different from zero and if
  // not Cerenkov (StdHep ID = gamma)
  if ( 0 != pp->jetsetID() && "Tcherenkov" != pp->particle() )
  { // try to add into StdHepID map
    const int ID = pp->jetsetID() ;
    // is this already in the map?
    auto ifind = m_stdhepidmap.find( ID ) ;
    if ( m_stdhepidmap.end() != ifind && 0 != m_stdhepidmap[ ID ])
    {
      diff ( ifind->second , pp ) ;
      m_replaced.insert( m_stdhepidmap[ ID ]->particle() ) ;
    }
    // put it into the map
    m_stdhepidmap[ ID ] = pp ;
  }
  //
  // add to Pythia map only if Pythia ID is different from
  // zero ( StdHep id is always different from zero in this case )
  if ( 0 != pp->pythiaID() &&
       0 != pp->jetsetID() &&
       "Tcherenkov" != pp->particle() )
  { // try to add into PythiaID map
    const int ID = pp->pythiaID() ;
    // is this already in the map?
    auto ifind = m_pythiaidmap.find( ID ) ;
    if ( m_pythiaidmap.end() != ifind && 0 != m_pythiaidmap[ ID ])
    {
      diff ( ifind->second , pp ) ;
      m_replaced.insert( m_pythiaidmap[ ID ]->particle() ) ;
    }
    // put it into the map
    m_pythiaidmap[ ID ] = pp ;
  }
  //
  return rebuild() ;
}
// =============================================================================
/// Erase a property from all maps
// =============================================================================
namespace
{
  template <class MAP>
  void _remove_ ( MAP& m , const ParticleProperty* pp )
  {
    auto i = std::find_if( m.begin(), m.end(), 
                           [&](typename MAP::const_reference i) { return i.second == pp; } );
    if  ( i != m.end() ) { m.erase ( i ) ; }
  }
}
// ============================================================================
StatusCode ParticlePropertySvc::erase( const ParticleProperty* pp )
{
  if ( !pp ) { return StatusCode::FAILURE ; }

  _remove_ ( m_idmap       , pp ) ;
  _remove_ ( m_namemap     , pp ) ;
  _remove_ ( m_stdhepidmap , pp ) ;
  _remove_ ( m_pythiaidmap , pp ) ;
  //
  return rebuild() ;
}
// ============================================================================
/// Parses the file and fill all the maps
// ============================================================================
StatusCode ParticlePropertySvc::parse()
{

  // parse "the main" file
  StatusCode sc = parse ( m_filename ) ;
  if ( sc.isFailure() ) { return sc ; }

  // parse "other" files
  for ( auto& file : m_other )
  {
    sc = parse ( file ) ;
    if ( sc.isFailure() ) { return sc ; }
  }

  // Now check that the file format was consistent with what parser
  // expected
  if ( m_namemap.empty() )
  {
    MsgStream log( msgSvc(), name() );
    log << MSG::ERROR
        << "Format of input file inconsistent with what expected"
        << " - Check you are using ParticleData.txt" << endmsg;
    return StatusCode::FAILURE;
  }

  return sc;
}
// ============================================================================
StatusCode ParticlePropertySvc::parse( const std::string& file )
{
  StatusCode sc = StatusCode::FAILURE;

  MsgStream log( msgSvc(), name() );
  char line[ 255 ];

  std::unique_ptr<std::istream> infile;
  if (m_fileAccess) infile = m_fileAccess->open(file);

  if ( !infile )
  {
    log << MSG::ERROR << "Unable to open properties file : " << file
        << endmsg;
    return StatusCode::FAILURE ;
  }

  sc = StatusCode::SUCCESS;
  log << MSG::INFO
      << "Opened particle properties file : " << file << endmsg;

  while( *infile )
  {
    // parse each line of the file (comment lines begin with # in the cdf
    // file,
    infile->getline( line, 255 );

    if ( line[0] == '#' ) continue;

    /// @todo: This PPS should be removed from Gaudi, if not, the parser must be improved
#ifdef WIN32
// Disable warning
//   C4996: 'strtok': This function or variable may be unsafe.
#pragma warning(disable:4996)
#endif
    std::string par, gid, jid, chg, mas, lif, evt, pyt, mwi ;
    char* token = strtok( line, " " );
    if ( token ) { par = token; token = strtok( nullptr, " " );} else continue;
    if ( token ) { gid = token; token = strtok( nullptr, " " );} else continue;
    if ( token ) { jid = token; token = strtok( nullptr, " " );} else continue;
    if ( token ) { chg = token; token = strtok( nullptr, " " );} else continue;
    if ( token ) { mas = token; token = strtok( nullptr, " " );} else continue;
    if ( token ) { lif = token; token = strtok( nullptr, " " );} else continue;
    if ( token ) { evt = token; token = strtok( nullptr, " " );} else continue;
    if ( token ) { pyt = token; token = strtok( nullptr, " " );} else continue;
    if ( token ) { mwi = token; token = strtok( nullptr, " " );} else continue;
    if ( token != nullptr ) continue;

    // In SICb cdf file mass and lifetime units are GeV and sec, specify it so
    // that they are converted to Gaudi units (MeV and ns)
    double mass = std::stod( mas ) * Gaudi::Units::GeV;
    double tlife = std::stod( lif ) * Gaudi::Units::s;
    long   ljid = std::stoi( jid );
    long   lgid = std::stoi( gid );
    long   lpyt = std::stoi( pyt ) ;
    double mW = std::stod( mwi ) * Gaudi::Units::GeV ;

    // Change the particles that do not correspond to a pdg number
    if ( ljid == 0 ) {
      ljid = 10000000*lgid;
    }

    // add a particle property
    sc = push_back( par, lgid, ljid,
                    std::stod( chg ), mass, tlife, evt, lpyt, mW ) ;
    if ( sc.isFailure() )
    {
      log << MSG::ERROR
          << "Error from ParticlePropertySvc::push_back for particle='"
          << par << "'" << endmsg ;
    }

  }

  return StatusCode::SUCCESS ;
}
// ============================================================================
/** Helper (protected) function to find an antiparticle for the given
 *  particle ID (StdHepID)
 *  @param pp pointer to particle property
 *  @return pointer to antiparticle
 */
// ============================================================================
const ParticleProperty*
ParticlePropertySvc::anti ( const ParticleProperty* pp ) const
{
  if ( !pp ) { return nullptr ; }
  const int     ID = pp->pdgID() ;
  const int antiID = -1 * ID     ;
  for ( const auto& ap : m_vectpp ) 
  {
    if ( ap && antiID == ap->pdgID() ) { return ap ; }          // RETURN
  };
  //
  return pp ;                                                   // RETURN
}
// ============================================================================
/** helper (protected) function to set the valid
 *  particle<-->antiparticle relations
 *  @return status code
 */
// ============================================================================
StatusCode ParticlePropertySvc::setAntiParticles()
{
  // initialize particle<-->antiParticle relations
  for ( auto& pp : m_vectpp )
  {
    if ( !pp                    ) { continue ; }   // CONTINUE
    const ParticleProperty* ap = anti ( pp ) ;
    if ( ap                    ) { pp->setAntiParticle( ap ) ; }
  }
  return StatusCode::SUCCESS ;
}
// ============================================================================
/// rebuild "the linear container" from the map
// ============================================================================
namespace
{
  /// load mapped values from maps into set
  template <typename MAP, typename SET>
  void _load_ ( MAP& m , SET& result )
  {
    for ( auto i = m.begin() ; m.end() != i ; ++i )
    { result.insert ( i->second ); }
  }
}
// ============================================================================
StatusCode ParticlePropertySvc::rebuild()
{
  std::set<mapped_type> local ;
  m_vectpp.clear() ;
  m_vectpp.reserve ( m_idmap.size() + 100 ) ;
  // load information from maps into the set
  _load_ ( m_idmap       , local ) ;
  _load_ ( m_namemap     , local ) ;
  _load_ ( m_stdhepidmap , local ) ;
  _load_ ( m_pythiaidmap , local ) ;
  // load information from set to the linear container vector
  std::copy( std::begin(local), std::end(local), std::back_inserter(m_vectpp) );
  return setAntiParticles() ;
}
// ============================================================================
// treat additional particles
// ============================================================================
StatusCode ParticlePropertySvc::addParticles()
{

  MsgStream log ( msgSvc() , name() ) ;
  // loop over all "explicit" particles
  for ( const auto& item : m_particles )
  {
    std::istringstream input( item ) ;
    // get the name
    std::string p_name   ;
    int         p_geant  ;
    int         p_jetset ;
    double      p_charge ;
    double      p_mass   ;
    double      p_ltime  ;
    std::string p_evtgen ;
    int         p_pythia ;
    double      p_maxwid ;
    if ( input
         >> p_name
         >> p_geant
         >> p_jetset
         >> p_charge
         >> p_mass
         >> p_ltime
         >> p_evtgen
         >> p_pythia
         >> p_maxwid )
    {
      log << MSG::ALWAYS
          << " Add/Modify the particle: "
          << " name='"   << p_name   << "'"
          << " geant="   << p_geant
          << " jetset="  << p_jetset
          << " charge="  << p_charge
          << " mass="    << p_mass
          << " ltime="   << p_ltime
          << " evtgen='" << p_evtgen << "'"
          << " pythia="  << p_pythia
          << " maxwid="  << p_maxwid << endmsg ;
      //
      StatusCode sc = push_back
        ( p_name                        ,
          p_geant                       ,
          p_jetset                      ,
          p_charge                      ,
          p_mass    * Gaudi::Units::GeV ,
          p_ltime   * Gaudi::Units::s   ,
          p_evtgen                      ,
          p_pythia                      ,
          p_maxwid  * Gaudi::Units::GeV ) ;
      if ( sc.isFailure() ) { return sc ; }                        // RETURN
    }
    else
    {
      log << MSG::ERROR
          << " could not parse '" << item << "'" << endmsg ;
      return StatusCode::FAILURE ;                                 // RETURN
    }
  }
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//   The comparison are meant
#pragma warning(push)
#pragma warning(disable:1572)
#endif
bool ParticlePropertySvc::diff
( const ParticleProperty* o ,
  const ParticleProperty* n ,
  const MSG::Level        l ) const
{
  //
  if ( o == n ) { return false ; }
  //
  MsgStream log ( msgSvc() , name() ) ;
  log << l ;
  if ( !o || !n  )
  {
    log << MSG::WARNING << " ParticleProperty* point to NULL" << endmsg ;
    return true ;                                                    // RETURN
  }
  //
  bool result = false ;
  if ( o -> particle () != n -> particle () )
  {
    result = true ;
    log << " Name:'"  << o -> particle () << "'/'" << n -> particle () << "'" ;
  }
  if ( o -> geantID  () != n -> geantID  () )
  {
    result = true ;
    log << " G3ID:"   << o -> geantID  () << "/"   << n -> geantID  () << "'" ;
  }
  if ( o -> pdgID    () != n -> pdgID    () )
  {
    result = true ;
    log << " PDGID:"  << o -> pdgID    () << "/"   << n -> pdgID    () << "'" ;
  }
  if ( o -> pythiaID () != n -> pythiaID () )
  {
    result = true ;
    log << " PYID:"   << o -> pythiaID () << "/"   << n -> pythiaID () << "'" ;
  }
  if ( o -> charge   () != n -> charge   () )
  {
    result = true ;
    log << " Q:"      << o -> charge   () << "/"   << n -> charge   () << "'" ;
  }
  if ( o -> mass     () != n -> mass     () )
  {
    result = true ;
    log << " M:"      << o -> mass     () << "/"   << n -> mass     () << "'" ;
  }
  if ( o -> lifetime () != n -> lifetime () )
  {
    result = true ;
    log << " T:"      << o -> lifetime () << "/"   << n -> lifetime () << "'" ;
  }
  if ( o -> evtGenName () != n -> evtGenName () )
  {
    result = true ;
    log << " EvtGen:" << o -> evtGenName () << "/"   << n -> evtGenName () << "'" ;
  }
  if ( o -> maxWidth () != n -> maxWidth () )
  {
    result = true ;
    log << " WMAX:"   << o -> maxWidth () << "/"   << n -> maxWidth () << "'" ;
  }
  if ( result ) { log << endmsg ; }
  //
  return result ;
}

}
#ifdef __ICC
// re-enable icc remark #1572
#pragma warning(pop)
#endif
// ============================================================================
// The END
// ============================================================================
