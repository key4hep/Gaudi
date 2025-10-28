/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
//====================================================================
//	CLHEP Basic Random Engine definition file
//--------------------------------------------------------------------
//
//	Package    : HepRndm ( The LHCb Offline System)
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/99| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#pragma once

// Framework include files
#include "RndmEngine.h"
#include <CLHEP/Random/Random.h>
#include <memory>
#include <mutex>
#include <utility>

// Forward declarations
namespace CLHEP {
  class HepRandomEngine;
}

namespace HepRndm {

  template <typename Engine>
  class SynchronizedEngine : public Engine {

  private:
    mutable std::mutex m_mutex;

  public:
    using Engine::Engine;

  public:
    // reimplement anything that should be locked for thread safety

    double flat() override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::flat();
    }
    void flatArray( const int size, double* vect ) override {
      std::lock_guard<std::mutex> lock( m_mutex );
      Engine::flatArray( size, vect );
    }
    void setSeed( long seed, int n ) override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::setSeed( seed, n );
    }
    void setSeeds( const long* seeds, int n ) override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::setSeeds( seeds, n );
    }
    void saveStatus( const char filename[] = "Config.conf" ) const override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::saveStatus( filename );
    }
    void restoreStatus( const char filename[] = "Config.conf" ) override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::restoreStatus( filename );
    }
    void showStatus() const override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::showStatus();
    }
    std::ostream& put( std::ostream& os ) const override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::put( os );
    }
    std::istream& get( std::istream& is ) override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::get( is );
    }
    std::istream& getState( std::istream& is ) override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::getState( is );
    }
    std::vector<unsigned long> put() const override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::put();
    }
    bool get( const std::vector<unsigned long>& v ) override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::get( v );
    }
    bool getState( const std::vector<unsigned long>& v ) override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::getState( v );
    }
    operator double() override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::operator double();
    }
    operator float() override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::operator float();
    }
    operator unsigned int() override {
      std::lock_guard<std::mutex> lock( m_mutex );
      return Engine::operator unsigned int();
    }
  };

  class BaseEngine : public RndmEngine {
  protected:
    std::unique_ptr<CLHEP::HepRandomEngine> m_hepEngine;
    Gaudi::Property<bool>                   m_threadSafe{ this, "ThreadSafe", true };

  public:
    BaseEngine( const std::string& name, ISvcLocator* loc ) : RndmEngine( name, loc ) {}
    CLHEP::HepRandomEngine*       hepEngine() { return m_hepEngine.get(); }
    const CLHEP::HepRandomEngine* hepEngine() const { return m_hepEngine.get(); }
    // Retrieve single random number
    double rndm() const override { return m_hepEngine->flat(); }

    StatusCode finalize() override {
      if ( m_hepEngine ) { CLHEP::HepRandom::setTheEngine( nullptr ); }
      m_hepEngine.reset();
      return RndmEngine::finalize();
    }

  protected:
    void                                            initEngine() { m_hepEngine = createEngine(); }
    virtual std::unique_ptr<CLHEP::HepRandomEngine> createEngine() = 0;
    template <typename Engine, typename... Args>
    auto create_engine( Args&&... args ) {
      return ( m_threadSafe.value() ? std::make_unique<SynchronizedEngine<Engine>>( std::forward<Args>( args )... )
                                    : std::make_unique<Engine>( std::forward<Args>( args )... ) );
    }
  };
} // namespace HepRndm
