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
#pragma once

#include <GaudiKernel/HashMap.h>
#include <GaudiKernel/Kernel.h>
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <tuple>

namespace Gaudi {
  /** @class ParticleID ParticleID.h
   *
   * Holds PDG + LHCb extension particle code, following the PDG
   * particle numbering scheme
   * (pdg.lbl.gov/2017/reviews/rpp2017-rev-monte-carlo-numbering.pdf). Specific
   * conventions followed by Pythia 8 for beyond the standard model
   * physics and color-octet quarkonia have been introduced.
   *
   * Nuclei with the PDG 2017 convention (following the 2006 Monte
   * Carlo nuclear code scheme) have the numbering +/- 10LZZZAAAI. Where
   * AAA is A - the total baryon number,
   * ZZZ is Z - the total number of protons,
   * L is the total number of strange quarks, and
   * I is the isomer number where I = 0 corresponds to the ground state.
   * Backwards compatibility with the old heavy ion scheme has also been kept.
   *
   * @date 19/02/2002
   * @author Gloria Corti
   * @date 22/03/2018
   * @author Philip Ilten
   */
  class GAUDI_API ParticleID final {
  public:
    /// PDG ID digits (base 10) are: n nr nl nq1 ne2 nq3 nj.
    enum Location { nj = 1, nq3, nq2, nq1, nl, nr, n, n8, n9, n10 };
    /// Quark PDG IDs.
    enum Quark { down = 1, up, strange, charm, bottom, top, bottom_prime, top_prime, first = down, last = top_prime };

    /// Constructor with PDG code.
    explicit ParticleID( const int pid = 0 ) { setPid( pid ); }

    /// Retrieve the PDG ID.
    int pid() const { return m_pid; }
    /// Absolute value of the PDG ID.
    constexpr unsigned int abspid() const { return 0 > m_pid ? -m_pid : m_pid; }
    /// Update the PDG ID.
    void setPid( const int pid ) { m_pid = pid; }

    /// Return if the PID is valid.
    bool isValid() const;
    /// Return if the PID is from the standard model.
    bool isSM() const;
    /// Return if the PID is for a meson.
    bool isMeson() const;
    /// Return if the PID is for a baryon.
    bool isBaryon() const;
    /// Return if the PID is for a di-quark.
    bool isDiQuark() const;
    /// Return if the PID is for a hadron.
    bool isHadron() const;
    /// Return if the PID is for a lepton.
    bool isLepton() const;
    /// Return if the PID is for a nucleus.
    bool isNucleus() const;
    /// Return if the PID is for a bare quark.
    bool isQuark() const;

    /// Return if the PID is a particle with quarks, but not a nucleus.
    bool hasQuarks() const;
    /// Return if the PID is a particle containing a specified quark flavor.
    bool hasQuark( const Quark& q ) const;
    /// Return if the PID is a particle with a down quark.
    bool hasDown() const { return hasQuark( down ); }
    /// Return if the PID is a particle with an up quark.
    bool hasUp() const { return hasQuark( up ); }
    /// Return if the PID is a particle with a down quark.
    bool hasStrange() const { return hasQuark( strange ); }
    /// Return if the PID is a particle with a charm quark.
    bool hasCharm() const { return hasQuark( charm ); }
    /// Return if the PID is a particle with a bottom quark.
    bool hasBottom() const { return hasQuark( bottom ); }
    /// Return if the PID is a particle with a top quark.
    bool hasTop() const { return hasQuark( top ); }
    /// Return if the PID is a particle with a bottom' quark.
    bool hasBottomPrime() const { return hasQuark( bottom_prime ); }
    /// Return if the PID is a particle with a top' quark.
    bool hasTopPrime() const { return hasQuark( top_prime ); }

    /// Return three times the charge, in units of e+, valid for all particles.
    int threeCharge() const;
    /// Return 2J+1, where J is the total spin, valid for all particles.
    int jSpin() const;
    /// Return 2S+1, where S is the spin, valid only for mesons.
    int sSpin() const;
    /// Return 2L+1, where L is the orbital angular momentum, valid only for mesons.
    int lSpin() const;

    /// Return the atomic number for a nucleus.
    int Z() const;
    /// Return the nucleon number for a nucleus.
    int A() const;
    /// Return the number of strange quarks for a nucleus.
    int nLambda() const;

    /** Return the fundamental ID.
     *  This is 0 for nuclei, mesons, baryons, and di-quarks.
     *  Otherwise, this is the first two digits of the PDG ID
     */
    int fundamentalID() const;

    /// Return everything beyond the 7th PDG ID digit.
    int extraBits() const;
    /// Return the digit for a given PDG ID digit location.
    constexpr unsigned short digit( const Location& loc ) const {
      constexpr std::uint32_t pows[10] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
      return ( abspid() / pows[loc - 1] ) % 10;
    }

    /// Equality operator.
    bool operator==( const ParticleID& o ) const { return m_pid == o.m_pid; }
    /// Non-equality operator.
    bool operator!=( const ParticleID& o ) const { return m_pid != o.m_pid; }
    /// Comparison operator.
    bool operator<( const ParticleID& o ) const {
      const unsigned int i1( abspid() ), i2( o.abspid() );
      return std::tie( i1, m_pid ) < std::tie( i2, o.m_pid );
    }

    /// Fill a stream with the PID.
    std::ostream& fillStream( std::ostream& s ) const;
    /// Return the PID stream representation as a string.
    std::string toString() const;
    /// Fill a stream with the PID digit enumeration.
    static std::ostream& printLocation( const long l, std::ostream& s );
    /// Return the PID digit enumeration stream representation as a string.
    static std::string printLocation( const long l );
    /// Fill a stream with the PID quark enumeration.
    static std::ostream& printQuark( const long q, std::ostream& s );
    /// Return the PID quark enumeration stream representation as a string.
    static std::string printQuark( const long q );

  private:
    /// PDG ID.
    int m_pid{ 0 };
  };

  // Inline stream operators.
  /// Stream operator for the PID.
  inline std::ostream& operator<<( std::ostream& s, const Gaudi::ParticleID& o ) { return o.fillStream( s ); }
  /// Stream operator for the PDG digit enumeration.
  inline std::ostream& operator<<( std::ostream& s, Gaudi::ParticleID::Location l ) {
    return Gaudi::ParticleID::printLocation( l, s );
  }
  /// Stream operator for the PDG quark enumeration.
  inline std::ostream& operator<<( std::ostream& s, Gaudi::ParticleID::Quark q ) {
    return Gaudi::ParticleID::printQuark( q, s );
  }
} // namespace Gaudi

namespace GaudiUtils {
  template <>
  struct Hash<Gaudi::ParticleID> {
    inline size_t operator()( const Gaudi::ParticleID& s ) const { return (size_t)s.pid(); }
  };
  template <>
  struct Hash<const Gaudi::ParticleID> {
    inline size_t operator()( const Gaudi::ParticleID& s ) const { return (size_t)s.pid(); }
  };
  template <>
  struct Hash<Gaudi::ParticleID&> {
    inline size_t operator()( const Gaudi::ParticleID& s ) const { return (size_t)s.pid(); }
  };
  template <>
  struct Hash<const Gaudi::ParticleID&> {
    inline size_t operator()( const Gaudi::ParticleID& s ) const { return (size_t)s.pid(); }
  };
} // namespace GaudiUtils
namespace std {
  /// Return the absolute value for a PID.
  inline Gaudi::ParticleID abs( const Gaudi::ParticleID& p ) { return Gaudi::ParticleID( p.abspid() ); }
} // namespace std
