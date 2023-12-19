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
#ifndef PARTPROP_DECAY_H
#define PARTPROP_DECAY_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <vector>
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiPartProp/ParticleID.h"
// ============================================================================
/// forward decalrations
// ============================================================================
namespace Gaudi {
  class IParticlePropertySvc;
  class ParticleProperty;
} // namespace Gaudi
// ============================================================================
namespace Decays {
  // ==========================================================================
  /** @class Decay Kernel/Decay.h
   *  The simple representation of "simple 1-step" decay (there are no trees!
   *
   *  @author  Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date   2008-03-31
   */
  class GAUDI_API Decay final {
  public:
    // ========================================================================
    /** @class Item
     *  the helper representation of the item in the decay chain
     *  @author  Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-03-31
     */
    class Item final {
    public:
      // ======================================================================
      /// the constructor from the particle property
      Item( const Gaudi::ParticleProperty* pp = 0 );
      /// the constructor from the particle name
      Item( const std::string& name );
      /// the constructor from the particle PID
      Item( const Gaudi::ParticleID& pid );
      // ======================================================================
    public:
      // ======================================================================
      /// get the particle name
      const std::string& name() const { return m_name; }
      /// get the particle PID
      const Gaudi::ParticleID& pid() const { return m_pid; }
      /// get the particle property
      const Gaudi::ParticleProperty* pp() const { return m_pp; }
      // ======================================================================
    public:
      // ======================================================================
      /// the default printout
      std::ostream& fillStream( std::ostream& s ) const;
      // ======================================================================
    public:
      // ======================================================================
      /// validate the item using the service
      StatusCode validate( const Gaudi::IParticlePropertySvc* svc ) const;
      /// validate the item using the particle property object
      StatusCode validate( const Gaudi::ParticleProperty* prop ) const;
      // ======================================================================
    private:
      // ======================================================================
      /// the particle name
      mutable std::string m_name; //    the particle name
      /// the particle PID
      mutable Gaudi::ParticleID m_pid; //     the particle PID
      /// the source of properties
      mutable const Gaudi::ParticleProperty* m_pp = nullptr; //    the property
      // ======================================================================
    };
    // ========================================================================
    /// the vector of items (the obvious representation of daughter particles)
    typedef std::vector<Item> Items;
    // ========================================================================
  public:
    // ========================================================================
    /// the default constructor
    Decay() = default;
    // ========================================================================
    /** the constructor from mother and daughters
     *  @param mother the mother
     *  @param daughters the daughters
     */
    Decay( const Gaudi::ParticleProperty* mother, const std::vector<const Gaudi::ParticleProperty*>& daughters );
    /** the constructor from mother and daughters
     *  @param mother the mother
     *  @param daughters the daughters
     */
    Decay( const std::string&              mother,      //    the mother
           const std::vector<std::string>& daughters ); // the daughters
    /** the constructor from mother and daughters
     *  @param mother the mother
     *  @param daughters the daughters
     */
    Decay( const Gaudi::ParticleID&              mother,      //    the mother
           const std::vector<Gaudi::ParticleID>& daughters ); // the daughters
    /** the constructor from mother and daughters
     *  @param mother the mother
     *  @param daughters the daughters
     */
    Decay( const Item&              mother,      //    the mother
           const std::vector<Item>& daughters ); // the daughters
    // ========================================================================
  public:
    // ========================================================================
    /// get the mother(head) of the decay
    const Item& mother() const { return m_mother; }
    /// get all daughters
    const Items& daughters() const { return m_daughters; }
    /// get all daughters
    const Items& children() const { return daughters(); }
    /// get the number of daughters
    size_t nDaughters() const { return m_daughters.size(); }
    /// get the number of daughters
    size_t nChildren() const { return m_daughters.size(); }
    /** get the component by the number
     *  @attention index 0 corresponds to the mother particle
     *  @param index the index (0 corresponds to the mother particle)
     *  @return the component
     */
    const Item& operator()( const unsigned int index ) const;
    /** get the component by the number
     *  @attention index 0 corresponds to th emother particle
     *  @param index the index (0 corresponds to the mother particle)
     *  @return the component
     */
    const Item& operator[]( const unsigned int index ) const { return ( *this )( index ); }
    // ========================================================================
  public:
    // ========================================================================
    /// set the mother
    void setMother( const Item& mom );
    /// set the mother
    void setMother( const Gaudi::ParticleProperty* mom );
    /// set the mother
    void setMother( const std::string& mom );
    /// set the mother
    void setMother( const Gaudi::ParticleID& mom );
    /// set the daughters
    void setDaughters( const Items& daugs );
    /// set the daughters
    void setDaughters( const std::vector<const Gaudi::ParticleProperty*>& daugs );
    /// set the daughters
    void setDaughters( const std::vector<std::string>& daugs );
    /// set the daughters
    void setDaughters( const std::vector<Gaudi::ParticleID>& daugs );
    /// set the daughters
    void setChildren( const Items& daugs ) { setDaughters( daugs ); }
    /// set the daughters
    void setChildren( const std::vector<const Gaudi::ParticleProperty*>& daugs ) { setDaughters( daugs ); }
    /// set the daughters
    void setChildren( const std::vector<std::string>& daugs ) { setDaughters( daugs ); }
    /// set the daughters
    void setChidlren( const std::vector<Gaudi::ParticleID>& daugs ) { setDaughters( daugs ); }
    // ========================================================================
  public:
    // ========================================================================
    /// add the child
    Decay& operator+=( const std::string& child ); // add child
    /// add the child
    Decay& operator+=( const Gaudi::ParticleID& child ); // add child
    /// add the child
    Decay& operator+=( const Gaudi::ParticleProperty* child ); // add child
    /// add the child
    Decay& operator+=( const Item& child ); // add child
    // ========================================================================
  public:
    // ========================================================================
    /// validate the decay using the service
    StatusCode validate( const Gaudi::IParticlePropertySvc* svc ) const;
    // ========================================================================
  public:
    // ========================================================================
    /// the default printout
    std::ostream& fillStream( std::ostream& s ) const;
    /// the conversion to the string
    std::string toString() const;
    // ========================================================================
  private:
    // ========================================================================
    /// the head of the decay
    mutable Item m_mother; // the head of the decay
    /// the daughter particles
    mutable Items m_daughters; // the daughter particles
    // ========================================================================
  };
  // ==========================================================================
} // namespace Decays
// ============================================================================
/// the printout operator to the stream
inline std::ostream& operator<<( std::ostream& s, const Decays::Decay& decay ) { return decay.fillStream( s ); }
// ============================================================================
/// the printout operator to the stream
inline std::ostream& operator<<( std::ostream& s, const Decays::Decay::Item& item ) { return item.fillStream( s ); }
// ============================================================================
//                                                                      The END
// ============================================================================
#endif // PARTPROP_DECAY_H
