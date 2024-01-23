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
// ============================================================================
#ifndef PARTPROP_PARTICLEPROPERTY_H
#define PARTPROP_PARTICLEPROPERTY_H 1
// ============================================================================
// Include files
// ============================================================================
#include <cmath>
#include <functional>
#include <iosfwd>
#include <set>
#include <string>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "Gaudi/ParticleID.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/PhysicalConstants.h"

// ============================================================================
namespace Gaudi {
  namespace Interfaces {
    class IParticlePropertySvc;
  }
  // ==========================================================================
  /** @class ParticleProperty ParticleProperty.h GaudiKernel/ParticleProperty.h
   *
   *  A trivial class to hold information about a single particle properties.
   *  All particle properties are accessible through accessor functions
   *
   *  @author Iain Last,G.Corti
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   */
  class GAUDI_API ParticleProperty final {
  public:
    // ========================================================================
    /** @struct Compare
     *  The comparion criteria for particle properties
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-10-14
     */
    struct Compare {
      inline bool operator()( const ParticleProperty* p1, const ParticleProperty* p2 ) const {
        return p1 == p2 ? false : 0 == p1 ? true : 0 == p2 ? false : ( *p1 < *p2 );
      }
    };
    // ========================================================================
  public: // constructors & destructors
    // ========================================================================
    /** full constructor, from all data (except the antiparticle )
     *  @param name       the name for the particle
     *  @param pid        the PID for the particle
     *  @param charge     the charge of the particle
     *  @param mass       the nominal mass of the particle
     *  @param tlife      the nominal lifetime of the particle
     *  @param maxWidth   the maximal width of the particle (used in generator)
     *  @param evtgen     the name of particle in EvtGen program
     *  @param pythia     the ID for the particle used in Pythia generator
     */
    ParticleProperty( const std::string& name, const Gaudi::ParticleID& pid, const double charge, const double mass,
                      const double tlife, const double maxWidth, const std::string& evtgen, const int pythia );
    /** a bit simplified constructor, from all data (except the antiparticle )
     *
     *    - "evtGenName" is set from the regular "name"
     *    - pythiaID     is set from the regular PID
     *
     *  @param name       the name for the particle
     *  @param pid        the PID for the particle
     *  @param charge     the charge of the particle
     *  @param mass       the nominal mass of the particle
     *  @param tlife      the nominal lifetime of the particle
     *  @param maxWidth   the maximal width of the particle (used in generator)
     */
    ParticleProperty( const std::string& name, const Gaudi::ParticleID& pid, const double charge, const double mass,
                      const double tlife, const double maxWidth );
    // ========================================================================
  public: // the major accessors
    // ========================================================================
    /// Get the particle name.
    const std::string& particle() const { return m_name; }
    /// Get the particle name.
    const std::string& name() const { return m_name; }
    /// get the particle ID
    const Gaudi::ParticleID& particleID() const { return m_pid; }
    /// get the particle ID
    const Gaudi::ParticleID& pdgID() const { return m_pid; }
    /// get the particle ID
    const Gaudi::ParticleID& pid() const { return m_pid; }
    /// Get the particle charge.
    double charge() const { return m_charge; } // the charge
    /// Get the particle mass.
    double mass() const { return m_mass; } // the mass
    /// Get the particle lifetime.
    double lifetime() const { return m_tlife; } // life-time
    /// Get the particle lifetime.
    double lifeTime() const { return m_tlife; } // life-time
    /// Get the particle proper lifetime in c*tau units
    double ctau() const { return Gaudi::Units::c_light * lifeTime(); }
#ifdef __INTEL_COMPILER             // Disable ICC remark
#  pragma warning( disable : 1572 ) // Floating-point equality and inequality comparisons are unreliable
#  pragma warning( push )
#endif
    /// Get the particle natural width
    double width() const { return 0 == ctau() ? 0.0 : Gaudi::Units::hbarc / ctau(); }
#ifdef __INTEL_COMPILER // Re-enable ICC remark
#  pragma warning( pop )
#endif
    /// Get the max width deviation
    double maxWidth() const { return m_maxWidth; } // max-width
    /// get the pointer to the anti-particle
    const Gaudi::ParticleProperty* antiParticle() const { return anti(); }
    /// get the pointer to the anti-particle
    const Gaudi::ParticleProperty* anti() const { return m_anti; }
    // ========================================================================
  public:
    // ========================================================================
    /// self-charge conjugated?
    bool selfcc() const { return m_anti == this; }
    // ========================================================================
  public: // some trivial delegation to ParticleID
    // ========================================================================
    ///  Three times the charge (in positron charge units)
    int threeCharge() const { return m_pid.threeCharge(); }
    // ========================================================================
  public: // the only one setter
    // ========================================================================
    /** set the pointer to the antipartice
     *  @attention it is the only one "setter"
     *  @param p pointer to anti-particle
     */
    void setAntiParticle( const ParticleProperty* p );
    // ========================================================================
  public: // other accessors
    // ========================================================================
    /// Get the EvtGen name
    const std::string& evtGenName() const { return m_evtgen; }
    /// Get the EvtGen name
    const std::string& evtGen() const { return m_evtgen; }
    /// Get the Pythia ID
    int pythiaID() const { return m_pythia; }
    /// Get the Pythia ID
    int pythia() const { return m_pythia; }
    // ========================================================================
  public: // some useful technicalities: comparison&ordering criteria
    // ========================================================================
    /// comparison/ordering operator  ( "strict-less-by-PID&Name" )
    friend bool operator<( const ParticleProperty& lhs, const ParticleProperty& rhs ) {
      return std::tie( lhs.m_pid, lhs.m_name ) < std::tie( rhs.m_pid, rhs.m_name );
    }
    // ========================================================================
  public: // Other minor details
    // ========================================================================
    /// implicit conversion to ParticleID class
    operator const Gaudi::ParticleID&() const { return m_pid; }
    // ========================================================================
  public:
    // ========================================================================
    /** the standard (a'la Gaudi) printout of the object
     *  @param s reference to the output stream
     *  @return reference to the output stream
     */
    std::ostream& fillStream( std::ostream& s ) const;
    /// simple method for conversion into the string
    std::string toString() const;
    // ========================================================================
  private:
    // ========================================================================
    /// The default constructor is disabled
    ParticleProperty(); // no default constructor
    // ========================================================================
  private: // data members
    // ========================================================================
    /// the name for the  particle
    std::string m_name; // the name for the  particle
    /// the PID for the patricle
    Gaudi::ParticleID m_pid; // the PID for the patricle
    /// the charge for the particle
    double m_charge; // the charge for the particle
    /// the nominal mass for the particle
    double m_mass; // the nominal mass for the particle
    /// the nominal proper lifetime for the particle
    double m_tlife; // the nominal proper lifetime
    /// The maximum width deviation
    double m_maxWidth; // The maximum width deviation
    /// the name of the particle for EvtGen program
    std::string m_evtgen; // the name of the particle for EvtGen
    /// the ID for the particle used in Pythia generator
    int m_pythia; // the ID for the particle used in Pythia
    /// the pointer to the anti-particle
    const Gaudi::ParticleProperty* m_anti; // the pointer to the anti-particle
    // ========================================================================
  };
  // ==========================================================================
} // end of namespace Gaudi
// ============================================================================
/** standard output operator to the stream
 *  @param stream the stream
 *  @param pp    the particle property object
 *  @return the stream
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date  2008-08-03
 */
std::ostream& operator<<( std::ostream& stream, const Gaudi::ParticleProperty& pp );
// ============================================================================
/// forward declaration
// ============================================================================
class MsgStream; // forward declaration
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  /// forward declaration
  class IParticlePropertySvc;
  // ==========================================================================
  namespace ParticleProperties {
    // ========================================================================
    /** print a list of properties in a form of the table
     *
     *  @code
     *
     *    Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *    const std::vector<const Gaudi::ParticleProperty*>& props = ... ;
     *
     *    std::cout << "Properties" << std::endl ;
     *    Gaudi::ParticleProperties::printAsTable ( props , std::cout , svc ) ;
     *
     *   @endcode
     *
     *  The utility is easy to use in conjunction with
     *  Gaudi::ParticleProperties:;get utilities:
     *  e.g. get all leptons from the service and print them as table:
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *
     *  typedef std::vector<const Gaudi::IParticleProperty*> Vector ;
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  // create the output vector:
     *  Vector leptons ;
     *  // use the function
     *  Gaudi::ParticleProperties::get
     *         ( svc ,
     *           // create the predicate:
     *           boost::lambda::bind ( &Gaudi::ParticleID::isLepton ,
     *               boost::lambda::bind ( &Gaudi::ParticleProperty::particleID , boost::lambda::_1 ) ) ,
     *           std::back_inserter ( leptons ) ) ; // output
     *
     *  // print the leptons:
     *  std::cout << "LEPTONS" << std::endl ;
     *  Gaudi::ParticleProperties::printAsTable_ ( leptons , std::cout , svc ) ;
     *
     *  @endcode
     *
     *  @see Gaudi::ParticleProperty
     *  @see Gaudi::Interfaces::IParticlePropertySvc
     *  @see Gaudi::ParticleProperties::get
     *  @param particles the list of particle properties
     *  @param stream  the reference to the output stream
     *  @param service the service to extract global information
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date  2008-08-03
     */
    GAUDI_API
    std::ostream& printAsTable_( const std::vector<const Gaudi::ParticleProperty*>& particles, std::ostream& stream,
                                 const Gaudi::Interfaces::IParticlePropertySvc* service = 0 );
    // ========================================================================
    /** print a list of properties in a form of the table
     *
     *  @code
     *
     *    Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *    const std::vector<const Gaudi::ParticleProperty*>& props = ... ;
     *
     *    std::cout << "Properties" << std::endl <<
     *    Gaudi::ParticleProperties::printAsTable ( props , svc ) << std::endl
     *
     *   @endcode
     *
     *  The utility is easy to use in conjunction with
     *  Gaudi::ParticleProperties:;get utilities:
     *  e.g. get all leptons from the service and print them as table:
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *
     *  typedef std::vector<const Gaudi::IParticleProperty*> Vector ;
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  // create the output vector:
     *  Vector leptons ;
     *  // use the function
     *  Gaudi::ParticleProperties::get
     *         ( svc ,
     *           // create the predicate:
     *           boost::lambda::bind ( &Gaudi::ParticleID::isLepton ,
     *               boost::lambda::bind ( &Gaudi::ParticleProperty::particleID , boost::lambda::_1 ) ) ,
     *           std::back_inserter ( leptons ) ) ; // output
     *
     *  // print the leptons:
     *  std::cout << "LEPTONS"
     *            << std::endl
     *  Gaudi::ParticleProperties::printAsTable ( leptons , svc )
     *            << std::endl ;
     *
     *  @endcode
     *
     *  @see Gaudi::ParticleProperty
     *  @see Gaudi::Interfaces::IParticlePropertySvc
     *  @see Gaudi::ParticleProperties::get
     *  @param particles the list of particle properties
     *  @param service the service to extract global information
     *  @return the string output
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date  2008-08-03
     */
    GAUDI_API
    std::string printAsTable( const std::vector<const Gaudi::ParticleProperty*>& particles,
                              const Gaudi::Interfaces::IParticlePropertySvc*     service = 0 );
    // ========================================================================
    /** print a list of properties in a form of the table
     *
     *  @code
     *
     *    Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *    const std::vector<const Gaudi::ParticleProperty*>& props = ... ;
     *
     *    MsgStream& log = ... ;
     *    Gaudi::ParticleProperties::printAsTable ( props , log , svc ) ;
     *    log << endmsg ;
     *
     *   @endcode
     *
     *  The utility is easy to use in conjunction with
     *  Gaudi::ParticleProperties:;get utilities:
     *  e.g. get all leptons from the service and print them as table:
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *
     *  typedef std::vector<const Gaudi::IParticleProperty*> Vector ;
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  // create the output vector:
     *  Vector leptons ;
     *  // use the function
     *  Gaudi::ParticleProperties::get
     *         ( svc ,
     *           // create the predicate:
     *           boost::lambda::bind ( &Gaudi::ParticleID::isLepton ,
     *               boost::lambda::bind ( &Gaudi::ParticleProperty::particleID , boost::lambda::_1 ) ) ,
     *           std::back_inserter ( leptons ) ) ; // output
     *
     *  // print the leptons:
     *  MsgStream& log = ... ;
     *  log << "LEPTONS" << std::endl ;
     *  Gaudi::ParticleProperties::printAsTable ( leptons , log , svc ) ;
     *  log << endmsg ;
     *
     *  @endcode
     *
     *  @see Gaudi::ParticleProperty
     *  @see Gaudi::Interfaces::IParticlePropertySvc
     *  @see Gaudi::ParticleProperties::get
     *  @param particles the list of particle properties
     *  @param stream  the reference to the output stream
     *  @param service the service to extract global information
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date  2008-08-03
     */
    GAUDI_API
    MsgStream& printAsTable( const std::vector<const Gaudi::ParticleProperty*>& particles, MsgStream& stream,
                             const Gaudi::Interfaces::IParticlePropertySvc* service = 0 );
    // ========================================================================
    /** print properties in a form of the table
     *  @param particles (INPUT) list of particles
     *  @param stream    (UPDATE) the stream
     *  @param service   (INPUT) pointer to particle property service
     *  @return the stream
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date  2010-01-04
     */
    GAUDI_API
    std::ostream& printAsTable_( const std::vector<Gaudi::ParticleID>& particles, std::ostream& stream,
                                 const Gaudi::Interfaces::IParticlePropertySvc* service = 0 );
    // ========================================================================
    template <class C_, class A_>
    inline std::ostream& printAsTable_( const std::set<Gaudi::ParticleID, C_, A_>& particles, std::ostream& stream,
                                        const Gaudi::Interfaces::IParticlePropertySvc* service = 0 ) {
      return printAsTable_( std::vector<Gaudi::ParticleID>( particles.begin(), particles.end() ), stream, service );
    }
    // ========================================================================
    /** print properties in a form of the table
     *  @param particles (INPUT) list of particles
     *  @param service   (INPUT) pointer to particle property service
     *  @return string-representation
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date  2010-01-04
     */
    GAUDI_API
    std::string printAsTable( const std::vector<Gaudi::ParticleID>&          particles,
                              const Gaudi::Interfaces::IParticlePropertySvc* service = 0 );
    // ========================================================================
    template <class C_, class A_>
    inline std::string printAsTable( const std::set<Gaudi::ParticleID, C_, A_>&     particles,
                                     const Gaudi::Interfaces::IParticlePropertySvc* service = 0 ) {
      return printAsTable( std::vector<Gaudi::ParticleID>( particles.begin(), particles.end() ), service );
    }
    // ========================================================================
    /** print properties in a form of the table
     *  @param particles (INPUT) list of particles
     *  @param stream    (UPDATE) the stream
     *  @param service   (INPUT) pointer to particle property service
     *  @return the stream
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date  2010-01-04
     */
    GAUDI_API
    MsgStream& printAsTable( const std::vector<Gaudi::ParticleID>& particles, MsgStream& stream,
                             const Gaudi::Interfaces::IParticlePropertySvc* service = 0 );
    // ========================================================================
    template <class C_, class A_>
    inline MsgStream& printAsTable( const std::set<Gaudi::ParticleID, C_, A_>& particles, MsgStream& stream,
                                    const Gaudi::Interfaces::IParticlePropertySvc* service = 0 ) {
      return printAsTable( std::vector<Gaudi::ParticleID>( particles.begin(), particles.end() ), stream, service );
    }
    // ========================================================================
  } // namespace ParticleProperties
  // ==========================================================================
} //                                                      end of namespace Gaudi
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace Utils {
    // ========================================================================
    /** print a list of particle properties as formatted table
     *  @see Gaudi::ParticleProperty
     *  @param particles the list of particle properties
     *  @param stream the reference to the output stream
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date  2008-08-03
     */
    GAUDI_API
    std::ostream& toStream( const std::vector<const Gaudi::ParticleProperty*>& particles, std::ostream& stream );
    // ========================================================================
  } // namespace Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
namespace std {
  // ==========================================================================
  GAUDI_API
  const Gaudi::ParticleProperty* abs( const Gaudi::ParticleProperty* p );
  // ==========================================================================
} // namespace std
// ============================================================================
//                                                                      The END
// ============================================================================
#endif // PARTPROP_PARTICLEPROPERTY_H
