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
#ifndef GAUDI_NAMEDRANGE_H
#define GAUDI_NAMEDRANGE_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
// ============================================================================
// GaudiUtils
// ============================================================================
#include "GaudiKernel/Range.h"
// ============================================================================
/** @file
 *
 *  This file has been imported from
 *  <a href="http://savannah.cern.ch/projects/loki">LoKi project</a>
 *  <a href="http://cern.ch/lhcb-comp/Analysis/Loki">
 *  "C++ ToolKit  for Smart and Friendly Physics Analysis"</a>
 *
 *  The package has been designed with the kind help from
 *  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas,
 *  contributions and advices from G.Raven, J.van Tilburg,
 *  A.Golutvin, P.Koppenburg have been used in the design.
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2001-01-23
 */
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  /** @class NamedRange NamedRange.h GaudiUtils/NamedRange.h
   *
   *  New concept of "named" range : range with name
   *
   *  @see Gaudi::Range_
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date   2004-11-19
   */
  template <class CONTAINER, class ITERATOR = typename Gaudi::details::container<CONTAINER>::Iterator>
  class NamedRange_ : public Gaudi::Range_<CONTAINER, ITERATOR> {
  protected:
    // ========================================================================
    /// the base class
    typedef Gaudi::Range_<CONTAINER, ITERATOR> Base;
    /// "self"-type
    typedef NamedRange_<CONTAINER, ITERATOR> Self;
    // ========================================================================
  public:
    // ========================================================================
    /// default constructor
    NamedRange_() = default;
    /** Constructor
     *  @param ibegin  iterator to begin of the sequence
     *  @param iend    iterator to end   of the sequence
     *  @param name    name of the range
     */
    NamedRange_( typename Base::iterator ibegin, typename Base::iterator iend, std::string name = "" )
        : Base( ibegin, iend ), m_name( std::move( name ) ){};
    /** constructor from the base class
     *  @param base base objects
     *  @param name name of the range
     */
    NamedRange_( const Base& base, std::string name = "" ) : Base( base ), m_name( std::move( name ) ){};
    /** constructor from the base class
     *  @param base base objects
     *  @param name name of the range
     */
    NamedRange_( const typename Base::Base& base, std::string name = "" ) : Base( base ), m_name( std::move( name ) ){};
    /** constructor from the base class
     *  @param base base objects
     *  @param name name of the range
     */
    NamedRange_( const typename Base::Container& base, std::string name = "" )
        : Base( base ), m_name( std::move( name ) ){};
    /* constructor of empty range/sequence
     * @param ibegin  iterator to begin of empty sequence
     *  @param name name of the range
     */
    NamedRange_( typename Base::iterator ibegin, std::string name = "" )
        : Base( ibegin, ibegin ), m_name( std::move( name ) ){};
    /// destructor
    ~NamedRange_() = default;
    // ========================================================================
  public:
    // ========================================================================
    /// get a "slice" of a range, in Python style
    NamedRange_ slice( long index1, long index2 ) const { return NamedRange_( Base::slice( index1, index2 ), m_name ); }
    // ========================================================================
  public:
    // ========================================================================
    /// get the name of the range
    const std::string& name() const { return m_name; }
    /// set the name of the range
    void setName( std::string value ) { m_name = std::move( value ); }
    // ========================================================================
  private:
    // ========================================================================
    /// the name, associated to the range
    std::string m_name; // the name associated to the range
    // ========================================================================
  };
  // ==========================================================================
  /** simple function to create the named range from arbitrary container
   *  @code
   *
   *    const CONTAINER& cnt = ... ;
   *    auto r = range ( cnt , "some name") ;
   *
   *  @endcode
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2007-11-29
   */
  template <class CONTAINER>
  NamedRange_<CONTAINER> range( const CONTAINER& cnt, std::string name ) {
    return NamedRange_<CONTAINER>( cnt.begin(), cnt.end(), std::move( name ) );
  }
  // ==========================================================================
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDI_NAMEDRANGE_H
