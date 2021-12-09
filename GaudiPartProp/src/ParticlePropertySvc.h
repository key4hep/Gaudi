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
#ifndef PARTICLEPROPERTYSVC_PARTICLEPROPERTYSVC_H
#define PARTICLEPROPERTYSVC_PARTICLEPROPERTYSVC_H
// ============================================================================
// Include Files
// ============================================================================
// STD&STL
// ============================================================================
#include <deque>
#include <map>
#include <set>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IParticlePropertySvc.h"
#include "GaudiKernel/Service.h"

class IFileAccess;

namespace Gaudi {
  // ============================================================================
  /** @class ParticlePropertySvc ParticlePropertySvc.h
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
   *  8/11/1999 change default file from
   *  $CDF/particle.cdf to $LHCBDBASE/cdf/particle.cdf on Unix
   *  and use environment variable also on WinNT
   *  Also introduce finding of particle by StdHepID
   *
   *  @author Vanya BELYAEV
   *  @date 2006-09-22
   *  1) Modified to add possibility to redefine properties of existing
   *  particles and to read additional files.
   *  New property "OtherFiles" (default is empty vector) is introduces.
   *  Service parsed additional files after the main one.
   *
   *    @code
   *
   *    // read additional properties
   *    ParticlePropertySvc.OtherFiles = {
   *       "$SOMELOCATION1/file1.txt" ,
   *       "$SOMELOCATION2/file2.txt"
   *    } ;
   *
   *    @endcode
   *
   *  2) Add possibility to modify only certain particles through the
   *  new property "Particles"  (default is empty list),
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
  class ParticlePropertySvc : public extends<Service, IParticlePropertySvc> {
  public:
    // typedefs for container type
    typedef ParticleProperty*                  mapped_type;
    typedef std::map<int, mapped_type>         MapID;
    typedef std::map<std::string, mapped_type> MapName;
    typedef std::map<int, mapped_type>         MapStdHepID;
    typedef std::map<int, mapped_type>         MapPythiaID;
    using IParticlePropertySvc::const_iterator;
    using IParticlePropertySvc::iterator;
    using IParticlePropertySvc::VectPP;
    // Inherited Service overrides:

    /// Initialise the service.
    StatusCode initialize() override;
    /// Finalise the service.
    StatusCode finalize() override;
    /** Create a new particle property.
     *  @attention ParticlePropertySvc *IS* the owner if this property!
     *  @paramparticle String name of the particle.
     *  @param geantId Geant3 ID of the particle.
     *  @param jetsetId Jetset ID of the particle.
     *  @param charge Particle charge (/e).
     *  @param mass Particle mass (GeV).
     *  @param tlife Particle lifetime (s).
     *  @param evtName String name of the particle in EvtGen.
     *  @param pythiaId Pythia ID of the particle.
     *  @return StatusCode - SUCCESS if the particle property was added.
     */
    StatusCode push_back( const std::string& particle, int geantId, int jetsetId, double charge, double mass,
                          double tlife, const std::string& evtName, int pythiaId, double maxWidth ) override;
    /** Add a new particle property.
     *  @attention ParticlePropertySvc is not the owner of this property!
     *  @param pp, a particle property class.
     *  @return StatusCode - SUCCESS if the particle property was added.
     */
    StatusCode push_back( ParticleProperty* pp ) override;
    /// Get a const reference to the beginning of the container.
    const_iterator begin() const override { return m_vectpp.begin(); }
    /// Get a const reference to the end of the container.
    const_iterator end() const override { return m_vectpp.end(); }
    /// Get the container size.
    int size() const override { return m_vectpp.size(); };
    /// Retrieve a property by geant3 id.
    ParticleProperty* find( int geantId ) override { return m_idmap[geantId]; }
    /// Retrieve a property by particle name.
    ParticleProperty* find( const std::string& name ) override { return m_namemap[name]; }
    /// Retrieve a property by StdHep id
    ParticleProperty* findByStdHepID( int stdHepId ) override { return m_stdhepidmap[stdHepId]; }
    /// Retrieve a property by Pythia id
    ParticleProperty* findByPythiaID( int pythiaID ) override { return m_pythiaidmap[pythiaID]; }
    /// Erase a property by geant3 id.
    StatusCode erase( int geantId ) override { return erase( find( geantId ) ); }
    /// Erase a property by particle name.
    StatusCode erase( const std::string& name ) override { return erase( find( name ) ); }
    /// Erase a property by StdHep id ???
    StatusCode eraseByStdHepID( int stdHepId ) override { return erase( findByStdHepID( stdHepId ) ); }
    /** Standard Constructor.
     *  @param  name   String with service name
     *  @param  svc    Pointer to service locator interface
     */
    ParticlePropertySvc( const std::string& name, ISvcLocator* svc );

  protected:
    /** helper (protected) function to
     *  find an antiparticle for the given particle ID (StdHepID)
     *  @param pp pointer to particle property
     *  @return pointer to antiparticle
     */
    const ParticleProperty* anti( const ParticleProperty* pp ) const;
    /** helper (protected) function to set the valid
     *  particle<-->antiparticle relations
     *  @return status code
     */
    StatusCode setAntiParticles();
    /// rebuild "the linear container" from the map
    StatusCode rebuild();
    /// remove particle property from all maps
    StatusCode erase( const ParticleProperty* pp );
    /// parse the file
    StatusCode parse( const std::string& file );
    // Text file parsing routine.
    StatusCode parse();
    // treat additional particles
    StatusCode addParticles();
    // compare 2 entries
    bool diff( const ParticleProperty* o, const ParticleProperty* n, const MSG::Level l = MSG::DEBUG ) const;

  private:
    Gaudi::Property<std::string>              m_filename{ this, "ParticlePropertiesFile", "ParticleTable.txt",
                                             "Filename of the particle properties file" };
    Gaudi::Property<std::vector<std::string>> m_other{ this, "OtherFiles", {}, "additional file names" };
    Gaudi::Property<std::vector<std::string>> m_particles{
        this, "Particles", {}, "properties to be redefined explicitly" };

    VectPP      m_vectpp;      ///< Vector of all particle properties
    MapID       m_idmap;       ///< Map for geant IDs
    MapName     m_namemap;     ///< Map for particle names
    MapStdHepID m_stdhepidmap; ///< Map for StdHep Ids
    MapPythiaID m_pythiaidmap; ///< Map for Pythia Ids

    // local storage of ALL properties
    std::deque<ParticleProperty> m_owned;
    std::set<std::string>        m_replaced;

    SmartIF<IFileAccess> m_fileAccess;
  };
} // namespace Gaudi
// =============================================================================
// The END
// =============================================================================
#endif
