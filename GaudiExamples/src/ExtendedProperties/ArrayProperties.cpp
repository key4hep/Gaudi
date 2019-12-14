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
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#define NO_C_ARRAY_AS_PROPERTY_WARNING
#include "GaudiKernel/CArrayAsProperty.h"
// ============================================================================
/** @file
 *  Simple example/test for "array"-properties
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-15
 */
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace Examples {
    // ========================================================================
    class ArrayProperties : public GaudiAlgorithm {
    public:
      // ======================================================================
      /// execute it!
      StatusCode execute() override;
      // ======================================================================
    public:
      // ======================================================================
      /** Standard constructor
       *
       */
      ArrayProperties( const std::string& name, // algorithm instance name
                       ISvcLocator*       pSvc )      //         service locator
          : GaudiAlgorithm( name, pSvc ) {
        //
        std::fill( std::begin( m_doubles ), std::end( m_doubles ), -10 );
        std::fill( std::begin( m_strings ), std::end( m_strings ), "blah-blah-blah" );
        //
        declareProperty( "Doubles", m_doubles, "C-array of doubles" );
        declareProperty( "Strings", m_strings, "C-array of strings" );
      }
      // ======================================================================
      /// copy constructor is disabled
      ArrayProperties( const ArrayProperties& ) = delete; // no copy constructor
      /// assignment operator is disabled
      ArrayProperties& operator=( const ArrayProperties& ) = delete; // no assignment
      // ======================================================================
    private:
      // ======================================================================
      /// array  of doubles
      double m_doubles[5]; // array  of doubles
      /// array of strings
      std::string m_strings[4]; // array of strings
      // ======================================================================
    };
    // ========================================================================
  } // namespace Examples
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// execute it!
// ============================================================================
StatusCode Gaudi::Examples::ArrayProperties::execute() {
  propsPrint();
  //
  info() << " Doubles : " << Gaudi::Utils::toString( m_doubles ) << endmsg;
  info() << " Strings : " << Gaudi::Utils::toString( m_strings ) << endmsg;
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/// The Factory
// ============================================================================
using Gaudi::Examples::ArrayProperties;
DECLARE_COMPONENT( ArrayProperties )
// ============================================================================
// The END
// ============================================================================
