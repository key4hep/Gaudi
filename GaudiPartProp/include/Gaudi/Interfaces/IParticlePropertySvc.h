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

#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/Kernel.h>
#include <vector>

namespace Gaudi {
  class ParticleID;
  class ParticleProperty;
} // namespace Gaudi

namespace Gaudi::Interfaces {
  /** @class IParticlePropertySvc Kernel/IParticlePropertySvc.h
   *  The abstract interface to Particle Property Service
   *  @author Iain Last
   *  @author G.Corti
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   */
  class GAUDI_API IParticlePropertySvc : public extend_interfaces<IInterface> {
  public:
    DeclareInterfaceID( Gaudi::Interfaces::IParticlePropertySvc, 2, 0 );

    /// the actual type of (ordered) container of particle properties
    typedef std::vector<const Gaudi::ParticleProperty*> ParticleProperties;
    /// the actual type of iterator over the (ordered) container of properties
    typedef ParticleProperties::const_iterator iterator;

    /** get the begin-iterator for the container of particle properties
     *  It is assumed that the container is properly ordered
     *  @return begin-iterator for the container of particle properties
     */
    virtual iterator begin() const = 0;
    /** get the end-iterator for the container of particle properties
     *  It is assumed that the container is properly ordered
     *  @return end-iterator for the container of particle properties
     */
    virtual iterator end() const = 0;
    /// get the container size.
    virtual size_t size() const = 0;
    /** Retrieve an object by name:
     *
     *  @code
     *
     *   Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *   const std::string& name = ... ;
     *
     *   const Gaudi::ParticleProperty*  pp = svc -> find ( name ) ;
     *
     *  @endcode
     *  @param name the particle name
     *  @return pointer to particle property object
     */
    virtual const ParticleProperty* find( const std::string& name ) const = 0; // find by particle name
    /** Retrieve an object by Gaudi::ParticleID:
     *
     *  @code
     *
     *   Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *   const Gaudi::ParticleID& pid = ... ;
     *
     *   const Gaudi::ParticleProperty*  pp = svc -> find ( pid ) ;
     *
     *  @endcode
     *  @param  pid     the particle's Gaudi::ParticleID
     *  @return pointer to particle property object
     */
    virtual const ParticleProperty* find( const Gaudi::ParticleID& pid ) const = 0; // find by Gaudi::ParticleID

    /** get the properties according to some criteria
     *
     *  e.g. get all leptons:
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *  ...
     *  using namespace boost::lambda ;
     *  ...
     *
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  typedef Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties Vector ;
     *
     *  // create the output vector:
     *  Vector leptons ;
     *  // use the service
     *  svc -> get
     *    ( bind ( &Gaudi::ParticleID::isLepton ,
     *             bind ( &Gaudi::ParticleProperty::particleID , _1 ) ) ,
     *      std::back_inserter ( lepton ) ) ; // output
     *
     *  @endcode
     *
     *  Essentially it is just <c>std::copy_if</c> STL-algorithm
     *
     *  @param cut the predicate
     *  @param output the output iterator
     *  @return the updated position of output iterator
     */
    template <class PREDICATE, class OUTPUT>
    OUTPUT get( const PREDICATE& cut, OUTPUT output ) const;

    /** make the charge conjugation for the string/decay descriptor
     *
     *  @code
     *
     *   std::string decay = "B0 -> pi+ pi-" ;
     *
     *   Gaudi::IParticlePropertySvc* svc = ... ;
     *
     *   std::string cc = svc -> cc ( decay ) ;
     *
     *  @endcode
     *
     *  @param decay the decay descriptor
     *  @return the charge conjugation for the decay descriptor
     */
    virtual std::string cc( const std::string& decay ) const = 0;
  };
} // namespace Gaudi::Interfaces

namespace Gaudi {
  namespace ParticleProperties {
    /** helper utility for mapping of Gaudi::ParticleProperty object into
     *  non-negative integral sequential identifier
     *  @code
     *
     *  const Gaudi::ParticlePropertySvc* svc = ... ;
     *
     *  const Gaudi::ParticleProperty* pp = ... ;
     *
     *   size_t index = index ( pp , svc ) ;
     *
     *   if ( !index )
     *   {
     *     // error here: conversion is not possible
     *   }
     *
     *  @endcode
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
    size_t index( const Gaudi::ParticleProperty* property, const Gaudi::Interfaces::IParticlePropertySvc* service );

    /** helper utility for mapping of Gaudi::ParticleID object into
     *  non-negative integral sequential identifier
     *  @code
     *
     *  const Gaudi::ParticlePropertySvc* svc = ... ;
     *
     *  const Gaudi::ParticleID& pid = ... ;
     *
     *  size_t index = index ( pid , svc ) ;
     *
     *  if ( !index )
     *   {
     *     // error here: conversion is not possible
     *   }
     *
     *  @endcode
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
    size_t index( const Gaudi::ParticleID& pid, const Gaudi::Interfaces::IParticlePropertySvc* service );

    /** the inverse mapping of the integer sequential number onto
     *  Gaudi::ParticleID object
     *  @code
     *
     *  const Gaudi::ParticlePropertySvc* svc = ... ;
     *
     *  const size_t index = ...
     *
     *  const Gaudi::ParticleProperty* pp = particle ( index , svc ) ;
     *
     *  if ( !pp )
     *   {
     *     // error here: conversion is not possible
     *   }
     *
     *  @endcode
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
    const Gaudi::ParticleProperty* particle( const size_t                                   index,
                                             const Gaudi::Interfaces::IParticlePropertySvc* service );

    /** the inverse mapping of the integer sequential number onto
     *  Gaudi::ParticleID object
     *  @code
     *
     *  const Gaudi::ParticlePropertySvc* svc = ... ;
     *
     *  const size_t index = ...
     *
     *  const Gaudi::ParticleID pid = particleID ( index , svc ) ;
     *
     *  if ( Gaudi::ParticleID() == pid )
     *   {
     *     // error here: conversion is not possible
     *   }
     *
     *  @endcode
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
    const Gaudi::ParticleID particleID( const size_t index, const Gaudi::Interfaces::IParticlePropertySvc* service );

    /** mapping by pythiaID
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
    const Gaudi::ParticleProperty* byPythiaID( const int pythia, const Gaudi::Interfaces::IParticlePropertySvc* svc );

    /** mapping by EvtGen-name
     *
     *  @code
     *
     *   const std::string& evtGen
     *
     *   const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *   const Gaudi::ParticleProperty* pp = byEvtGenName ( pythiaID , svc ) ;
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
    const Gaudi::ParticleProperty* byEvtGenName( const std::string&                             evtGen,
                                                 const Gaudi::Interfaces::IParticlePropertySvc* svc );

    /** get all particle properties which satisfy the certain criteria
     *
     *  e.g. select all particles with 'invalid' Pythia ID = 0 :
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *
     *  typedef Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties Vector ;
     *  using namespace boost::lambda ;
     *
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  // create the output vector:
     *  Vector output ;
     *  // use the function
     *  Gaudi::ParticleProperties::get
     *         ( svc->begin() ,  // begin-sequence
     *           svc->end  () ,  // end-sequence
     *           bind ( &Gaudi::ParticleProperty::pythiaID , _1 ) == 0 , // predicate
     *           std::back_inserter( output ) ) ; // output
     *
     *  @endcode
     *
     *  or select all particles with 'invalid' EvtGen  = "unknown" :
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *
     *  typedef Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties Vector ;
     *  using namespace boost::lambda ;
     *
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  // create the output vector:
     *  Vector output ;
     *  // use the function
     *  Gaudi::ParticleProperties::get
     *         ( svc->begin() ,  // begin-sequence
     *           svc->end  () ,  // end-sequence
     *           bind ( &Gaudi::ParticleProperty::evtGen , _1 ) == "unknown" , // predicate
     *           std::back_inserter( output ) ) ; // output
     *
     *  @endcode
     *
     *  select leptons:
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *  ...
     *  ...
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  typedef Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties Vector ;
     *  using namespace boost::lambda ;
     *
     *  // create the output vector:
     *  Vector leptons ;
     *  // use the function
     *  Gaudi::ParticleProperties::get
     *         ( svc->begin() ,  // begin-sequence
     *           svc->end  () ,  // end-sequence
     *           bind ( &Gaudi::ParticleID::isLepton , bind ( &Gaudi::ParticleProperty::particleID ,_1 ) ) ,
     *           std::back_inserter ( lepton ) ) ; // output
     *
     *  @endcode
     *
     *  Essentially it is just a missing <c>std::copy_if</c> STL-algorithm
     *
     *  @param first begin-iterator of input sequence of particle properties
     *  @param last  end-iterator of input sequence of particle properties
     *  @param cut the predicate
     *  @param output the output iterator
     *  @return the updated position of output iterator
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-08-03
     */
    template <class INPUT, class PREDICATE, class OUTPUT>
    OUTPUT get( INPUT first, INPUT last, const PREDICATE& cut, OUTPUT output ) {
      for ( ; first != last; ++first ) {
        if ( cut( *first ) ) {
          *output = *first;
          ++output;
        }
      }
      return output;
    }

    /** get all particle properties which satisfy the certain criteria
     *
     *  e.g. select all particles with 'invalid' Pythia ID = 0 :
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *
     *  typedef Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties Vector ;
     *  using namespace boost::lambda ;
     *
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  // create the output vector:
     *  Vector output ;
     *  // use the function
     *  Gaudi::ParticleProperties::get
     *    ( svc ,
     *      bind ( &Gaudi::ParticleProperty::pythiaID , _1 ) == 0 , // predicate
     *      std::back_inserter( output ) ) ; // output
     *
     *  @endcode
     *
     *  or select all particles with 'invalid' EvtGen  = "unknown" :
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *
     *  typedef Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties Vector ;
     *  using namespace boost::lambda ;
     *
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  // create the output vector:
     *  Vector output ;
     *  // use the function
     *  Gaudi::ParticleProperties::get
     *    ( svc ,
     *      bind ( &Gaudi::ParticleProperty::evtGen , _1 ) == "unknown" , // predicate
     *      std::back_inserter( output ) ) ; // output
     *
     *  @endcode
     *
     *  Select all leptons:
     *
     *  @code
     *
     *  #include "boost/lambda/lambda.hpp"
     *  #include "boost/lambda/bind.hpp"
     *
     *  typedef Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties Vector ;
     *  using namespace boost::lambda ;
     *
     *  const Gaudi::Interfaces::IParticlePropertySvc* svc = ... ;
     *
     *  // create the output vector:
     *  Vector leptons ;
     *  // use the function
     *  Gaudi::ParticleProperties::get
     *    ( svc ,
     *      bind ( &Gaudi::ParticleID::isLepton ,
     *      bind ( &Gaudi::ParticleProperty::particleID , _1 ) ) ,
     *      std::back_inserter ( leptons ) ) ; // output
     *
     *  @endcode
     *
     *  @param first begin-iterator of input sequence of particle properties
     *  @param end   end-iterator of input sequence of particle properties
     *  @param cut the predicate
     *  @param output the output iterator
     *  @return the updated position of output iterator
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-08-03
     */
    template <class PREDICATE, class OUTPUT>
    OUTPUT get( const Gaudi::Interfaces::IParticlePropertySvc* service, const PREDICATE& cut, OUTPUT output ) {
      if ( 0 == service ) { return output; }
      return service->get( cut, output );
    }

    /** get all the properties at once
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-08-03
     */
    Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties
    allProperties( const Gaudi::Interfaces::IParticlePropertySvc* service );
  } // namespace ParticleProperties

  namespace Interfaces {
    // get the properties according to some criteria
    template <class PREDICATE, class OUTPUT>
    OUTPUT IParticlePropertySvc::get( const PREDICATE& cut, OUTPUT output ) const {
      iterator first = this->begin();
      iterator last  = this->end();
      return Gaudi::ParticleProperties::get( first, last, cut, output );
    }
  } // namespace Interfaces
} // namespace Gaudi
