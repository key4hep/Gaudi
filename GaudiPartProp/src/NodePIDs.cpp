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
// Include files
// ============================================================================
// Event
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <climits>
#include <functional>
// ============================================================================
// GauidKernel
// ============================================================================
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/SystemOfUnits.h"
// ============================================================================
// PartProp
// ============================================================================
#include "Decays/NodesPIDs.h"
#include "Decays/Symbols.h"
#include "Gaudi/ParticleID.h"
#include "Gaudi/ParticleProperty.h"
#include <Gaudi/Interfaces/IParticlePropertySvc.h>
// ============================================================================
// Originally from LHCbMath
// ============================================================================
#include "Gaudi/Math/Lomont.h"
// ============================================================================
/** @file
 *  The implementation file for various decay nodes
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-04-12
 */
// ============================================================================
namespace {
  // ==========================================================================
  /** @var s_SHORTLIVED
   *  The default high-edge for c*tau for particle to be considered
   *  as "short-lived" particles
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  const double s_SHORTLIVED = 0.1 * Gaudi::Units::micrometer;
  // ==========================================================================
  /** @var s_LONGLIVED
   *  The default low-edge for c*tau for particle to be considered
   *  as "short-lived" particles
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  const double s_LONGLIVED = s_SHORTLIVED;
  // ==========================================================================
  /** @var s_STABLE
   *  The default low-edge for c*tau for particle to be considered
   *  as "stable" particle
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  const double s_STABLE = 1 * Gaudi::Units::meter;
  // ==========================================================================
  /** @var s_INFINITY
   *  The default low-edge for c*tau for particle to be considered
   *  as "stable" particle
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  const double s_INFINITY = 0.5 * std::numeric_limits<double>::max();
  // ==========================================================================
} // namespace
namespace Decays {
  // ============================================================================
  /*  Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  // ============================================================================
  Decays::Nodes::Or operator||( const Decays::iNode& o1, const std::string& o2 ) {
    return o1 || Decays::Nodes::Pid( o2 );
  }
  // ============================================================================
  Decays::Nodes::Or operator||( const Decays::iNode& o1, const Gaudi::ParticleID& o2 ) {
    return o1 || Decays::Nodes::Pid( o2 );
  }
  // ============================================================================
  Decays::Nodes::Or operator||( const Decays::iNode& o1, const Decays::Decay::Item& o2 ) {
    return o1 || Decays::Nodes::Pid( o2 );
  }
  // ============================================================================
  Decays::Nodes::Or operator||( const Decays::iNode& o1, const Gaudi::ParticleProperty* o2 ) {
    return o1 || Decays::Nodes::Pid( o2 );
  }
  // ============================================================================
  Decays::Nodes::Or operator||( const std::string& o2, const Decays::iNode& o1 ) {
    return Decays::Nodes::Pid( o2 ) || o1;
  }
  // ============================================================================
  Decays::Nodes::Or operator||( const Gaudi::ParticleID& o2, const Decays::iNode& o1 ) {
    return Decays::Nodes::Pid( o2 ) || o1;
  }
  // ============================================================================
  Decays::Nodes::Or operator||( const Decays::Decay::Item& o2, const Decays::iNode& o1 ) {
    return Decays::Nodes::Pid( o2 ) || o1;
  }
  // ============================================================================
  Decays::Nodes::Or operator||( const Gaudi::ParticleProperty* o2, const Decays::iNode& o1 ) {
    return Decays::Nodes::Pid( o2 ) || o1;
  }
  // ============================================================================
  /*  Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  // ============================================================================
  Decays::Nodes::And operator&&( const Decays::iNode& o1, const std::string& o2 ) {
    return o1 && Decays::Nodes::Pid( o2 );
  }
  // ============================================================================
  Decays::Nodes::And operator&&( const Decays::iNode& o1, const Gaudi::ParticleID& o2 ) {
    return o1 && Decays::Nodes::Pid( o2 );
  }
  // ============================================================================
  Decays::Nodes::And operator&&( const Decays::iNode& o1, const Decays::Decay::Item& o2 ) {
    return o1 && Decays::Nodes::Pid( o2 );
  }
  // ============================================================================
  Decays::Nodes::And operator&&( const Decays::iNode& o1, const Gaudi::ParticleProperty* o2 ) {
    return o1 && Decays::Nodes::Pid( o2 );
  }
  // ============================================================================
  Decays::Nodes::And operator&&( const std::string& o2, const Decays::iNode& o1 ) {
    return Decays::Nodes::Pid( o2 ) && o1;
  }
  // ============================================================================
  Decays::Nodes::And operator&&( const Gaudi::ParticleID& o2, const Decays::iNode& o1 ) {
    return Decays::Nodes::Pid( o2 ) && o1;
  }
  // ============================================================================
  Decays::Nodes::And operator&&( const Decays::Decay::Item& o2, const Decays::iNode& o1 ) {
    return Decays::Nodes::Pid( o2 ) && o1;
  }
  // ============================================================================
  Decays::Nodes::And operator&&( const Gaudi::ParticleProperty* o2, const Decays::iNode& o1 ) {
    return Decays::Nodes::Pid( o2 ) && o1;
  }
  // ============================================================================
} // namespace Decays
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Any* Decays::Nodes::Any::clone() const { return new Any( *this ); }
// ============================================================================
// MANDATORY: check the validity
// ============================================================================
bool Decays::Nodes::Any::valid() const { return true; }
// ============================================================================
// MANDATORY: the proper validation of the node
// ============================================================================
StatusCode Decays::Nodes::Any::validate( const Gaudi::Interfaces::IParticlePropertySvc* /*svc */ ) const {
  return StatusCode::SUCCESS;
}
// ============================================================================
// MANDATORY: the only one essential method
// ============================================================================
bool Decays::Nodes::Any::operator()( const Gaudi::ParticleID& /* p */ ) const { return true; }
// ============================================================================

// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Pid* Decays::Nodes::Pid::clone() const { return new Pid( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::CC* Decays::Nodes::CC::clone() const { return new CC( *this ); }
// ============================================================================

// ============================================================================
// xSpin: valid only for reasonable spin
bool Decays::Nodes::JSpin::valid() const { return 0 < spin(); }
// ============================================================================
// MANDATORY: the proper validation of the node
// ============================================================================
StatusCode Decays::Nodes::JSpin::validate( const Gaudi::Interfaces::IParticlePropertySvc* /*svc */ ) const {
  return valid() ? StatusCode::SUCCESS : StatusCode{ InvalidSpin };
}
// ============================================================================

// ============================================================================
// constructor from the decay item
// ============================================================================
Decays::Nodes::Pid::Pid( const Decays::Decay::Item& item ) : Decays::iNode(), m_item( item ) {}
// ============================================================================
// constructor from Particle Property
// ============================================================================
Decays::Nodes::Pid::Pid( const Gaudi::ParticleProperty* item ) : Decays::iNode(), m_item( item ) {}
// ============================================================================
// constructor from ParticleID
// ============================================================================
Decays::Nodes::Pid::Pid( const Gaudi::ParticleID& item ) : Decays::iNode(), m_item( item ) {}
// ============================================================================
// constructor from Particle name
// ============================================================================
Decays::Nodes::Pid::Pid( const std::string& item ) : Decays::iNode(), m_item( item ) {}
// ============================================================================
// MANDATORY: check the validity
// ============================================================================
bool Decays::Nodes::Pid::valid() const { return 0 != m_item.pp(); }
// ============================================================================
// MANDATORY: the proper validation of the node
// ============================================================================
StatusCode Decays::Nodes::Pid::validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  return m_item.validate( svc );
}
// ============================================================================

// ============================================================================
// constructor from the decay item
// ============================================================================
Decays::Nodes::CC::CC( const Decays::Decay::Item& item ) : Decays::Nodes::Pid( item ) {}
// ============================================================================
// constructor from Particle Property
// ============================================================================
Decays::Nodes::CC::CC( const Gaudi::ParticleProperty* item ) : Decays::Nodes::Pid( item ) {}
// ============================================================================
// constructor from ParticleID
// ============================================================================
Decays::Nodes::CC::CC( Gaudi::ParticleID item ) : Decays::Nodes::Pid( item ) {}
// ============================================================================
// constructor from Particle name
// ============================================================================
Decays::Nodes::CC::CC( const std::string& item ) : Decays::Nodes::Pid( item ) {}
// ============================================================================
// constructor from Pid
// ============================================================================
Decays::Nodes::CC::CC( const Decays::Nodes::Pid& item ) : Decays::Nodes::Pid( item ) {}
// ============================================================================

// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Meson* Decays::Nodes::Meson::clone() const { return new Meson( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Lepton* Decays::Nodes::Lepton::clone() const { return new Lepton( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Nu* Decays::Nodes::Nu::clone() const { return new Nu( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Ell* Decays::Nodes::Ell::clone() const { return new Ell( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::EllPlus* Decays::Nodes::EllPlus::clone() const { return new EllPlus( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::EllMinus* Decays::Nodes::EllMinus::clone() const { return new EllMinus( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Baryon* Decays::Nodes::Baryon::clone() const { return new Baryon( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Hadron* Decays::Nodes::Hadron::clone() const { return new Hadron( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Charged* Decays::Nodes::Charged::clone() const { return new Charged( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Positive* Decays::Nodes::Positive::clone() const { return new Positive( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Negative* Decays::Nodes::Negative::clone() const { return new Negative( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Neutral* Decays::Nodes::Neutral::clone() const { return new Neutral( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Nucleus* Decays::Nodes::Nucleus::clone() const { return new Nucleus( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::HasQuark* Decays::Nodes::HasQuark::clone() const { return new HasQuark( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::JSpin* Decays::Nodes::JSpin::clone() const { return new JSpin( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::SSpin* Decays::Nodes::SSpin::clone() const { return new SSpin( *this ); }
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::LSpin* Decays::Nodes::LSpin::clone() const { return new LSpin( *this ); }

// ============================================================================
// constructor from the quark
// ============================================================================
Decays::Nodes::HasQuark::HasQuark( Gaudi::ParticleID::Quark quark ) : Decays::Nodes::Any(), m_quark( quark ) {}
// ============================================================================
// constructor from the 2J+1
// ============================================================================
Decays::Nodes::JSpin::JSpin( const int spin ) : Decays::Nodes::Any(), m_spin( spin ) {}
// ============================================================================
// constructor from the 2S+1
// ============================================================================
Decays::Nodes::SSpin::SSpin( const int spin ) : Decays::Nodes::JSpin( spin ) {}
// ============================================================================
// constructor from the 2L+1
// ============================================================================
Decays::Nodes::LSpin::LSpin( const int spin ) : Decays::Nodes::SSpin( spin ) {}
// ============================================================================

// ============================================================================
std::ostream& Decays::Nodes::Any::fillStream( std::ostream& s ) const { return s << " X "; }
// ============================================================================
std::ostream& Decays::Nodes::Pid::fillStream( std::ostream& s ) const { return s << " " << m_item.name() << " "; }
// ============================================================================
std::ostream& Decays::Nodes::CC::fillStream( std::ostream& s ) const { return s << " [" << item().name() << "]cc "; }
// ============================================================================
std::ostream& Decays::Nodes::Meson::fillStream( std::ostream& s ) const { return s << " Meson "; }
// ============================================================================
std::ostream& Decays::Nodes::Hadron::fillStream( std::ostream& s ) const { return s << " Hadron "; }
// ============================================================================
std::ostream& Decays::Nodes::Baryon::fillStream( std::ostream& s ) const { return s << " Baryon "; }
// ============================================================================
std::ostream& Decays::Nodes::Lepton::fillStream( std::ostream& s ) const { return s << " Lepton "; }
// ============================================================================
std::ostream& Decays::Nodes::Neutral::fillStream( std::ostream& s ) const { return s << " X0 "; }
// ============================================================================
std::ostream& Decays::Nodes::Charged::fillStream( std::ostream& s ) const { return s << " Xq "; }
// ============================================================================
std::ostream& Decays::Nodes::Positive::fillStream( std::ostream& s ) const { return s << " X+ "; }
// ============================================================================
std::ostream& Decays::Nodes::Negative::fillStream( std::ostream& s ) const { return s << " X- "; }
// ============================================================================
std::ostream& Decays::Nodes::Nu::fillStream( std::ostream& s ) const { return s << " Nu "; }
// ============================================================================
std::ostream& Decays::Nodes::Ell::fillStream( std::ostream& s ) const { return s << " l "; }
// ============================================================================
std::ostream& Decays::Nodes::EllPlus::fillStream( std::ostream& s ) const { return s << " l+ "; }
// ============================================================================
std::ostream& Decays::Nodes::EllMinus::fillStream( std::ostream& s ) const { return s << " l- "; }
// ============================================================================
std::ostream& Decays::Nodes::Nucleus::fillStream( std::ostream& s ) const { return s << " Nucleus "; }
// ============================================================================
std::ostream& Decays::Nodes::HasQuark::fillStream( std::ostream& s ) const {
  switch ( m_quark ) {
  case Gaudi::ParticleID::down:
    return s << " Xd ";
  case Gaudi::ParticleID::up:
    return s << " Xu ";
  case Gaudi::ParticleID::strange:
    return s << " Xs ";
  case Gaudi::ParticleID::charm:
    return s << " Xc ";
  case Gaudi::ParticleID::bottom:
    return s << " Xb ";
  case Gaudi::ParticleID::top:
    return s << " Xt ";
  default:;
  }
  return s << " HasQuark(" << (int)m_quark << ") ";
}
// ============================================================================
std::ostream& Decays::Nodes::JSpin::fillStream( std::ostream& s ) const {
  switch ( spin() ) {
  case 1:
    return s << " Scalar ";
  case 2:
    return s << " Spinor ";
  case 3:
    return s << " Vector ";
  case 4:
    return s << " ThreeHalf ";
  case 5:
    return s << " Tensor ";
  case 6:
    return s << " FiveHalf ";
  default:;
  }
  return s << " JSpin(" << m_spin << ") ";
}
// ===========================================================================
std::ostream& Decays::Nodes::SSpin::fillStream( std::ostream& s ) const { return s << " SSpin(" << spin() << ") "; }
// ===========================================================================
std::ostream& Decays::Nodes::LSpin::fillStream( std::ostream& s ) const { return s << " LSpin(" << spin() << ") "; }
// ===========================================================================

// ============================================================================
// constructor from c-tau range & Service
// ============================================================================
Decays::Nodes::CTau::CTau( const double low, const double high, const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : m_low( low ), m_high( high ) {
  if ( svc ) { m_ppSvc = const_cast<Gaudi::Interfaces::IParticlePropertySvc*>( svc ); }
}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::CTau* Decays::Nodes::CTau::clone() const { return new CTau( *this ); }
// ============================================================================
// MANDATORY: the specific printout
std::ostream& Decays::Nodes::CTau::fillStream( std::ostream& s ) const {
  return s << " CTau(" << low() << "," << high() << ") ";
}
// ============================================================================
// MANDATORY: check the validity
bool Decays::Nodes::CTau::valid() const { return m_ppSvc.isValid(); }
// ============================================================================
// MANDATORY: the proper validation of the node
// ============================================================================
StatusCode Decays::Nodes::CTau::validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  return setService( svc );
}
// ============================================================================
// MANDATORY: the only one essential method
// ============================================================================
bool Decays::Nodes::CTau::operator()( const Gaudi::ParticleID& pid ) const {
  return classify( pid, [&]( Gaudi::ParticleID id ) {
    // invalid node
    if ( !ppSvc() ) { return false; } //  REJECT for invalid service
    // use the service
    const Gaudi::ParticleProperty* pp = ppSvc()->find( id );
    if ( !pp ) { return false; } // REJECT for invalid dparticle
    // final decision: check ctau range
    const auto ctau = pp->ctau();
    return low() <= ctau && ctau <= high();
  } );
}
// ============================================================================
bool Decays::Nodes::CTau::add_( Gaudi::ParticleID pid, bool acc ) const {
  return m_pids.with_lock(
      []( KnownPids& pids, Gaudi::ParticleID pid, bool acc ) {
        auto& c     = ( acc ? pids.accepted : pids.rejected );
        auto  ifind = std::lower_bound( c.begin(), c.end(), pid );
        if ( c.end() == ifind || pid < ( *ifind ) ) { c.insert( ifind, pid ); }
        return acc;
      },
      pid, acc );
}
// ============================================================================
// MANDATORY: the proper validation of the node
// ============================================================================
StatusCode Decays::Nodes::CTau::setService( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  // if service is substituted, clear the cache
  if ( m_ppSvc != svc ) {
    m_pids.with_lock( []( KnownPids& pids ) {
      pids.accepted.clear();
      pids.rejected.clear();
    } );
  }
  //
  if ( svc ) {
    m_ppSvc = const_cast<Gaudi::Interfaces::IParticlePropertySvc*>( svc );
  } else {
    m_ppSvc = nullptr;
  }
  //
  return m_ppSvc.isValid() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
// ============================================================================
// constructor from c-tau range & Service
// ============================================================================
Decays::Nodes::Mass::Mass( const double low, const double high, const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : Decays::Nodes::CTau( low, high, svc ) {}
// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::Mass* Decays::Nodes::Mass::clone() const { return new Mass( *this ); }
// ============================================================================
// MANDATORY: the specific printout
std::ostream& Decays::Nodes::Mass::fillStream( std::ostream& s ) const {
  return s << " Mass(" << low() << "," << high() << ") ";
}
// ============================================================================
// MANDATORY: the only one essential method
// ============================================================================
bool Decays::Nodes::Mass::operator()( const Gaudi::ParticleID& pid ) const {
  return classify( pid, [&]( Gaudi::ParticleID pid ) {
    // invalid node
    if ( !ppSvc() ) { return false; } //  REJECT for invaild service
    // use the service
    const Gaudi::ParticleProperty* pp = ppSvc()->find( pid );
    if ( !pp ) { return false; } // REJECT for invalid particle
    // final decision: check mass range
    const auto mass = pp->mass();
    return low() <= mass && mass <= high();
  } );
}
// ===========================================================================

// ===========================================================================
// constructor with high edge and service
// ===========================================================================
Decays::Nodes::ShortLived_::ShortLived_( const double high, const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : Decays::Nodes::CTau( -1 * Gaudi::Units::km, high, svc ) {}
// ===========================================================================
// constructor with high edge and service
// ===========================================================================
Decays::Nodes::ShortLived_::ShortLived_( const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : Decays::Nodes::CTau( -1 * Gaudi::Units::km, s_SHORTLIVED, svc ) {}
// ===========================================================================
// MANDATORY: clone method ("virtual constructor")
// ===========================================================================
Decays::Nodes::ShortLived_* Decays::Nodes::ShortLived_::clone() const {
  return new Decays::Nodes::ShortLived_( *this );
}
// ===========================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Nodes::ShortLived_::fillStream( std::ostream& s ) const {
  if ( Gaudi::Math::lomont_compare_double( s_SHORTLIVED, high(), 1000 ) ) { return s << " ShortLived "; }
  return s << " ShortLived_( " << high() << ") ";
}
// ===========================================================================

// ===========================================================================
// constructor with high edge and service
// ===========================================================================
Decays::Nodes::LongLived_::LongLived_( const double low, const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : Decays::Nodes::CTau( low, s_INFINITY, svc ) {}
// ===========================================================================
// constructor with high edge and service
// ===========================================================================
Decays::Nodes::LongLived_::LongLived_( const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : Decays::Nodes::CTau( s_LONGLIVED, s_INFINITY, svc ) {}
// ===========================================================================
// MANDATORY: clone method ("virtual constructor")
// ===========================================================================
Decays::Nodes::LongLived_* Decays::Nodes::LongLived_::clone() const { return new Decays::Nodes::LongLived_( *this ); }
// ===========================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Nodes::LongLived_::fillStream( std::ostream& s ) const {
  if ( Gaudi::Math::lomont_compare_double( s_LONGLIVED, low(), 1000 ) ) { return s << " LongLived "; }
  return s << " LongLived_( " << low() << ") ";
}
// ===========================================================================

// ===========================================================================
// constructor with the service
// ===========================================================================
Decays::Nodes::Stable::Stable( const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : Decays::Nodes::LongLived_( s_STABLE, svc ) {}
// ===========================================================================
// MANDATORY: clone method ("virtual constructor")
// ===========================================================================
Decays::Nodes::Stable* Decays::Nodes::Stable::clone() const { return new Decays::Nodes::Stable( *this ); }
// ===========================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Nodes::Stable::fillStream( std::ostream& s ) const { return s << " Stable "; }
// ===========================================================================

// ===========================================================================
// constructor with the service
// ===========================================================================
Decays::Nodes::StableCharged::StableCharged( const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : Decays::Nodes::Stable( svc ) {}
// ===========================================================================
// MANDATORY: clone method ("virtual constructor")
// ===========================================================================
Decays::Nodes::StableCharged* Decays::Nodes::StableCharged::clone() const {
  return new Decays::Nodes::StableCharged( *this );
}
// ===========================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Nodes::StableCharged::fillStream( std::ostream& s ) const { return s << " StableCharged "; }
// ===========================================================================
// MANDATORY: the only one essential method
// ============================================================================
bool Decays::Nodes::StableCharged::operator()( const Gaudi::ParticleID& pid ) const {
  return classify( pid, [&]( Gaudi::ParticleID id ) {
    // invalid node
    if ( !ppSvc() ) { return false; } //  REJECT for invalid service
    // use the service
    const Gaudi::ParticleProperty* pp = ppSvc()->find( id );
    if ( !pp ) { return false; } // REJECT for invalid dparticle
    // final decision: check ctau range
    return low() <= pp->ctau() && 0 != id.threeCharge();
  } );
}
// ============================================================================

// ===========================================================================
// constructor with high edge and service
// ===========================================================================
Decays::Nodes::Light::Light( const double high, const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : Decays::Nodes::Mass( -1 * Gaudi::Units::TeV, high, svc ) {}
// ===========================================================================
// MANDATORY: clone method ("virtual constructor")
// ===========================================================================
Decays::Nodes::Light* Decays::Nodes::Light::clone() const { return new Decays::Nodes::Light( *this ); }
// ===========================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Nodes::Light::fillStream( std::ostream& s ) const { return s << " Light( " << high() << ") "; }
// ===========================================================================

// ===========================================================================
// constructor with high edge and service
// ===========================================================================
Decays::Nodes::Heavy::Heavy( const double low, const Gaudi::Interfaces::IParticlePropertySvc* svc )
    : Decays::Nodes::Mass( low, s_INFINITY, svc ) {}
// ===========================================================================
// MANDATORY: clone method ("virtual constructor")
// ===========================================================================
Decays::Nodes::Heavy* Decays::Nodes::Heavy::clone() const { return new Decays::Nodes::Heavy( *this ); }
// ===========================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Nodes::Heavy::fillStream( std::ostream& s ) const { return s << " Heavy( " << high() << ") "; }
// ===========================================================================

// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::PosID* Decays::Nodes::PosID::clone() const { return new PosID( *this ); }
// ============================================================================
// MANDATORY: check the validity
// ============================================================================
bool Decays::Nodes::PosID::valid() const { return true; }
// ============================================================================
// MANDATORY: the proper validation of the node
// ============================================================================
StatusCode Decays::Nodes::PosID::validate( const Gaudi::Interfaces::IParticlePropertySvc* /*svc */ ) const {
  return StatusCode::SUCCESS;
}
// ============================================================================
// MANDATORY: the only one essential method
// ============================================================================
bool Decays::Nodes::PosID::operator()( const Gaudi::ParticleID& p ) const { return 0 < p.pid(); }
// ============================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Nodes::PosID::fillStream( std::ostream& s ) const { return s << " ID+ "; }
// ===========================================================================

// ============================================================================
// MANDATORY: clone method ("virtual constructor")
// ============================================================================
Decays::Nodes::NegID* Decays::Nodes::NegID::clone() const { return new NegID( *this ); }
// ============================================================================
// MANDATORY: check the validity
// ============================================================================
bool Decays::Nodes::NegID::valid() const { return true; }
// ============================================================================
// MANDATORY: the proper validation of the node
// ============================================================================
StatusCode Decays::Nodes::NegID::validate( const Gaudi::Interfaces::IParticlePropertySvc* /*svc */ ) const {
  return StatusCode::SUCCESS;
}
// ============================================================================
// MANDATORY: the only one essential method
// ============================================================================
bool Decays::Nodes::NegID::operator()( const Gaudi::ParticleID& p ) const { return 0 > p.pid(); }
// ============================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Nodes::NegID::fillStream( std::ostream& s ) const { return s << " ID- "; }
// ===========================================================================

// ===========================================================================
//  constructor from the symbol
// ===========================================================================
Decays::Nodes::Symbol::Symbol( const std::string& s ) : Decays::iNode(), m_symbol( Decays::Nodes::Invalid() ) {
  const Decays::Symbols& sym = Decays::Symbols::instance();
  StatusCode             sc  = sym.symbol( s, m_symbol );
  sc.ignore();
}
// ===========================================================================
// MANDATORY: clone method ("virtual constructor")
// ===========================================================================
Decays::Nodes::Symbol* Decays::Nodes::Symbol::clone() const { return new Decays::Nodes::Symbol( *this ); }
// ===========================================================================
// MANDATORY: the only one essential method
// ===========================================================================
bool Decays::Nodes::Symbol::operator()( const Gaudi::ParticleID& pid ) const { return m_symbol.node( pid ); }
// ===========================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Nodes::Symbol::fillStream( std::ostream& s ) const { return s << m_symbol.node(); }
// ===========================================================================
// MANDATORY: check the validity
// ===========================================================================
bool Decays::Nodes::Symbol::valid() const { return m_symbol.valid(); }
// ===========================================================================
// MANDATORY: validate
// ===========================================================================
StatusCode Decays::Nodes::Symbol::validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  return m_symbol.validate( svc );
}
// ===========================================================================

// ===========================================================================
// The END
// ===========================================================================
