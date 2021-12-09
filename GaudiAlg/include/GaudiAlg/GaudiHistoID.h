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
#ifndef GAUDIALG_GAUDIHISTOID_H
#define GAUDIALG_GAUDIHISTOID_H 1
// ============================================================================
// Include files
// ============================================================================
// STD&STL
// ============================================================================
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Hash.h"
#include "GaudiKernel/Kernel.h"
// ============================================================================
/*  @file
 *
 *  Header file for class GaudiAlg::ID
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-01-23
 */
// ============================================================================
/** @namespace GaudiAlg
 *
 *  Definitions of few useful hash-maps, classes and typedefs
 *  used for classes GaudiHistos and GaudiTuples.
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-01-23
 */
// ============================================================================
namespace GaudiAlg {
  // ==========================================================================
  /** @class ID GaudiHistoID.h GaudiAlg/GaudiHistoID.h
   *
   *  ID class for Histogram and Ntuples. Internally handles both
   *  numeric and string like IDs
   *
   *  @author Chris Jones  Christopher.Rob.Jones@cern.ch
   *  @date   2005-08-12
   */
  class GAUDI_API ID {
  public:
    // ========================================================================
    /// type for internal numeric ID
    typedef int NumericID;
    /// type for internal literal ID
    typedef std::string LiteralID;
    // ========================================================================
  public:
    // ========================================================================
    /// Implicit constructor from a numeric ID
    ID( const NumericID id = -1 ) : m_nID( id ), m_hash( boost::hash_value( id ) ) {}
    /// Implicit 'move' constructor from a literal ID
    ID( LiteralID&& id ) : m_aID( std::move( id ) ), m_hash( boost::hash_value( m_aID ) ) {}
    /// Implicit 'copy' constructor from a literal ID
    ID( const LiteralID& id ) : m_aID( id ), m_hash( boost::hash_value( m_aID ) ) {}
    /// Implicit constructor from a literal ID
    ID( const char* id ) : m_aID( id ), m_hash( boost::hash_value( m_aID ) ) {}
    /// Destructor
    ~ID() = default;
    /// Is this ID numeric
    inline bool numeric() const noexcept { return -1 != m_nID; }
    /// Is this ID numeric
    inline bool literal() const noexcept { return !m_aID.empty(); }
    /// Is this ID undefined
    inline bool undefined() const noexcept { return !numeric() && !literal(); }
    /// Returns the ID as a LiteralID
    inline const LiteralID& literalID() const noexcept { return m_aID; }
    /// Returns the numerical ID
    inline NumericID numericID() const noexcept { return m_nID; }
    /// Return ID as string, for both numeric and literal IDs
    GAUDI_API LiteralID idAsString() const;
    /// cast operator to std::string
    operator std::string() const { return idAsString(); }
    /** @brief Implement the operator ==
     *  Implementation depends on type of ID
     *  @return boolean indicating if the IDs are equal
     */
    inline bool operator==( const ID& id ) const noexcept {
      return hash() != id.hash()         ? false
             : numeric() && id.numeric() ? id.numericID() == numericID()
             : literal() && id.literal() ? id.literalID() == literalID()
                                         : idAsString() == id.idAsString();
    }
    /// Implement the != operator, using the == operator
    inline bool operator!=( const ID& id ) const { return !( *this == id ); }
    /** @brief Implement the operator <
     *  Implementation depends on type of ID
     *  @return boolean indicating the order of the IDs
     */
    inline bool operator<( const ID& id ) const noexcept {
      return
          // hash () < id.hash () ? true  :
          // hash () > id.hash () ? false :
          numeric() && id.numeric()   ? numericID() < id.numericID()
          : literal() && id.literal() ? literalID() < id.literalID()
                                      : idAsString() < id.idAsString();
    }
    // ========================================================================
    GAUDI_API std::ostream& fillStream( std::ostream& s ) const;
    // ========================================================================
  public:
    // ========================================================================
    /// good ID?
    bool operator!() const noexcept { return undefined(); }
    // ========================================================================
  public:
    // ========================================================================
    /// return hash value (for python)
    inline size_t hash() const noexcept { return m_hash; }
    /// return hash value (for python)
    inline size_t __hash__() const noexcept { return hash(); }
    // ========================================================================
  private:
    // ========================================================================
    /// Internal numeric ID
    NumericID m_nID{ -1 }; //       Internal numeric ID
    /// Internal alpha-numeric ID
    LiteralID m_aID; // Internal alpha-numeric ID
    // ========================================================================
  private:
    // ========================================================================
    /// the hash value of ID
    size_t m_hash{ 0 }; // the hash value of ID
    // ========================================================================
  };
  // ==========================================================================
  /// Operator overloading for ostream
  inline std::ostream& operator<<( std::ostream& str, const GaudiAlg::ID& id ) { return id.fillStream( str ); }
  // ==========================================================================
} //                                                  end of namespace GaudiAlg
// ============================================================================
namespace GaudiUtils {
  // ==========================================================================
  /// Hash-function for class GaudiAlg::ID
  template <>
  inline size_t Hash<GaudiAlg::ID>::operator()( const GaudiAlg::ID& key ) const {
    return key.hash();
  }
  // ==========================================================================
} //                                                end of namespace GaudiUtils
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDIHISTOID_H
