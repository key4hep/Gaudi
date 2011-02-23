// $Id: ParticleProperty.h,v 1.6 2004/08/05 09:58:08 mato Exp $
#ifndef GAUDIKERNEL_PARTICLEPROPERTY_H
#define GAUDIKERNEL_PARTICLEPROPERTY_H 1
// ============================================================================
// Include files
// ============================================================================
#include <string>
#include <ostream>
#include <iomanip>
// ============================================================================

/** @class ParticleProperty ParticleProperty.h GaudiKernel/ParticleProperty.h
 *
 *  A trivial class to hold information about a single particle properties. 
 *  All particle properties are accessible through accessor functions
 *
 *  @author  Iain Last,G.Corti
 */
class ParticleProperty {
public:
  /// Constructors
  ParticleProperty() {}
  
  ParticleProperty
  ( const std::string& particle   , 
    int                geantId    , 
    int                jetsetId   , 
    double             charge     , 
    double             mass       , 
    double             tlife      , 
    const std::string& evtgenName ,
    int                pythiaId   , 
    double             maxWidth   ) 
    : m_name         ( particle   )
    , m_idgeant      ( geantId    )
    , m_idjetset     ( jetsetId   )
    , m_charge       ( charge     )
    , m_mass         ( mass       )
    , m_tlife        ( tlife      )
    , m_evtgenName   ( evtgenName )
    , m_pythiaId     ( pythiaId   )
    , m_maxWidth     ( maxWidth   ) 
    , m_anti         ( 0          ) 
  {}
  
  /// Destructor.
  ~ParticleProperty() {}

  /// Get the particle name.
  const std::string& particle() const { return m_name; }

  /// Set the particle name.
  void setParticle( const std::string& particle ) { m_name = particle; }

  /// Get the GEANT3 ID.
  int geantID() const { return m_idgeant; }

  /// Set the GEANT3 ID.
  void setGeantID( int id ) { m_idgeant = id; }

  /// Get the PDG (= JETSET) ID
  int pdgID() const { return m_idjetset; }

  /// Set the PDG (= JETSET) ID
  void setPdgID( int id) { m_idjetset = id; }

  /// Get the JETSET(StdHep) ID.
  int jetsetID() const { return m_idjetset; }

  /// Set the JETSET(StdHep) ID.
  void setJetsetID( int id ) { m_idjetset = id; }

  /// Get the particle charge.
  double charge() const { return m_charge; }

  /// Set the particle charge.
  void setCharge( double q ) { m_charge = q; }
  
  /// Get the particle mass.
  double mass() const { return m_mass; }

  /// Set the particle charge.
  void setMass( double m ) { m_mass = m; }
  
  /// Get the particle lifetime.
  double lifetime() const { return m_tlife; }

  /// Set the particle lifetime.
  void setLifetime( double t ) { m_tlife = t; }

  /// Get the EvtGen name
  const std::string& evtGenName() const { return m_evtgenName ; }

  /// Set the EvtGen name
  void setEvtGenName( const std::string & name ) { m_evtgenName = name ; }
  
  /// Get the Pythia ID
  int pythiaID() const { return m_pythiaId ; }
  
  /// Set the Pythia ID
  void setPythiaID( int pId ) { m_pythiaId = pId ; }

  /// Get the max width deviation
  double maxWidth() const { return m_maxWidth ; }
  
  /// Set the max width deviation
  void setMaxWidth( double mW ) { m_maxWidth = mW ; }
  
  /// get the pointer to the antiparticle
  const ParticleProperty* antiParticle() const { return m_anti ; }
  /// set the pointer to the antiparticle
  void setAntiParticle( const ParticleProperty* p ) { m_anti = p ; }
  
  friend std::ostream& operator<< 
    ( std::ostream& stream, const ParticleProperty& pp)
  {
    stream << "Name : " << pp.m_name 
           << ", Geant ID : " << pp.m_idgeant
           << ", JetSet ID : " << pp.m_idjetset
           << ", Charge (/e): " << pp.m_charge
           << ", Mass (MeV): " << pp.m_mass
           << ", Lifetime (ns): " << pp.m_tlife
           << ", EvtGen Name: " << pp.m_evtgenName
           << ", Pythia ID: " << pp.m_pythiaId
           << ", Max width deviation (MeV): " << pp.m_maxWidth ;
    return stream;
  }
  
private:
  /// The particle name.
  std::string m_name;

  /// The GEANT ID.
  int m_idgeant;
  
  /// The Jetset ID.
  int m_idjetset;

  /// The charge.
  double m_charge;

  /// The mass.
  double m_mass;

  /// The lifetime.
  double m_tlife;

  /// The EvtGen Name
  std::string m_evtgenName ;
  
  /// The Pythia ID
  int m_pythiaId ;

  /// The maximum width deviation
  double m_maxWidth ;
  
  /// the antiparticle 
  const ParticleProperty* m_anti ;
};

#endif

