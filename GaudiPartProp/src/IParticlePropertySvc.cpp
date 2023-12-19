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
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
// ============================================================================
// GaudiPartProp
// ============================================================================
#include "Gaudi/ParticleID.h"
#include "Gaudi/ParticleProperty.h"
#include <Gaudi/Interfaces/IParticlePropertySvc.h>
// ============================================================================
/** @file
 *  Implementation file for class Gaudi::Interfaces::IParticlePropertySvc
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-08-03
 */
// ============================================================================
/* helper utility for mapping of Gaudi::ParticleProperty object into
 *  non-negative integral sequential identifier
 *
 *  This appears to be useful operation, but since it is
 *  "pure technical" it does not appear as interface method.
 *
 *  For invalid/missing property and/or  service
 *  <c>0</c> is returned. The valid result is always
 *  satisfy the condition: <c> index <= service->size() </c>
 *
 *  @param property the property to be mapped
 *  @param service the service
 *  @return the sequential non-negative index
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2008-08-03
 */
// ============================================================================
size_t Gaudi::ParticleProperties::index( const Gaudi::ParticleProperty*                 property,
                                         const Gaudi::Interfaces::IParticlePropertySvc* service ) {
  if ( !property || !service ) { return 0; } // RETURN
  // ==========================================================================
  auto first = service->begin();
  auto last  = service->end();
  // start the binary_search
  static constexpr auto cmp   = Gaudi::ParticleProperty::Compare();
  auto                  ifind = std::lower_bound( first, last, property, cmp );
  return last != ifind && !cmp( *ifind, property ) ? ( ifind - first + 1 ) : 0;
}
// ============================================================================
/* helper utility for mapping of Gaudi::ParticleID object into
 *  non-negative integral sequential identifier
 *
 *  This appears to be useful operation, but since it is
 *  "pure technical" it does not appear as interface method.
 *
 *  For invalid/missing PID and/or  service
 *  <c>0</c> is returned. The valid result is always
 *  satisfy the condition: <c> index <= service->size() </c>
 *
 *  @param pid the object to be mapped
 *  @param service the service
 *  @return the sequential non-negative index
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2008-08-03
 */
// ============================================================================
size_t Gaudi::ParticleProperties::index( const Gaudi::ParticleID&                       pid,
                                         const Gaudi::Interfaces::IParticlePropertySvc* service ) {
  if ( !service ) { return 0; } // RETURN
  // ==========================================================================
  const Gaudi::ParticleProperty* pp = service->find( pid );
  return pp ? Gaudi::ParticleProperties::index( pp, service ) : 0;
}
// ============================================================================
/* the inverse mapping of the integer sequential number onto
 *  Gaudi::ParticleID object
 *
 *  This appears to be useful operation, but since it is
 *  "pure technical" it does not appear as interface method.
 *
 *  For invalid/missing PID and/or  service
 *  <c>NULL</c> is returned.
 *
 *  @param pid the object to be mapped
 *  @param service the service
 *  @return the sequential non-negative index
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2008-08-03
 */
// ============================================================================
const Gaudi::ParticleProperty*
Gaudi::ParticleProperties::particle( const size_t index, const Gaudi::Interfaces::IParticlePropertySvc* service ) {
  if ( 0 == index || !service ) { return nullptr; } // RETURN
  // get the iterators from the service
  auto first = service->begin();
  auto last  = service->end();
  if ( index > (size_t)std::distance( first, last ) ) { return 0; } // RETURN
  std::advance( first, index - 1 );
  return *first; // RETURN
}
// ============================================================================
/*  the inverse mapping of the integer sequential number onto
 *  Gaudi::ParticleID object
 *
 *  This appears to be useful operation, but since it is
 *  "pure technical" it does not appear as interface method.
 *
 *  For invalid/missing index and/or  service
 *  <c>Gaudi::ParticleID()</c> is returned.
 *
 *  @param pid the object to be mapped
 *  @param service the service
 *  @return the sequential non-negative index
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2008-08-03
 */
// ============================================================================
const Gaudi::ParticleID
Gaudi::ParticleProperties::particleID( const size_t index, const Gaudi::Interfaces::IParticlePropertySvc* service ) {
  if ( 0 == index || !service ) { return Gaudi::ParticleID(); } // RETURN
  const Gaudi::ParticleProperty* pp = Gaudi::ParticleProperties::particle( index, service );
  return pp ? pp->particleID() : Gaudi::ParticleID();
}
// ============================================================================
/* mapping by pythiaID
 *
 *  @code
 *
 *   const int pythiaID = ... ;
 *
 *   const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
 *
 *   const Gaudi::ParticleProeprty* pp = byPythiaID( pythiaID , svc ) ;
 *
 *  @endcode
 *
 *  @attention the method is not very efficient and should not be abused
 *  @see Gaudi::ParticleProperties::particle
 *  @param pythia pythia identifier
 *  @param svc    pointer to particle property service
 *  @return the particle property for the given pythia ID
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2008-08-03
 */
// ============================================================================
const Gaudi::ParticleProperty*
Gaudi::ParticleProperties::byPythiaID( const int pythia, const Gaudi::Interfaces::IParticlePropertySvc* svc ) {
  if ( !svc ) { return nullptr; }
  // to be efficient
  // 1) try to use PDG-ID (fast, logarithmic search)
  const Gaudi::ParticleProperty* pp = svc->find( Gaudi::ParticleID( pythia ) );
  // 2) check the proper pythia ID
  if ( pp && pythia == pp->pythiaID() ) { return pp; } // RETURN
  // 3) use the resular (linear search)
  auto begin = svc->begin();
  auto end   = svc->end();
  auto found =
      std::find_if( begin, end, [&]( const Gaudi::ParticleProperty* pp ) { return pp->pythiaID() == pythia; } );
  //
  return found != end ? *found : nullptr;
}
// ============================================================================
/*  mapping by EvtGen-name
 *
 *  @code
 *
 *   const std::string& evtGen = ...
 *
 *   const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
 *
 *   const Gaudi::ParticleProperty* pp = byEvtGenName ( evtGen , svc ) ;
 *
 *  @endcode
 *
 *  @attention the method is not very efficient and should not be abused
 *  @see Gaudi::ParticleProperties::particle
 *  @param evtGen the particle naem in EvtGen-generator
 *  @param svc    pointer to particle property service
 *  @return the particle property for the given EvtGen-name
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2008-08-03
 */
// ============================================================================
const Gaudi::ParticleProperty*
Gaudi::ParticleProperties::byEvtGenName( const std::string&                             evtGen,
                                         const Gaudi::Interfaces::IParticlePropertySvc* svc ) {
  if ( !svc ) { return nullptr; }
  // to be more efficient:
  // 1) try to use the regualr name (fast, logarithmic search)
  const Gaudi::ParticleProperty* pp = svc->find( evtGen );
  // 2) check the proper evtgen name
  if ( pp && evtGen == pp->evtGen() ) { return pp; } // RETURN
  // 3) use the resular (linear search)
  auto begin = svc->begin();
  auto end   = svc->end();
  auto found = std::find_if( begin, end, [&]( const Gaudi::ParticleProperty* pp ) { return pp->evtGen() == evtGen; } );
  //
  return found != end ? *found : nullptr;
}
// ============================================================================
/*  get all the properties at once
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2008-08-03
 */
// ============================================================================
Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties
Gaudi::ParticleProperties::allProperties( const Gaudi::Interfaces::IParticlePropertySvc* service ) {
  return service ? Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties( service->begin(), service->end() )
                 : Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties();
}
// ============================================================================
// The END
// ============================================================================
