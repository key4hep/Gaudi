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
#include <Gaudi/Decays/Decay.h>
#include <Gaudi/Interfaces/IParticlePropertySvc.h>
#include <Gaudi/ParticleID.h>
#include <Gaudi/ParticleProperty.h>
#include <sstream>

namespace Decays = Gaudi::Decays;

/** @file
 *  Implementation file for class Decays::Decay
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-03-31
 */
Decays::Decay::Item::Item( const Gaudi::ParticleProperty* pp ) : m_pp( pp ) {
  if ( m_pp ) {
    m_name = m_pp->particle();
    m_pid  = m_pp->particleID();
  }
}

Decays::Decay::Item::Item( const std::string& name ) : m_name( name ) {}

Decays::Decay::Item::Item( const Gaudi::ParticleID& pid ) : m_pid( pid ) {}

StatusCode Decays::Decay::Item::validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  if ( m_pp ) {
    m_name = m_pp->particle();
    m_pid  = m_pp->particleID();
    return StatusCode::SUCCESS;
  }
  // it not possible to validate it!
  if ( 0 == svc ) { return StatusCode::FAILURE; }
  // check by name
  if ( !m_name.empty() ) {
    m_pp = svc->find( m_name );
    if ( !m_pp ) { return StatusCode::FAILURE; }
    m_pid = m_pp->particleID();
    return StatusCode::SUCCESS;
  }
  // check by PID
  if ( Gaudi::ParticleID() != m_pid ) {
    m_pp = svc->find( m_pid );
    if ( !m_pp ) { return StatusCode::FAILURE; }
    m_name = m_pp->particle();
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode Decays::Decay::Item::validate( const Gaudi::ParticleProperty* pp ) const {
  if ( m_pp && !pp ) {
    m_name = m_pp->particle();
    m_pid  = m_pp->particleID();
    return StatusCode::SUCCESS;
  } else if ( pp ) {
    m_pp   = pp;
    m_name = m_pp->particle();
    m_pid  = m_pp->particleID();
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

Decays::Decay::Decay( const Gaudi::ParticleProperty*                     mother,
                      const std::vector<const Gaudi::ParticleProperty*>& daughters )
    : m_mother( mother ) {
  setDaughters( daughters );
}

Decays::Decay::Decay( const std::string& mother, const std::vector<std::string>& daughters ) : m_mother( mother ) {
  setDaughters( daughters );
}

Decays::Decay::Decay( const Gaudi::ParticleID& mother, const std::vector<Gaudi::ParticleID>& daughters )
    : m_mother( mother ) {
  setDaughters( daughters );
}

Decays::Decay::Decay( const Decays::Decay::Item& mother, const std::vector<Decays::Decay::Item>& daughters )
    : m_mother( mother ), m_daughters( daughters ) {}

StatusCode Decays::Decay::validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  // validate the mother
  StatusCode sc = m_mother.validate( svc );
  if ( sc.isFailure() ) { return sc; }
  if ( m_daughters.empty() ) { return StatusCode::FAILURE; }
  // loop over daughters
  for ( auto idau = m_daughters.begin(); m_daughters.end() != idau; ++idau ) {
    sc = idau->validate( svc );
    if ( sc.isFailure() ) { return sc; }
  }
  return sc;
}

std::ostream& Decays::Decay::fillStream( std::ostream& s ) const {
  m_mother.fillStream( s );
  if ( m_daughters.empty() ) { return s; }
  s << "->";
  // loop over daughters
  for ( const auto& dau : m_daughters ) { dau.fillStream( s ); }
  return s;
}

std::ostream& Decays::Decay::Item::fillStream( std::ostream& s ) const {
  if ( m_name.empty() ) {
    if ( m_pp ) {
      m_name = m_pp->particle();
    } else if ( Gaudi::ParticleID() != m_pid ) {
      return s << ' ' << m_pid.pid() << ' ';
    } else {
      return s << " ? ";
    }
  }
  return s << ' ' << m_name << ' ';
}

std::string Decays::Decay::toString() const {
  std::ostringstream s;
  fillStream( s );
  return s.str();
}

void Decays::Decay::setDaughters( const std::vector<const Gaudi::ParticleProperty*>& daugs ) {
  m_daughters.clear();
  for ( const auto& pp : daugs ) *this += pp;
}

void Decays::Decay::setDaughters( const std::vector<std::string>& daugs ) {
  m_daughters.clear();
  for ( const auto& pp : daugs ) *this += pp;
}

void Decays::Decay::setDaughters( const std::vector<Gaudi::ParticleID>& daugs ) {
  m_daughters.clear();
  for ( const auto& pp : daugs ) *this += pp;
}

void Decays::Decay::setDaughters( const Decays::Decay::Items& daugs ) { m_daughters = daugs; }

Decays::Decay& Decays::Decay::operator+=( const std::string& child ) { return ( *this ) += Item( child ); }

Decays::Decay& Decays::Decay::operator+=( const Gaudi::ParticleID& child ) { return ( *this ) += Item( child ); }

Decays::Decay& Decays::Decay::operator+=( const Gaudi::ParticleProperty* child ) { return ( *this ) += Item( child ); }

Decays::Decay& Decays::Decay::operator+=( const Decays::Decay::Item& child ) {
  m_daughters.push_back( child );
  return *this;
}

/*  get the component by the number
 *  @attention index 0 corresponds to the mother particle
 *  @param index the index (0 corresponds to the mother particle)
 *  @return the component
 */
const Decays::Decay::Item& Decays::Decay::operator()( const unsigned int index ) const {
  if ( 0 == index ) { return m_mother; }
  return m_daughters[index - 1];
}

void Decays::Decay::setMother( const Decays::Decay::Item& mom ) { m_mother = mom; }

void Decays::Decay::setMother( const Gaudi::ParticleProperty* mom ) { setMother( Item( mom ) ); }

void Decays::Decay::setMother( const Gaudi::ParticleID& mom ) { setMother( Item( mom ) ); }

void Decays::Decay::setMother( const std::string& mom ) { setMother( Item( mom ) ); }
