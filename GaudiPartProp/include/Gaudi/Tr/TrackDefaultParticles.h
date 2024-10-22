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

#include <GaudiKernel/StatusCode.h>
#include <array>
#include <iomanip>
#include <stdexcept>
#include <stdint.h>

namespace Gaudi {
  namespace Tr {
    class PID {
      enum class validated_pid_t { Electron = 0, Muon, Pion, Kaon, Proton };
      static constexpr std::array<double, 5> s_mass = { 0.51099891, 105.65837, 139.57018, 493.677, 938.27203 };

      validated_pid_t m_value;

      static constexpr validated_pid_t validate( int id ) {
        switch ( id ) {
        case 11:
          return validated_pid_t::Electron;
        case 13:
          return validated_pid_t::Muon;
        case 211:
          return validated_pid_t::Pion;
        case 321:
          return validated_pid_t::Kaon;
        case 2212:
          return validated_pid_t::Proton;
        default:
          throw std::runtime_error( "invalid PID" );
        }
      }

    public:
      constexpr explicit PID( const int id ) : m_value( validate( id ) ) {}

      constexpr explicit PID( validated_pid_t pid ) : m_value( pid ) {}

      PID() = default;
      // Make the creation of thes PID objects expressive in user code
      // by calling Gaudi::Tr::PID::Electron()
      static constexpr PID Electron() { return PID{ validated_pid_t::Electron }; }

      static constexpr PID Muon() { return PID{ validated_pid_t::Muon }; }

      static constexpr PID Pion() { return PID{ validated_pid_t::Pion }; }

      static constexpr PID Kaon() { return PID{ validated_pid_t::Kaon }; }

      static constexpr PID Proton() { return PID{ validated_pid_t::Proton }; }

      // Framwork functions allowing the use of PID inside a property
      friend const char* toString( PID pid ) {
        switch ( pid.m_value ) {
        case validated_pid_t::Electron:
          return "Electron";
        case validated_pid_t::Muon:
          return "Muon";
        case validated_pid_t::Pion:
          return "Pion";
        case validated_pid_t::Kaon:
          return "Kaon";
        case validated_pid_t::Proton:
          return "Proton";
        default:
          throw std::runtime_error( "Calling toString on invalid PID" );
        }
      }

      friend std::ostream& toStream( const PID& pid, std::ostream& os ) {
        return os << std::quoted( toString( pid ), '\'' );
      }
      friend std::ostream& operator<<( std::ostream& os, const PID& pid ) { return toStream( pid, os ); }

      friend StatusCode parse( PID& pid, std::string_view in ) {
        for ( PID ref : { Electron(), Muon(), Pion(), Kaon(), Proton() } ) {
          if ( in != toString( ref ) ) continue;
          pid = ref;
          return StatusCode::SUCCESS;
        }
        return StatusCode::FAILURE;
      }
      //----------------------------------------------------------

      constexpr double mass() const { return s_mass[static_cast<int>( m_value )]; }

      constexpr bool isElectron() const { return validated_pid_t::Electron == m_value; }

      constexpr bool isMuon() const { return validated_pid_t::Muon == m_value; }

      constexpr bool isPion() const { return validated_pid_t::Pion == m_value; }

      constexpr bool isKaon() const { return validated_pid_t::Kaon == m_value; }

      constexpr bool isProton() const { return validated_pid_t::Proton == m_value; }
    };
  } // namespace Tr
} // namespace Gaudi
