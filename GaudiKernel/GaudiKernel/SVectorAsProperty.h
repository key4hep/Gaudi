// $Id: $
// ============================================================================
#ifndef GAUDIKERNEL_GENVCTPARSERS_H
#define GAUDIKERNEL_GENVCTPARSERS_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <vector>
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Parsers.h"
// ============================================================================
// ROOT/SVector
// ============================================================================
#include "Math/SVector.h"
// ============================================================================
/** @file
 *  Declaration of parsing functions for generic vectors to allow
 *  their usage as properties for Gaudi components
 *
 *  @attention! this file must be included *BEFORE* *
 *   any direct or indirect inclusion of GaudiKernel/ToStream.h
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-05
 */
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Parsers
  {
    // ========================================================================
    /** parse SVector
     *  @param result (output) the parsed vector
     *  @param input  (input)  the input string
     *  @return status code
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    template <class SCALAR, unsigned int N>
    StatusCode parse
    ( ROOT::Math::SVector<SCALAR,N>& result ,
      const std::string&             input  )
    {
      std::vector<double> tmp ;
      StatusCode sc = parse ( tmp , input ) ;
      if ( sc.isFailure()  ) { return sc                  ; } // RETURN
      if ( N != tmp.size() ) { return StatusCode::FAILURE ; } // RETURN
#ifdef WIN32
// Disable warning
//    C4996: 'std::copy': Function call with parameters that may be unsafe
// The parameters are checked
#pragma warning(push)
#pragma warning(disable:4996)
#endif
      std::copy ( tmp.begin() , tmp.end() , result.begin() ) ;
#ifdef WIN32
#pragma warning(pop)
#endif
      return StatusCode::SUCCESS ;
    }
    // ========================================================================
  } //                                          end of namespace Gaudi::Parsers
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Utils
  {
    // ========================================================================
    template <class SCALAR>
    std::ostream& toStream ( const SCALAR& , std::ostream& ) ;
    // ========================================================================
    /** the general streaming function for SVector using python-tupel format
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-07
     */
    template <class SCALAR, unsigned int N>
    std::ostream& toStream
    ( const ROOT::Math::SVector<SCALAR,N>& obj , std::ostream& s )
    {
      s << "( ";
      for ( typename ROOT::Math::SVector<SCALAR,N>::const_iterator cur = obj.begin() ;
            obj.end() != cur ; ++cur )
      {
        if ( obj.begin() != cur ) { s << " , "; }
        toStream ( *cur , s ) ;
      }
      return s << " )";
    }
    // ========================================================================
  } // end of namespace Gaudi::Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_GENVCTPARSERS_H
// ============================================================================
