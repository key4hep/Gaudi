// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/FindByMassRange.h,v 1.2 2000/12/13 12:57:13 mato Exp $
#ifndef GAUDIKERNEL_FINDBYMASSRANGE_H
#define GAUDIKERNEL_FINDBYMASSRANGE_H


// Include files
#include "GaudiKernel/IParticlePropertySvc.h"
#include "GaudiKernel/ParticleProperty.h"


/** @class FindByMassRange FindByMassRange.h GaudiKernel/FindByMassRange.h
    The function object must be derived from
    std::unary_function< IParticlePropertySvc::value_type ,bool >
    IParticlePropertySvc::value_type is a typedef that defines the
    internal service storage type that is returned when an iterator
    is dereferenced. In this case it corresponds to
    pair< std::string, ParticleProperty* > as the ParticlePropertySvc
    stores all its data in a map. The string will be the map's key
    (usually the particle name) and the pointer is a pointer to the
    ParticleProperty object.
    The bool template argument states that operator() returns
    true or false.

    The data is accessed in the following manner:
    const IParticlePropertySvc::value_type& pp_ref;
    std::string key = pp_ref.first;
    ParticleProperty* pp = pp_ref.second;

    @author Ian Last
*/
class GAUDI_API FindByMassRange: public std::unary_function< ParticleProperty* ,bool > {

public:

  /// Constructor - sets the mass range in Gev.
  FindByMassRange( double low, double high ) : m_low( low ), m_high( high ) { }

  /** This routine check to see if the mass of a particle lies within the
      given mass range and returns true if it does.
  */
  bool operator() ( const ParticleProperty* pp ) const                        {
    bool in_range = false;
    if ( pp ) {
      if ( pp->mass() >= m_low && pp->mass() <= m_high ) {
	      in_range = true;
      }
    }
    return in_range;
  }

private:

  /// Low and high mass range (GeV)
  double m_low;
  double m_high;
};

#endif  // GAUDIKERNEL_FINDBYMASSRANGE_H
