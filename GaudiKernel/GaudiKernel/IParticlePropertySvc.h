// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IParticlePropertySvc.h,v 1.4 2004/03/04 13:30:35 mato Exp $
#ifndef GAUDIKERNEL_IPARTICLEPROPERTYSVC_H
#define GAUDIKERNEL_IPARTICLEPROPERTYSVC_H

// Include Files
#include "GaudiKernel/IService.h"
#include <vector>

// Forward declarations.
class ParticleProperty;

/** @class IParticlePropertySvc IParticlePropertySvc.h GaudiKernel/IParticlePropertySvc.h

    This class is an interface to the ParticlePropertySvc.

    @author Iain Last
    @author G.Corti
*/
class GAUDI_API IParticlePropertySvc: virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID(IParticlePropertySvc,3,0);

  // Typedefs for container type
  typedef std::vector<ParticleProperty*> VectPP;
  typedef VectPP::const_iterator         const_iterator;
  typedef VectPP::iterator               iterator;

  /** Create a new particle property
      @param  particle  String name of the particle
      @param  geantId  Geant3 ID of the particle
      @param  jetsetId Jetset ID of the particle
      @param  charge  Particle charge (/e)
      @param  mass  Particle mass (MeV)
      @param  tlife Particle lifetime (s)
      @param  evtName  String name of the particle in EvtGen
      @param  pythiaId  ID of particle in Pythia
      @return StatusCode - SUCCESS if the particle property was added
  */
  virtual StatusCode push_back( const std::string& particle,
				int geantId, int jetsetId,
				double charge, double mass, double tlife ,
				const std::string& evtName , int pythiaId ,
				double maxWidth ) = 0;

  /** Create a new particle property.
      @param  pp a particle property class
      @return StatusCode - SUCCESS if the particle property was added
  */
  virtual StatusCode push_back( ParticleProperty* pp ) = 0;

  /// Get a const reference to the begining of the container.
  virtual const_iterator begin() const = 0;

  /// Get a const reference to the end of the container.
  virtual const_iterator end() const = 0;

  /// Get the container size.
  virtual int size() const = 0;

  /// Retrieve an object by geant3 id.
  virtual ParticleProperty* find( int geantId ) = 0;

  /// Retrieve an object by StdHep id
  virtual ParticleProperty* findByStdHepID( int stdHepID ) = 0;

  /// Retrieve an object by name.
  virtual ParticleProperty* find( const std::string& name ) = 0;

  /// Retrieve an object by PythiaID
  virtual ParticleProperty* findByPythiaID( int pythiaID ) =0;

  /// Erase a property by geant3 id.
  virtual StatusCode erase( int geantId ) = 0;

  /// Erase a property by particle name.
  virtual StatusCode erase( const std::string& name ) = 0;

  /// Erase a property by StdHep id
  virtual StatusCode eraseByStdHepID( int stdHepID ) = 0;

  /// virtual destructor
  virtual ~IParticlePropertySvc() {}
};

#endif


