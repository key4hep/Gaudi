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
#ifndef PARTPROP_NODESPID_H
#define PARTPROP_NODESPID_H 1
// ============================================================================
// Include files
// ============================================================================
// PartProp
// ============================================================================
#include "GaudiKernel/MsgStream.h"
#include "GaudiPartProp/IParticlePropertySvc.h"
#include "GaudiPartProp/Nodes.h"
#include "GaudiPartProp/ParticleProperty.h"
#include "GaudiPartProp/SynchronizedValue.h"

// ============================================================================
/** @file  Kernel/NodePIDs.h
 *  The actual implementation of various decay nodes
 *  @see Decays::iNode
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-04-21
 */
// ============================================================================
namespace Decays {
  // ==========================================================================
  namespace Nodes {
    // ========================================================================
    /** @class Any
     *  the most simple node in the decay tree:
     *  it matches to all valid the Gaudi::Particles
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Any : public Decays::iNode {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Any* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& /* p */ ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Pid
     *  The simple node in the decay tree:
     *  it matches to a certain particle ID
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Pid : public Decays::iNode {
    public:
      // ======================================================================
      /// constructor from the decay item
      Pid( const Decays::Decay::Item& item );
      /// constructor from Particle Property
      Pid( const Gaudi::ParticleProperty* item );
      /// constructor from ParticleID
      Pid( const Gaudi::ParticleID& item );
      /// constructor from Particle name
      Pid( const std::string& item );
      /// MANDATORY: clone method ("virtual constructor")
      Pid* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return check( pid ); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    public:
      // ======================================================================
      bool check( Gaudi::ParticleID pid ) const { return pid == m_item.pid(); }
      // ======================================================================
    public:
      // ======================================================================
      /// get DaVinci Decay item
      const Decays::Decay::Item& item() const { return m_item; }
      /// cast to DaVinci decay item
      operator const Decays::Decay::Item&() const { return item(); }
      // ======================================================================
    private:
      // ======================================================================
      /// the decay item itself
      Decays::Decay::Item m_item; // the decay item
      // ======================================================================
    };
    // ========================================================================
    /** @class CC
     *  The simple node in the decay tree:
     *  it matches to a certain particle ID or its antiparticle
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API CC : public Pid {
    public:
      // ======================================================================
      /// constructor from the decay item
      CC( const Decays::Decay::Item& item );
      /// constructor from Particle Property
      CC( const Gaudi::ParticleProperty* item );
      /// constructor from ParticleID
      CC( Gaudi::ParticleID item );
      /// constructor from Particle name
      CC( const std::string& item );
      /// constructor from node
      CC( const Decays::Nodes::Pid& pid );
      /// MANDATORY: clone method ("virtual constructor")
      CC* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return pid.abspid() == item().pid().abspid(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Lepton
     *  The trivial node : it match the Lepton
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Lepton : public Any {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Lepton* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return pid.isLepton(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Nu
     *  The trivial node : it match any neutral lepton
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Nu : public Lepton {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Nu* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override {
        return pid.isLepton() && ( 0 == pid.threeCharge() );
      }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Ell
     *  The trivial node : it match any charged lepton
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Ell : public Lepton {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Ell* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override {
        return pid.isLepton() && ( 0 != pid.threeCharge() );
      }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class EllPlus
     *  The trivial node : it match any positive lepton
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API EllPlus : public Ell {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      EllPlus* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override {
        return pid.isLepton() && ( 0 < pid.threeCharge() );
      }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class EllMinus
     *  The trivial node : it match any negative lepton
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API EllMinus : public Ell {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      EllMinus* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override {
        return pid.isLepton() && ( 0 > pid.threeCharge() );
      }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Hadron
     *  The trivial node : it match the Hadron
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Hadron : public Any {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Hadron* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return pid.isHadron(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Meson
     *  The trivial node : it match the meson
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Meson : public Hadron {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Meson* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return pid.isMeson(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Baryon
     *  The trivial node : it match the Baryon
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Baryon : public Hadron {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Baryon* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return pid.isBaryon(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Charged
     *  The trivial node : it match the Charged
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Charged : public Any {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Charged* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return 0 != pid.threeCharge(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Positive
     *  The trivial node : it match the positively charged  particles
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Positive : public Charged {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Positive* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return 0 < pid.threeCharge(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Negative
     *  The trivial node : it match the negatively charged  particles
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Negative : public Charged {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Negative* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return 0 > pid.threeCharge(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Neutral
     *  The trivial node : it match the Neutral
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Neutral : public Charged {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Neutral* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return 0 == pid.threeCharge(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Nucleus
     *  The trivial node : it match the Nucleus
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Nucleus : public Any {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Nucleus* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return pid.isNucleus(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class HasQuark
     *  The trivial node : it match the quark content
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API HasQuark : public Any {
    public:
      // ======================================================================
      /// constructor from the quark
      HasQuark( Gaudi::ParticleID::Quark quark );
      /// MANDATORY: clone method ("virtual constructor")
      HasQuark* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return pid.hasQuark( m_quark ); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled
      HasQuark(); // the default constructro is disabled
      // ======================================================================
    private:
      // ======================================================================
      /// the quark to be tested
      Gaudi::ParticleID::Quark m_quark; // the quark to be tested
      // ======================================================================
    };
    // ========================================================================
    /** @class JSpin
     *  The trivial node : it match the 2J+1 spin
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API JSpin : public Any {
    public:
      // ======================================================================
      enum { InvalidSpin = 501 };
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from the 2J+1
      JSpin( const int spin );
      /// MANDATORY: clone method ("virtual constructor")
      JSpin* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return spin() == pid.jSpin(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    public:
      // ======================================================================
      /// valid only for positive spin-values
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::IParticlePropertySvc* /* svc */ ) const override;
      // ======================================================================
    public:
      // ======================================================================
      int spin() const { return m_spin; }
      // ======================================================================
    private:
      // ======================================================================
      /// the j-spin to be tested
      int m_spin; // the spin to be tested
      // ======================================================================
    };
    // ========================================================================
    /** @class SSpin
     *  The trivial node : it match the 2S+1 spin
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API SSpin : public JSpin {
    public:
      // ======================================================================
      /// constructor from the 2S+1
      SSpin( const int spin );
      /// MANDATORY: clone method ("virtual constructor")
      SSpin* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return spin() == pid.sSpin(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class LSpin
     *  The trivial node : it match the 2L+1 spin
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API LSpin : public SSpin {
    public:
      // ======================================================================
      /// constructor from the 2L+1
      LSpin( const int spin );
      /// MANDATORY: clone method ("virtual constructor")
      LSpin* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override { return spin() == pid.lSpin(); }
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class CTau
     *  simple pid-checker for particle lifetime (in c*tau units)
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-05-11
     */
    class GAUDI_API CTau : public Decays::iNode {
    public:
      // ======================================================================
      /// constructor from c-tau range
      CTau( const double low, const double high, const Gaudi::IParticlePropertySvc* svc = nullptr );
      /// MANDATORY: clone method ("virtual constructor")
      CTau* clone() const override;
      // ======================================================================
    public:
      // ======================================================================
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    public:
      // ======================================================================
      const Gaudi::IParticlePropertySvc* ppSvc() const { return m_ppSvc; }
      // ======================================================================
      /// get low  edge
      double low() const { return m_low; }
      /// get high edge
      double high() const { return m_high; }
      // ======================================================================
      StatusCode setService( const Gaudi::IParticlePropertySvc* svc ) const;
      // ======================================================================
    protected:
      template <typename F, typename = std::is_invocable_r<bool, F, Gaudi::ParticleID>>
      bool classify( Gaudi::ParticleID pid, F&& f ) const {
        enum struct Classification { Accepted = +1, Rejected = -1, Unknown = 0 };
        auto r = m_pids.with_lock(
            []( const auto& pids, Gaudi::ParticleID pid ) {
              return std::binary_search( pids.accepted.begin(), pids.accepted.end(), pid )   ? Classification::Accepted
                     : std::binary_search( pids.rejected.begin(), pids.rejected.end(), pid ) ? Classification::Rejected
                                                                                             : Classification::Unknown;
            },
            pid );
        switch ( r ) {
        case Classification::Accepted:
          return true;
        case Classification::Rejected:
          return false;
        case Classification::Unknown:
          return add_( pid, std::invoke( std::forward<F>( f ), pid ) );
        }
        __builtin_unreachable();
      }

    private:
      bool add_( Gaudi::ParticleID pid, bool acc ) const;
      // ======================================================================
    public:
      MsgStream& printAcceptedAsTable( MsgStream& s ) const {
        return m_pids.with_lock(
            [&]( const KnownPids& pids, MsgStream& s, auto const& ppSvc ) -> decltype( auto ) {
              return Gaudi::ParticleProperties::printAsTable( pids.accepted, s, ppSvc );
            },
            s, m_ppSvc );
      }
      // ======================================================================
    private:
      // ======================================================================
      typedef SmartIF<Gaudi::IParticlePropertySvc> Service;
      /// the service :
      mutable Service m_ppSvc; // the service
      // ======================================================================
    public:
      struct KnownPids {
        std::vector<Gaudi::ParticleID> accepted; // the list of "accepted" pids
        std::vector<Gaudi::ParticleID> rejected; // the list of "rejected" pids
      };

    private:
      mutable Gaudi::cxx::SynchronizedValue<KnownPids> m_pids;
      // ======================================================================
      /// low edge
      double m_low; //  low edge
      /// high edge
      double m_high; // high edge
      // ======================================================================
    };
    // ========================================================================
    /** @class ShortLived_
     *  represent simple predicate for short-lived particles
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-05-11
     */
    class GAUDI_API ShortLived_ : public CTau {
    public:
      // ======================================================================
      /// constructor with high edge and service
      ShortLived_( const double high, const Gaudi::IParticlePropertySvc* svc = 0 );
      /// constructor service
      ShortLived_( const Gaudi::IParticlePropertySvc* svc = 0 );
      /// MANDATORY: clone method ("virtual constructor")
      ShortLived_* clone() const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class LongLived_
     *  represent simple predicate for Long-lived particles
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-05-11
     */
    struct GAUDI_API LongLived_ : CTau {
      // ======================================================================
      /// constructor with high edge and service
      LongLived_( const double high, const Gaudi::IParticlePropertySvc* svc = 0 );
      /// constructor service
      LongLived_( const Gaudi::IParticlePropertySvc* svc = 0 );
      /// MANDATORY: clone method ("virtual constructor")
      LongLived_* clone() const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Stable
     *  represent simple predicate for Stable particles
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-05-11
     */
    class GAUDI_API Stable : public LongLived_ {
    public:
      // ======================================================================
      /// constructor service
      Stable( const Gaudi::IParticlePropertySvc* svc = 0 );
      /// MANDATORY: clone method ("virtual constructor")
      Stable* clone() const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class StableCharged
     *  represent simple predicate for Stable+Charged particles
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-05-11
     */
    class GAUDI_API StableCharged : public Stable {
    public:
      // ======================================================================
      /// constructor service
      StableCharged( const Gaudi::IParticlePropertySvc* svc = 0 );
      /// MANDATORY: clone method ("virtual constructor")
      StableCharged* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class Mass
     *  simple pid-checker for particle mass
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-05-11
     */
    class GAUDI_API Mass : public CTau {
    public:
      // ======================================================================
      /// constructor from mass-range
      Mass( const double low, const double high, const Gaudi::IParticlePropertySvc* svc = 0 );
      /// MANDATORY: clone method ("virtual constructor")
      Mass* clone() const override;
      // ======================================================================
    public:
      // ======================================================================
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    private:
      /// the default constructor is disabled
      Mass(); // the default constructor is disabled
      // ======================================================================
    };
    // ========================================================================
    /** @class Light
     *  simple pid-checker for particle mass
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-05-11
     */
    class GAUDI_API Light : public Mass {
    public:
      // ======================================================================
      /// constructor from mass-range
      Light( const double high, const Gaudi::IParticlePropertySvc* svc = 0 );
      /// MANDATORY: clone method ("virtual constructor")
      Light* clone() const override;
      // ======================================================================
    public:
      // ======================================================================
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    private:
      /// the default constructor is disabled
      Light(); // the default constructor is disabled
      // ======================================================================
    };
    // ========================================================================
    /** @class Heavy
     *  simple pid-checker for particle mass
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-05-11
     */
    class GAUDI_API Heavy : public Mass {
    public:
      // ======================================================================
      /// constructor from mass-range
      Heavy( const double low, const Gaudi::IParticlePropertySvc* svc = 0 );
      /// MANDATORY: clone method ("virtual constructor")
      Heavy* clone() const override;
      // ======================================================================
    public:
      // ======================================================================
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      // ======================================================================
    private:
      /// the default constructor is disabled
      Heavy(); // the default constructor is disabled
      // ======================================================================
    };
    // ========================================================================
    /** @class PosID
     *  - It matches the particles with positive ID
     *  - Useful way to separate e.g. charm particle and charm antiparticle
     *  - Useful mainly in boolean operatios, e.g. <code>Charm & ID+ </code>
     *  - Charge conjugation could be fragile
     *  @author Vanya BELYAEV Ivan.Belyaev@iep.ru
     *  @date 2017-02-24
     */
    class GAUDI_API PosID : public Decays::iNode {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      PosID* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& /* p */ ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class NegID
     *  - It matches the particles with negative ID
     *  - Useful way to separate e.g. charm particle and charm antiparticle
     *  - Useful mainly in boolean operatios, e.g. <code>Charm & ID- </code>
     *  - Charge conjugation could be fragile
     *  @author Vanya BELYAEV Ivan.Belyaev@iep.ru
     *  @date 2017-02-24
     */
    class GAUDI_API NegID : public Decays::iNode {
    public:
      // ======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      NegID* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& /* p */ ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    };
    // ========================================================================
    class GAUDI_API Symbol : public Decays::iNode {
    public:
      // =======================================================================
      /// the constructor from the symbol
      Symbol( const std::string& sym );
      // =======================================================================
      /// MANDATORY: clone method ("virtual constructor")
      Symbol* clone() const override;
      // ======================================================================
    public:
      // ======================================================================
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled
      Symbol(); // the default constructor is disabled
      // ======================================================================
    private:
      // ======================================================================
      /// the actual symbol
      Decays::Node m_symbol;
      // ======================================================================
    };
    // ========================================================================
  } // namespace Nodes
  // ==========================================================================
} // namespace Decays
// ============================================================================
//                                                                      The END
// ============================================================================
#endif // PARTPROP_NODESPID_H
