/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Interfaces/IParticlePropertySvc.h>
#include <Gaudi/ParticleID.h>
#include <Gaudi/ParticleProperty.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <GaudiKernel/ToStream.h>
#include <boost/format.hpp>
#include <ostream>

/** @file
 *  Implementation file for class Gaudi:::ParticleProperty
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-08-03
 */
/*  standard output operator to the stream
 *  @param stream the stream
 *  @param pp    the particle property object
 *  @return the stream
 */
std::ostream& operator<<( std::ostream& stream, const Gaudi::ParticleProperty& pp ) { return pp.fillStream( stream ); }
/*  full constructor, from all data (except the antiparticle )
 *  @param name       the name for the particle
 *  @param pid        the PID for the particle
 *  @param charge     the charge of the particle
 *  @param mass       the nominal mass of the particle
 *  @param tlife      the nominal lifetime of the particle
 *  @param evtGen     the name of particle in EvtGen program
 *  @param pythia     the ID for the particle used in Pythia generator
 *  @param maxWidth   the maximal width of the particle (used in generator)
 */
Gaudi::ParticleProperty::ParticleProperty( const std::string& name, const Gaudi::ParticleID& pid, const double charge,
                                           const double mass, const double tlife, const double maxWidth,
                                           const std::string& evtgen, const int pythia )
    : m_name( name )
    , m_pid( pid )
    , m_charge( charge )
    , m_mass( mass )
    , m_tlife( tlife )
    , m_maxWidth( maxWidth )
    , m_evtgen( evtgen )
    , m_pythia( pythia )
    , m_anti( 0 ) {}
/*  A bit simplified constructor, from all data (except the antiparticle )
 *
 *    - "evtgen" is set from the regular "name"
 *    - "pythia" is set from the regular PID
 *
 *  @param name       the name for the particle
 *  @param pid        the PID for the particle
 *  @param charge     the charge of the particle
 *  @param mass       the nominal mass of the particle
 *  @param tlife      the nominal lifetime of the particle
 *  @param maxWidth   the maximal width of the particle (used in generator)
 */
Gaudi::ParticleProperty::ParticleProperty( const std::string& name, const Gaudi::ParticleID& pid, const double charge,
                                           const double mass, const double tlife, const double maxWidth )
    : m_name( name )
    , m_pid( pid )
    , m_charge( charge )
    , m_mass( mass )
    , m_tlife( tlife )
    , m_maxWidth( maxWidth )
    , m_evtgen( name )
    , m_pythia( pid.pid() )
    , m_anti( 0 ) {}
/*  the standard (a'la Gaudi) printout of the object
 *  @param s reference to the output stream
 *  @return reference to the output stream
 */
std::ostream& Gaudi::ParticleProperty::fillStream( std::ostream& s ) const {
  //
  typedef boost::format BF;
  //
  // name & code
  s << BF( "%1$-14s %|16t| PDG:%2$9d," ) % name() % particleID().pid();
#ifdef __INTEL_COMPILER             // Disable ICC remark
#  pragma warning( disable : 1572 ) // Floating-point equality and inequality comparisons are unreliable
#  pragma warning( push )
#endif
  // charge
  if ( 0 == charge() ) {
    s << " Q:   0";
  } else if ( 1.0 == charge() ) {
    s << " Q:  +1";
  } else if ( -1.0 == charge() ) {
    s << " Q:  -1";
  } else if ( 2.0 == charge() ) {
    s << " Q:  +2";
  } else if ( -2.0 == charge() ) {
    s << " Q:  -2";
  } else if ( 0.3 == charge() ) {
    s << " Q:+1/3";
  } else if ( -0.3 == charge() ) {
    s << " Q:-1/3";
  } else if ( 0.7 == charge() ) {
    s << " Q:+2/3";
  } else if ( -0.7 == charge() ) {
    s << " Q:-2/3";
  } else {
    s << BF( " Q:%|+3.1f|," ) % float( charge() );
  }
#ifdef __INTEL_COMPILER // Re-enable ICC remark
#  pragma warning( pop )
#endif
  // mass
  if ( mass() < 1 * Gaudi::Units::MeV ) {
    s << BF( ", mass:%|9.6g| keV" ) % ( mass() / Gaudi::Units::keV );
  } else if ( mass() < 1 * Gaudi::Units::GeV ) {
    s << BF( ", mass:%|9.6g| MeV" ) % ( mass() / Gaudi::Units::MeV );
  } else {
    s << BF( ", mass:%|9.6g| GeV" ) % ( mass() / Gaudi::Units::GeV );
  }
  // lifetime
  if ( 1 * Gaudi::Units::km < ctau() ) {
    s << ", lifetime: infinity  ";
  } else if ( 0 == lifeTime() ) {
  } // ATTENTION!!
  else if ( 1 * Gaudi::Units::meter < ctau() ) {
    s << BF( ",  ctau:%|12.6g| m" ) % ( ctau() / Gaudi::Units::meter );
  } else if ( 1 * Gaudi::Units::cm < ctau() ) {
    s << BF( ",  ctau:%|11.6g| cm" ) % ( ctau() / Gaudi::Units::cm );
  } else if ( 0.5 * Gaudi::Units::mm < ctau() ) {
    s << BF( ",  ctau:%|11.6g| mm" ) % ( ctau() / Gaudi::Units::mm );
  } else if ( 0.1 * Gaudi::Units::micrometer < ctau() ) {
    s << BF( ",  ctau:%|11.6g| um" ) % ( ctau() / Gaudi::Units::micrometer );
  } else if ( 1 * Gaudi::Units::GeV <= width() ) {
    s << BF( ", width:%|10.6g| GeV" ) % ( width() / Gaudi::Units::GeV );
  } else if ( 1 * Gaudi::Units::MeV <= width() ) {
    s << BF( ", width:%|10.6g| MeV" ) % ( width() / Gaudi::Units::MeV );
  } else if ( 1 * Gaudi::Units::keV <= width() ) {
    s << BF( ", width:%|10.6g| keV" ) % ( width() / Gaudi::Units::keV );
  } else if ( 1 * Gaudi::Units::eV <= width() ) {
    s << BF( ", width:%|10.6g| eV" ) % ( width() / Gaudi::Units::eV );
  } else {
    s << BF( ", ltime:%|12.6g| ns" ) % ( lifetime() / Gaudi::Units::ns );
  }
  // Evt Gen name
  if ( evtGen() != name() ) { s << BF( ", EvtGen: %|-10s|" ) % evtGen(); }
  // pythia ID
  if ( m_pid.pid() != pythia() ) { s << BF( ", Pythia: %|-9d|" ) % pythia(); }
  return s;
}
// simple method for conversion into string
std::string Gaudi::ParticleProperty::toString() const {
  std::ostringstream s;
  fillStream( s );
  return s.str();
}
/*  set the pointer to the antipartice
 *  @attention it is the only one "setter"
 *  @param p pointer to anti-particle
 */
void Gaudi::ParticleProperty::setAntiParticle( const ParticleProperty* p ) { m_anti = p; }

/*  print a list of properties in a form of the table
 *  @see Gaudi::ParticleProperty
 *  @param particles the list of particle properties
 *  @param stream  the reference to the output stream
 *  @param service the service to extract global information
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date  2008-08-03
 */
std::ostream& Gaudi::ParticleProperties::printAsTable_( const std::vector<const Gaudi::ParticleProperty*>& particles,
                                                        std::ostream&                                      stream,
                                                        const Gaudi::Interfaces::IParticlePropertySvc*     service ) {
  std::string fmt, hdr;
  //
  if ( 0 != service ) {
    fmt += " | %|-4.4s| "; // index
    hdr += " | %|-4.4s| "; // index
  }
  //
  fmt += "| %|-18.18s|";  // the name
  fmt += "| %|12d| ";     // PDGID
  fmt += "| %|=4s| ";     // charge
  fmt += "| %|13.8g|";    // mass
  fmt += " %|-3s| ";      // unit
  fmt += "| %|13.8g|";    // lifetime/gamma/ctau
  fmt += " %|-3s| ";      // unit
  fmt += "| %|=10.4g| ";  // maxwidth
  fmt += "| %|=20.20s|";  // evtgen name
  fmt += " | %|=10d| |";  // pythia ID
  fmt += " %|=20.20d| |"; // antiparticle
  //
  hdr += "| %|=18.18s|";  // the name
  hdr += "| %|=12.12s| "; // PDGID
  hdr += "| %|=4.4s| ";   // charge
  hdr += "| %|=18.18s|";  // mass
  // hdr +=   "%|=5.5s|"     ;  // unit
  hdr += "| %|=18.18s|"; // lifetime/gamma/ctau
  // hdr +=   "%|=5.5s|"     ;  // unit
  hdr += "| %|=10.10g| ";   // maxwidth
  hdr += "| %|=20.20s|";    // evtgen name
  hdr += " | %|=10.10s| |"; // pythia ID
  hdr += " %|=20.20d| |";   // antiparticle

  boost::format header( hdr );
  header.exceptions( boost::io::all_error_bits ^ ( boost::io::too_many_args_bit | boost::io::too_few_args_bit ) );

  if ( service ) { header % "#"; }
  header % "Name" % "PdgID" % "Q" % "Mass" % "(c*)Tau/Gamma" % "MaxWidth" % "EvtGen" % "PythiaID" % "Antiparticle";

  const std::string hdrLine = header.str();
  const std::string dashes  = " " + std::string( hdrLine.size() - 1, '-' );
  stream << dashes << std::endl << hdrLine << std::endl << dashes << std::endl;

  for ( auto ip = particles.begin(); particles.end() != ip; ++ip ) {
    const Gaudi::ParticleProperty* pp = *ip;
    if ( 0 == pp ) { continue; }
    boost::format line( fmt );
    line.exceptions( boost::io::all_error_bits ^ ( boost::io::too_many_args_bit | boost::io::too_few_args_bit ) );
    //
    if ( 0 != service ) {
      size_t index = Gaudi::ParticleProperties::index( pp, service );
      if ( 0 == index ) {
        line % "?";
      } else {
        line % index;
      }
    }
    //
    line % pp->name()               // name
        % pp->particleID().pid();   // PDG-ID
                                    //
#ifdef __INTEL_COMPILER             // Disable ICC remark
#  pragma warning( disable : 1572 ) // Floating-point equality and inequality comparisons are unreliable
#  pragma warning( push )
#endif
    if ( 0 == pp->charge() ) {
      line % "0";
    } else if ( 0.3 == pp->charge() ) {
      line % "+1/3";
    } else if ( -0.3 == pp->charge() ) {
      line % "-1/3";
    } else if ( 0.7 == pp->charge() ) {
      line % "+2/3";
    } else if ( -0.7 == pp->charge() ) {
      line % "-2/3";
    } else {
      line % pp->charge();
    }
#ifdef __INTEL_COMPILER // Re-enable ICC remark
#  pragma warning( pop )
#endif
    //
    // mass
    if ( pp->mass() < 1 * Gaudi::Units::keV ) {
      line % ( pp->mass() / Gaudi::Units::eV ) % "eV";
    } else if ( pp->mass() < 1 * Gaudi::Units::MeV ) {
      line % ( pp->mass() / Gaudi::Units::keV ) % "keV";
    } else if ( pp->mass() < 1 * Gaudi::Units::GeV ) {
      line % ( pp->mass() / Gaudi::Units::MeV ) % "MeV";
    } else if ( pp->mass() < 1 * Gaudi::Units::TeV ) {
      line % ( pp->mass() / Gaudi::Units::GeV ) % "GeV";
    } else {
      line % ( pp->mass() / Gaudi::Units::TeV ) % "TeV";
    }
    // lifetime/width/ctau
    const double ctau  = pp->ctau();
    const double width = pp->width();
    const double ltime = pp->lifetime();
    if ( 3600 * Gaudi::Units::second < ltime ) {
      line % "stable" % "";
    } else if ( 1 * Gaudi::Units::second < ltime ) {
      line % ( ltime / Gaudi::Units::second ) % "s";
    } else if ( 1 * Gaudi::Units::meter < ctau ) {
      line % ( ctau / Gaudi::Units::meter ) % "m";
    } else if ( 1 * Gaudi::Units::cm < ctau ) {
      line % ( ctau / Gaudi::Units::cm ) % "cm";
    } else if ( 0.5 * Gaudi::Units::mm < ctau ) {
      line % ( ctau / Gaudi::Units::mm ) % "mm";
    } else if ( 0.1 * Gaudi::Units::micrometer < ctau ) {
      line % ( ctau / Gaudi::Units::micrometer ) % "um";
    } else if ( 1 * Gaudi::Units::GeV < width ) {
      line % ( width / Gaudi::Units::GeV ) % "GeV";
    } else if ( 1 * Gaudi::Units::MeV < width ) {
      line % ( width / Gaudi::Units::MeV ) % "MeV";
    } else if ( 1 * Gaudi::Units::keV < width ) {
      line % ( width / Gaudi::Units::keV ) % "keV";
    } else if ( 1 * Gaudi::Units::eV < width ) {
      line % ( width / Gaudi::Units::eV ) % "eV";
    } else {
      line % ( ltime / Gaudi::Units::ns ) % "ns";
    }
    //
    line % pp->maxWidth() % pp->evtGen() % pp->pythia();
    //
    const Gaudi::ParticleProperty* const anti = pp->antiParticle();
    //
    if ( 0 == anti ) {
      line % "-";
    } else if ( anti == pp ) {
      line % "self-cc";
    } else {
      line % anti->name();
    }
    //
    stream << line << std::endl;
  }
  return stream << dashes << std::endl;
}
/*  print a list of properties in a form of the table
 *  @see Gaudi::ParticleProperty
 *  @param particles the list of particle properties
 *  @param service the service to extract global information
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date  2008-08-03
 */
std::string Gaudi::ParticleProperties::printAsTable( const std::vector<const Gaudi::ParticleProperty*>& particles,
                                                     const Gaudi::Interfaces::IParticlePropertySvc*     service ) {
  std::ostringstream s;
  printAsTable_( particles, s, service );
  return s.str();
}
/*  print a list of properties in a form of the table
 *  @see Gaudi::ParticleProperty
 *  @param particles the list of particle properties
 *  @param stream  the reference to the output stream
 *  @param service the service to extract global information
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date  2008-08-03
 */
MsgStream& Gaudi::ParticleProperties::printAsTable( const std::vector<const Gaudi::ParticleProperty*>& particles,
                                                    MsgStream&                                         stream,
                                                    const Gaudi::Interfaces::IParticlePropertySvc*     service ) {
  if ( stream.isActive() ) { printAsTable_( particles, stream.stream(), service ); }
  return stream;
}
/*  print a list of particle properties as formatted table
 *  @see Gaudi::ParticleProperty
 *  @param particles the list of particle properties
 *  @param stream the reference to the output stream
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date  2008-08-03
 */
std::ostream& Gaudi::Utils::toStream( const std::vector<const Gaudi::ParticleProperty*>& particles,
                                      std::ostream&                                      stream ) {
  return Gaudi::ParticleProperties::printAsTable_( particles, stream );
}
/* print properties in a form of the table
 *  @param particles (INPUT) list of particles
 *  @param stream    (UPDATE) the stream
 *  @param service   (INPUT) pointer to particle property service
 *  @return the stream
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date  2010-01-04
 */
std::ostream& Gaudi::ParticleProperties::printAsTable_( const std::vector<Gaudi::ParticleID>&          particles,
                                                        std::ostream&                                  stream,
                                                        const Gaudi::Interfaces::IParticlePropertySvc* service ) {
  //
  if ( 0 == service ) { return Gaudi::Utils::toStream( particles, stream ); }

  // convert list of ParticleIDs to list fo Particle properties:
  std::vector<const Gaudi::ParticleProperty*> props;
  for ( std::vector<Gaudi::ParticleID>::const_iterator ipid = particles.begin(); particles.end() != ipid; ++ipid ) {
    const Gaudi::ParticleProperty* pp = service->find( *ipid );
    if ( 0 == pp ) { continue; }
    props.push_back( pp );
  }
  return printAsTable_( props, stream, service );
}
/* print properties in a form of the table
 *  @param particles (INPUT) list of particles
 *  @param stream    (UPDATE) the stream
 *  @param service   (INPUT) pointer to particle property service
 *  @return the stream
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date  2010-01-04
 */
MsgStream& Gaudi::ParticleProperties::printAsTable( const std::vector<Gaudi::ParticleID>& particles, MsgStream& stream,
                                                    const Gaudi::Interfaces::IParticlePropertySvc* service ) {
  if ( stream.isActive() ) { printAsTable_( particles, stream.stream(), service ); }
  return stream;
}
/*  print properties in a form of the table
 *  @param particles (INPUT) list of particles
 *  @param service   (INPUT) pointer to particle property service
 *  @return string-representation
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date  2010-01-04
 */
std::string Gaudi::ParticleProperties::printAsTable( const std::vector<Gaudi::ParticleID>&          particles,
                                                     const Gaudi::Interfaces::IParticlePropertySvc* service ) {
  std::ostringstream s;
  printAsTable_( particles, s, service );
  return s.str();
}
const Gaudi::ParticleProperty* std::abs( const Gaudi::ParticleProperty* p ) {
  if ( 0 == p ) { return 0; }
  //
  if ( p->selfcc() || 0 <= p->particleID().pid() ) { return p; }
  //
  const Gaudi::ParticleProperty* a = p->anti();
  if ( 0 != a && 0 <= a->particleID().pid() ) { return a; }
  //
  return p; // should it be 0 ?
}
