// $Id: 
// ============================================================================
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
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Hash.h"
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
namespace GaudiAlg
{
  // ==========================================================================
  /** @class ID GaudiHistoID.h GaudiAlg/GaudiHistoID.h
   *
   *  ID class for Histogram and Ntuples. Internally handles both
   *  numeric and string like IDs
   *
   *  @author Chris Jones  Christopher.Rob.Jones@cern.ch
   *  @date   2005-08-12
   */
  class GAUDI_API ID
  {
  public:
    // ========================================================================
    /// type for internal numeric ID
    typedef int         NumericID;
    /// type for internal literal ID
    typedef std::string LiteralID;
    // ========================================================================
  public:
    // ========================================================================
    /// Implicit constructor from a numeric ID
    ID  ( const NumericID    id = -1 ) ; 
    /// Implicit constructor from a literal ID
    ID  ( const LiteralID&   id      ) ; 
    /// Implicit constructor from a literal ID
    ID  ( const char*        id      ) ; 
    /// Destructor
    ~ID ( ) {} ;
    /// Is this ID numeric
    inline bool numeric   () const { return -1 != m_nID ;    }
    /// Is this ID numeric
    inline bool literal   () const { return !m_aID.empty() ; }
    /// Is this ID undefined
    inline bool undefined () const { return !numeric() && !literal(); }
    /// Returns the ID as a LiteralID
    inline const LiteralID& literalID() const { return m_aID; }
    /// Returns the numerical ID
    inline NumericID        numericID() const { return m_nID; }
    /// Return ID as string, for both numeric and literal IDs
    GAUDI_API LiteralID idAsString() const ;
    /// cast operator to std::string
    operator std::string () const { return idAsString () ; }
    /** @brief Implement the operator ==
     *  Implementation depends on type of ID
     *  @return boolean indicating if the IDs are equal
     */
    inline bool operator==( const ID& id ) const
    {
      return 
        hash    () != id.hash    () ? false                           :  
        numeric () && id.numeric () ? id.numericID () == numericID () :
        literal () && id.literal () ? id.literalID () == literalID () :
        idAsString () == id.idAsString() ; 
    }
    /// Implement the != operator, using the == operator
    inline bool operator!=( const ID& id ) const { return ! ( *this == id ) ; }
    /** @brief Implement the operator <
     *  Implementation depends on type of ID
     *  @return boolean indicating the order of the IDs
     */
    inline bool operator<( const ID& id ) const
    {
      return
        //hash () < id.hash () ? true  :
        //hash () > id.hash () ? false :      
        numeric    () && id.numeric() ? numericID() < id.numericID() :
        literal    () && id.literal() ? literalID() < id.literalID() :
        idAsString () < id.idAsString() ;
    }
    // ========================================================================
    GAUDI_API std::ostream& fillStream ( std::ostream& s ) const ;
    // ========================================================================
  public:
    // ========================================================================
    /// good ID?
    bool operator!() const { return undefined() ; }
    // ========================================================================
  public:
    // ========================================================================
    /// return hash value (for python)
    inline size_t   hash   () const { return m_hash    ; }
    /// return hash value (for python)
    inline size_t __hash__ () const { return   hash () ; }
    // ========================================================================
  private:
    // ========================================================================
    /// Internal numeric ID
    NumericID   m_nID ;                            //       Internal numeric ID
    /// Internal alpha-numeric ID
    LiteralID   m_aID ;                            // Internal alpha-numeric ID
    // ========================================================================
  private:
    // ========================================================================
    /// the hash value of ID
    size_t m_hash  ;                                    // the hash value of ID
    // ========================================================================
  };
  // ==========================================================================
  /// Operator overloading for ostream
  inline std::ostream& operator << ( std::ostream& str , const GaudiAlg::ID& id )
  { return id.fillStream ( str ) ; }
  // ==========================================================================
} //                                                  end of namespace GaudiAlg 
// ============================================================================
namespace GaudiUtils
{
  // ==========================================================================
  /// Hash-function for class GaudiAlg::ID
  template <>
  inline size_t Hash<GaudiAlg::ID>::operator() 
    ( const GaudiAlg::ID& key ) const { return key.hash () ; }   
  // ==========================================================================
} //                                                end of namespace GaudiUtils 
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIALG_GAUDIHISTOID_H
// ============================================================================
