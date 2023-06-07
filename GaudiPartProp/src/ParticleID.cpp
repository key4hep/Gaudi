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
// ===========================================================================
// Include files
// ===========================================================================
// STD&STL
// ===========================================================================
#include <array>
#include <iostream>
#include <sstream>
// ===========================================================================
// PartProp
// ===========================================================================
#include "GaudiPartProp/ParticleID.h"
// ===========================================================================
/** @file ParticleID.cpp
 *  Implementation file for class ParticleID.
 *  @date 19/02/2002
 *  @author Gloria Corti
 *  @date 22/03/2018
 *  @author Philip Ilten
 */
// ============================================================================
// local anonymous namespace to hide all helper functions
namespace {
  // ==========================================================================
  typedef std::array<unsigned int, 8> DATA;
  // ==========================================================================
  inline DATA decode( const Gaudi::ParticleID& pid ) {
    unsigned int m_extra = pid.abspid() / 10000000;
    unsigned int m_n     = pid.digit_<Gaudi::ParticleID::n>();
    unsigned int m_nr    = pid.digit_<Gaudi::ParticleID::nr>();
    unsigned int m_nl    = pid.digit_<Gaudi::ParticleID::nl>();
    unsigned int m_nq1   = pid.digit_<Gaudi::ParticleID::nq1>();
    unsigned int m_nq2   = pid.digit_<Gaudi::ParticleID::nq2>();
    unsigned int m_nq3   = pid.digit_<Gaudi::ParticleID::nq3>();
    unsigned int m_nj    = pid.digit_<Gaudi::ParticleID::nj>();
    unsigned int aid     = pid.abspid();

    // Switch gluons to 0.
    if ( m_nq1 == 9 ) m_nq1 = 0;
    // Special particles with 99 code.
    if ( m_n == 9 && m_nr == 9 ) {
      // Translate the color-octet quarkonia numbering scheme from Pythia.
      if ( m_nl > 3 ) {
        std::swap( m_nl, m_nq3 );
        m_nj  = m_nq1 % 2 ? 1 : 3;
        m_nq1 = 0;
        m_nq2 = m_nq3;
      }
      // Handle the X(3872) states.
      else if ( m_nl != 0 ) {
        m_nr = 0;
      }
      // Map left-right symmetry for double charged Higgses from Pythia.
      else if ( !m_nq2 && m_nq3 > 2 ) {
        m_nq3 = 0;
        m_nj  = 9;
      }
    }
    // The PDG hidden valley scheme.
    else if ( m_n == 4 && m_nr == 9 && m_nq2 ) {
      m_nq3 = m_nq2;
    }
    // The K0S/L and B0L/H.
    else if ( aid == 130 || aid == 310 || aid == 150 || aid == 510 ) {
      m_nl = 1;
      m_nj = 1;
    }
    // Technical photons.
    else if ( aid == 10022 || aid == 20022 ) {
      m_nl = 0;
    }
    // Old heavy ion convention for hydrogen-2 nucleus.
    else if ( m_extra == 45 ) {
      m_extra = 100;
      m_n     = 0;
      m_nr    = 0;
      m_nl    = 1;
      m_nq1   = 0;
      m_nq2   = 0;
      m_nq3   = 2;
      m_nj    = 0;
    }
    // Old heavy ion convention for hydrogen-3 nucleus.
    else if ( m_extra == 46 ) {
      m_extra = 100;
      m_n     = 0;
      m_nr    = 0;
      m_nl    = 1;
      m_nq1   = 0;
      m_nq2   = 0;
      m_nq3   = 3;
      m_nj    = 0;
    }
    // Old heavy ion convention for helium-4 nucleus.
    else if ( m_extra == 47 ) {
      m_extra = 100;
      m_n     = 0;
      m_nr    = 0;
      m_nl    = 2;
      m_nq1   = 0;
      m_nq2   = 0;
      m_nq3   = 4;
      m_nj    = 0;
    }
    // Old heavy ion convention for helium-3 nucleus.
    else if ( m_extra == 49 ) {
      m_extra = 100;
      m_n     = 0;
      m_nr    = 0;
      m_nl    = 2;
      m_nq1   = 0;
      m_nq2   = 0;
      m_nq3   = 3;
      m_nj    = 0;
    }
    // The EvtGen particles Xsd (30343), Xsu (30353), and Xss (30363)
    // which break the PDG conventions.
    else if ( aid == 30343 ) {
      m_nq1 = 2;
      m_nq2 = 3;
      m_nq3 = 1;
    } else if ( aid == 30353 ) {
      m_nq1 = 2;
      m_nq2 = 3;
      m_nq3 = 2;
    } else if ( aid == 30363 ) {
      m_nq1 = 2;
      m_nq2 = 3;
      m_nq3 = 3;
    }
    // Specific BSM particles.
    else if ( aid == 1009002 ) {
      m_nq1 = 0;
      m_nq3 = 2;
      m_nj  = 1;
    } else if ( aid == 1000542 ) {
      m_nq1 = 5;
      m_nq2 = 5;
      m_nq3 = 5;
    } else if ( aid == 1000522 ) {
      m_nq1 = 1;
      m_nq2 = 1;
      m_nq3 = 1;
    } else if ( aid == 1000039 ) {
      m_n   = 9;
      m_nr  = 9;
      m_nq3 = 1;
      m_nj  = 0;
    } else if ( aid == 3100021 ) {
      m_nq1 = 1;
      m_nq2 = 1;
      m_nq3 = 2;
    }
    //
    return { m_extra, m_nj, m_nq3, m_nq2, m_nq1, m_nl, m_nr, m_n };
  }
  // ==========================================================================
} // namespace
// ============================================================================
// Return everything beyond the 7th PDG ID digit.
// ============================================================================
int Gaudi::ParticleID::extraBits() const {
  const unsigned int m_extra = abspid() / 10000000;
  return 45 == m_extra ? 100u : 46 == m_extra ? 100u : 47 == m_extra ? 100u : 49 == m_extra ? 100u : m_extra;
}
// ============================================================================
/*  Return the fundamental ID.
 *  This is 0 for nuclie, mesons, baryons, and di-quarks.
 *  Otherwise, this is the first two digits of the PDG ID
 */
// ============================================================================
int Gaudi::ParticleID::fundamentalID() const {
  //
  DATA               data    = decode( *this );
  const unsigned int m_extra = data[0];
  const unsigned int m_nl    = data[Gaudi::ParticleID::nl];
  const unsigned int m_nq1   = data[Gaudi::ParticleID::nq1];
  const unsigned int m_nq2   = data[Gaudi::ParticleID::nq2];
  const unsigned int m_nq3   = data[Gaudi::ParticleID::nq3];
  const unsigned int m_nj    = data[Gaudi::ParticleID::nj];
  //
  return !m_extra && !( m_nl + m_nq1 + m_nq2 ) ? 10 * m_nq3 + m_nj : 0;
}
// ============================================================================
// Return if the PID is valid.
// ============================================================================
bool Gaudi::ParticleID::isValid() const {
  // Nuclei, geantino, and intermediate.
  if ( extraBits() ) {
    return m_pid == 480000000 || m_pid == -990000000 || isNucleus();
  }
  // Standard model particles.
  else if ( abspid() < 100 || isHadron() || isDiQuark() ) {
    return true;
  }
  // Beyond the standard model particles.
  else {
    return !isSM();
  }
}
// ============================================================================
// Return if the PID is from the standard model.
// ============================================================================
bool Gaudi::ParticleID::isSM() const {
  DATA               data = decode( *this );
  const unsigned int m_n  = data[Gaudi::ParticleID::n];
  const unsigned int m_nr = data[Gaudi::ParticleID::nr];
  //
  return m_n < 1 || ( m_n == 9 && m_nr != 9 );
}
// ============================================================================
// Return if the PID is for a meson.
// ============================================================================
bool Gaudi::ParticleID::isMeson() const {
  //
  if ( !isSM() ) { return false; }
  //
  DATA               data  = decode( *this );
  const unsigned int m_nq1 = data[Gaudi::ParticleID::nq1];
  const unsigned int m_nq2 = data[Gaudi::ParticleID::nq2];
  const unsigned int m_nq3 = data[Gaudi::ParticleID::nq3];
  const unsigned int m_nj  = data[Gaudi::ParticleID::nj];
  //
  return m_nq1 == 0 && m_nq2 != 0 && m_nq3 != 0 && m_nj != 0 && !( pid() < 0 && m_nq2 == m_nq3 );
}
// ============================================================================
// Return if the PID is for a baryon.
// ============================================================================
bool Gaudi::ParticleID::isBaryon() const {
  //
  if ( !isSM() ) { return false; }
  //
  DATA               data  = decode( *this );
  const unsigned int m_nq1 = data[Gaudi::ParticleID::nq1];
  const unsigned int m_nq2 = data[Gaudi::ParticleID::nq2];
  const unsigned int m_nq3 = data[Gaudi::ParticleID::nq3];
  const unsigned int m_nj  = data[Gaudi::ParticleID::nj];
  //
  return m_nq1 != 0 && m_nq2 != 0 && m_nq3 != 0 && m_nj != 0;
}
// ============================================================================
// Return if the PID is for a di-quark.
// ============================================================================
bool Gaudi::ParticleID::isDiQuark() const {
  //
  if ( !isSM() ) { return false; }
  //
  DATA               data  = decode( *this );
  const unsigned int m_nq1 = data[Gaudi::ParticleID::nq1];
  const unsigned int m_nq2 = data[Gaudi::ParticleID::nq2];
  const unsigned int m_nq3 = data[Gaudi::ParticleID::nq3];
  const unsigned int m_nj  = data[Gaudi::ParticleID::nj];
  //
  return m_nq1 != 0 && m_nq2 != 0 && m_nq3 == 0 && m_nj != 0 && !( m_nj == 1 && m_nq1 == m_nq2 );
}
// ============================================================================
// Return if the PID is for a hadron.
// ============================================================================
bool Gaudi::ParticleID::isHadron() const { return isMeson() || isBaryon(); }
// ============================================================================
// Return if the PID is for a lepton.
// ============================================================================
bool Gaudi::ParticleID::isLepton() const {
  const unsigned int apid = abspid();
  return 11 <= apid && apid <= 18;
}
// ============================================================================
// Return if the PID is for a nucleus.
// ============================================================================
bool Gaudi::ParticleID::isNucleus() const { return 100 <= extraBits() && Z() <= A(); }
// ============================================================================
// Return if the PID is for a bare quark.
// ============================================================================
bool Gaudi::ParticleID::isQuark() const {
  const unsigned int apid = abspid();
  return Gaudi::ParticleID::Quark::first <= apid && apid <= Gaudi::ParticleID::Quark::last;
}
// ============================================================================
// Return if the PID is a particle with quarks, but not a nucleus.
// ============================================================================
bool Gaudi::ParticleID::hasQuarks() const { return !extraBits() && !fundamentalID(); }
// ============================================================================
// Return if the PID is a particle containing a specified quark flavor.
// ============================================================================
bool Gaudi::ParticleID::hasQuark( const Quark& q ) const {
  if ( abspid() == q ) { return true; }
  if ( !hasQuarks() ) { return false; }
  //
  DATA               data  = decode( *this );
  const unsigned int m_nq1 = data[Gaudi::ParticleID::nq1];
  const unsigned int m_nq2 = data[Gaudi::ParticleID::nq2];
  const unsigned int m_nq3 = data[Gaudi::ParticleID::nq3];
  //
  return q == m_nq3 || q == m_nq2 || q == m_nq1;
}
// ============================================================================
// Return three times the charge, in units of e+, valid for all particles.
// ============================================================================
int Gaudi::ParticleID::threeCharge() const {
  // Lookup table for charges. The Xu+ (43) and Xu0 (44) break the PDG
  // conventions and are EvtGen specific. The c-hadron (84), b-hadron
  // (85), t-hadron (86), b'-hadron (87), are LHCb specific but within
  // the acceptable 81 - 100 range. A double charged Higgs is inserted
  // as 9 and a gravintino as 10.
  static const int   q[101] = { 0,  -1, 2,  -1, 2,  -1, 2,  -1, 2, 6, 0, // 1  - 10.
                                -3, 0,  -3, 0,  -3, 0,  -3, 0,  0, 0,    // 11 - 20.
                                0,  0,  0,  3,  0,  0,  0,  0,  0, 0,    // 21 - 30.
                                0,  0,  0,  3,  0,  0,  3,  0,  0, 0,    // 31 - 40.
                                0,  -1, 0,  3,  0,  0,  0,  0,  0, 0,    // 41 - 50.
                                0,  0,  0,  0,  0,  0,  0,  0,  0, 0,    // 51 - 60.
                                0,  0,  0,  0,  0,  0,  0,  0,  0, 0,    // 61 - 70.
                                0,  0,  0,  0,  0,  0,  0,  0,  0, 0,    // 71 - 80.
                                0,  0,  0,  2,  -1, 2,  -1, 0,  0, 0,    // 81 - 90.
                                0,  0,  0,  0,  0,  0,  0,  0,  0, 0 };  // 91 - 100.
  const unsigned int fid    = fundamentalID();
  int                charge = 0;
  //
  DATA               data  = decode( *this );
  const unsigned int m_nq1 = data[Gaudi::ParticleID::nq1];
  const unsigned int m_nq2 = data[Gaudi::ParticleID::nq2];
  const unsigned int m_nq3 = data[Gaudi::ParticleID::nq3];
  //
  // Nuclie.
  if ( isNucleus() ) {
    charge = 3 * Z();
  }
  // Fundamental particles from the charge table.
  else if ( fid ) {
    charge = q[fid];
  }
  // Mesons.
  else if ( !m_nq1 ) {
    charge = abs( q[m_nq2] - q[m_nq3] );
  }
  // Everything else.
  else {
    charge = q[m_nq3] + q[m_nq2] + q[m_nq1];
  }
  //
  return ( pid() < 0 ? -1 : 1 ) * charge;
}
// ============================================================================
// Return 2J+1, where J is the total spin, valid for all particles.
// ============================================================================
int Gaudi::ParticleID::jSpin() const {
  // Lookup table for spins. The Xu+ (43) and Xu0 (44) break the PDG
  // conventions and are EvtGen specific. The c-hadron (84), b-hadron
  // (85), t-hadron (86), b'-hadron (87), are LHCb specific but within
  // the acceptable 81 - 100 range. A double charged Higgs is inserted
  // as 9 and a gravintino is inserted as 10.
  static const unsigned int j[101] = { 0, 2, 2, 2, 2, 2, 2, 2, 2, 1, 4, // 1  - 10.
                                       2, 2, 2, 2, 2, 2, 2, 2, 0, 0,    // 11 - 20.
                                       3, 3, 3, 3, 1, 0, 0, 0, 0, 0,    // 21 - 30.
                                       0, 3, 3, 3, 1, 1, 1, 1, 5, 0,    // 31 - 40.
                                       3, 1, 0, 0, 1, 1, 0, 0, 0, 0,    // 41 - 50.
                                       1, 2, 3, 1, 3, 1, 1, 1, 1, 1,    // 51 - 60.
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    // 61 - 70.
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    // 71 - 80.
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    // 81 - 90.
                                       0, 0, 3, 0, 0, 0, 0, 0, 0, 0 };  // 91 - 100.
  const unsigned int        fid    = fundamentalID();
  int                       spin   = 0;
  //
  DATA               data  = decode( *this );
  const unsigned int m_n   = data[Gaudi::ParticleID::n];
  const unsigned int m_nq3 = data[Gaudi::ParticleID::nq3];
  const unsigned int m_nj  = data[Gaudi::ParticleID::nj];
  //
  // Fundamental particles from the spin table.
  if ( fid ) {
    // Supersymmetric particles.
    if ( m_n == 1 || m_n == 2 || m_n == 3 ) {
      spin = j[fid] % 2 + 1;
    }
    // Standard particles.
    else {
      spin = j[fid];
    }
  }
  // Nuclei.
  else if ( isNucleus() ) {
    spin = m_nq3 % 2 + 1;
  }
  // Everything else.
  else {
    spin = m_nj;
  }
  //
  return spin;
}

// ============================================================================
// Return 2S+1, where S is the spin, valid only for mesons.
// ============================================================================
int Gaudi::ParticleID::sSpin() const {
  //
  if ( !isMeson() ) { return 0; }
  //
  DATA               data = decode( *this );
  const unsigned int m_n  = data[Gaudi::ParticleID::n];
  const unsigned int m_nl = data[Gaudi::ParticleID::nl];
  const unsigned int m_nj = data[Gaudi::ParticleID::nj];
  //
  if ( m_n == 9 ) {
    return 0;
  } else if ( m_nj == 1 ) {
    return m_nl;
  } else if ( m_nl == 1 ) {
    return 0;
  } else {
    return 1;
  }
}

// ============================================================================
// Return 2L+1, where L is the orbital angular momentum, valid only for mesons.
// ============================================================================
int Gaudi::ParticleID::lSpin() const {
  //
  if ( !isMeson() ) { return 0; }
  //
  DATA               data = decode( *this );
  const unsigned int m_n  = data[Gaudi::ParticleID::n];
  const unsigned int m_nl = data[Gaudi::ParticleID::nl];
  const unsigned int m_nj = data[Gaudi::ParticleID::nj];
  //
  if ( m_n == 9 ) {
    return 0;
  } else if ( m_nj == 1 ) {
    return m_nl;
  } else if ( m_nl == 0 ) {
    return ( m_nj - 3 ) / 2;
  } else if ( m_nl == 3 ) {
    return ( m_nj + 1 ) / 2;
  } else {
    return ( m_nj - 1 ) / 2;
  }
}
// ============================================================================
// Return the atomic number for a nucleus.
// ============================================================================
int Gaudi::ParticleID::Z() const {
  if ( extraBits() < 100 ) { return 0; }
  //
  DATA data = decode( *this );
  //
  const unsigned int m_n  = data[Gaudi::ParticleID::n];
  const unsigned int m_nr = data[Gaudi::ParticleID::nr];
  const unsigned int m_nl = data[Gaudi::ParticleID::nl];
  //
  return m_n * 100 + m_nr * 10 + m_nl;
}
// ============================================================================
// Return the nucleon number for a nucleus.
// ============================================================================
int Gaudi::ParticleID::A() const {
  if ( extraBits() < 100 ) { return 0; }
  //
  DATA               data  = decode( *this );
  const unsigned int m_nq1 = data[Gaudi::ParticleID::nq1];
  const unsigned int m_nq2 = data[Gaudi::ParticleID::nq2];
  const unsigned int m_nq3 = data[Gaudi::ParticleID::nq3];
  //
  return m_nq1 * 100 + m_nq2 * 10 + m_nq3;
}
// ============================================================================
// Return the number of strange quarks for a nucleus.
// ============================================================================
int Gaudi::ParticleID::nLambda() const {
  const unsigned int m_extra = extraBits();
  return 100 <= m_extra ? m_extra - 100 : 0;
}
// ============================================================================
// Fill a stream with the PID.
// ============================================================================
std::ostream& Gaudi::ParticleID::fillStream( std::ostream& s ) const {
  return s << "Gaudi.ParticleID(" << m_pid << ")";
}
// ============================================================================
// Return the PID stream representation as a string.
// ============================================================================
std::string Gaudi::ParticleID::toString() const {
  std::ostringstream s;
  fillStream( s );
  return s.str();
}

// ============================================================================
// Fill a stream with the PID digit enumeration.
// ============================================================================
std::ostream& Gaudi::ParticleID::printLocation( const long l, std::ostream& s ) {
  switch ( l ) {
  case Gaudi::ParticleID::nj:
    return s << "Gaudi.ParticleID.nj";
  case Gaudi::ParticleID::nq3:
    return s << "Gaudi.ParticleID.nq3";
  case Gaudi::ParticleID::nq2:
    return s << "Gaudi.ParticleID.nq2";
  case Gaudi::ParticleID::nq1:
    return s << "Gaudi.ParticleID.nq1";
  case Gaudi::ParticleID::nl:
    return s << "Gaudi.ParticleID.nl";
  case Gaudi::ParticleID::nr:
    return s << "Gaudi.ParticleID.nr";
  case Gaudi::ParticleID::n:
    return s << "Gaudi.ParticleID.n";
  case Gaudi::ParticleID::n8:
    return s << "Gaudi.ParticleID.n8";
  case Gaudi::ParticleID::n9:
    return s << "Gaudi.ParticleID.n9";
  case Gaudi::ParticleID::n10:
    return s << "Gaudi.ParticleID.n10";
  default:;
  }
  return s << "Gaudi.ParticleID.Location(" << l << ")";
}

// ============================================================================
// Return the PID digit enumeration stream representation as a string.
// ============================================================================
std::string Gaudi::ParticleID::printLocation( const long l ) {
  std::ostringstream s;
  printLocation( l, s );
  return s.str();
}

// ============================================================================
// Fill a stream with the PID quark enumeration.
// ============================================================================
std::ostream& Gaudi::ParticleID::printQuark( const long q, std::ostream& s ) {
  switch ( q ) {
  case Gaudi::ParticleID::down:
    return s << "Gaudi.ParticleID.down";
  case Gaudi::ParticleID::up:
    return s << "Gaudi.ParticleID.up";
  case Gaudi::ParticleID::strange:
    return s << "Gaudi.ParticleID.strange";
  case Gaudi::ParticleID::charm:
    return s << "Gaudi.ParticleID.charm";
  case Gaudi::ParticleID::bottom:
    return s << "Gaudi.ParticleID.bottom";
  case Gaudi::ParticleID::top:
    return s << "Gaudi.ParticleID.top";
  case Gaudi::ParticleID::bottom_prime:
    return s << "Gaudi.ParticleID.bottom_prime";
  case Gaudi::ParticleID::top_prime:
    return s << "Gaudi.ParticleID.top_prime";
  default:;
  }
  return s << "Gaudi.ParticleID.Quark(" << q << ")";
}

// ============================================================================
// Return the PID quark enumeration stream representation as a string.
// ============================================================================
std::string Gaudi::ParticleID::printQuark( const long q ) {
  std::ostringstream s;
  printQuark( q, s );
  return s.str();
}

// ============================================================================
// The END
// ============================================================================
