#ifndef GAUDIKERNEL_STDARRAYASPROPERTY_H
#define GAUDIKERNEL_STDARRAYASPROPERTY_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include "GaudiKernel/ToStream.h"
#include <algorithm>
#include <array>
// ============================================================================
/** @file
 *  Collection of utilities, which allows to use class std::array
 *  as property for Gaudi-components
 *
 *  @attention this file must be "included" before GaudiKernel/Property.h
 *
 *  @author Alexander Mazurov alexander.mazurov@cern.ch
 *  @date 2015-02-23
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-16
 */
// ============================================================================
// 1) Streamers : value -> string
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Utils
  {
    // ========================================================================
    template <class ITERATOR>
    inline std::ostream& toStream( ITERATOR           first,   // begin of the sequence
                                   ITERATOR           last,    //   end of the sequence
                                   std::ostream&      s,       //            the stream
                                   const std::string& open,    //               opening
                                   const std::string& close,   //               closing
                                   const std::string& delim ); //             delimiter
    // ========================================================================
  } //                                            end of namespace Gaudi::Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// 2) Parsers: string -> value
// ============================================================================
// GaudiKernel
// ============================================================================
#include <Gaudi/Parsers/CommonParsers.h>
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Parsers
  {
    // ========================================================================
    /** parse class std::array from the string
     *  @param result (OUTPUT) the actual result
     *  @param input  (INPUT)  the input string
     *  @return status code
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-16
     */
    template <class TYPE, std::size_t N>
    StatusCode parse( std::array<TYPE, N>& result, const std::string& input )
    {
      typedef std::vector<TYPE> _Vct;
      // create the temporaty vector
      _Vct tmp;
      // parse the string
      StatusCode sc = parse( tmp, input );
      if ( sc.isFailure() ) {
        return sc;
      } // RETURN
      if ( N != tmp.size() ) {
        return StatusCode( StatusCode::FAILURE, true );
      } // RETURN
      // copy vector into array
      std::copy( tmp.begin(), tmp.end(), result.begin() );
      //
      return StatusCode::SUCCESS;
    }
    // ========================================================================
  } //                                          end of namespace Gaudi::Parsers
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_STDARRAYASPROPERTY_H
// ============================================================================
