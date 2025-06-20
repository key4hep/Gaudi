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
// -*- C++ -*-
// ----------------------------------------------------------------------
// HEP coherent Physical Constants
//
// This file has been provided by Geant4 (simulation toolkit for HEP).
//
// The basic units are :
//  		millimeter
// 		nanosecond
// 		Mega electron Volt
// 		positon charge
// 		degree Kelvin
//              amount of substance (mole)
//              luminous intensity (candela)
// 		radian
//              steradian
//
// Below is a non exhaustive list of Physical CONSTANTS,
// computed in the Internal HEP System Of Units.
//
// Most of them are extracted from the Particle Data Book :
//        Phys. Rev. D  volume 50 3-1 (1994) page 1233
//
//        ...with a meaningful (?) name ...
//
// You can add your own constants.
//
// Author: M.Maire
//
// History:
//
// 23.02.96 Created
// 26.03.96 Added constants for standard conditions of temperature
//          and pressure; also added Gas threshold.
// 28.04.06 Imported from CLHEP into GaudiKernel -- HD

#ifndef GAUDI_PHYSICAL_CONSTANTS_H
#define GAUDI_PHYSICAL_CONSTANTS_H

#include <GaudiKernel/SystemOfUnits.h>

namespace Gaudi {
  namespace Units {

    constexpr double pi     = 3.14159265358979323846;
    constexpr double twopi  = 2 * pi;
    constexpr double halfpi = pi / 2;
    constexpr double pi2    = pi * pi;

    constexpr double Avogadro = 6.02214076e+23 / mole;

    //
    // c   = 299.792458 mm/ns
    // c^2 = 898.7404 (mm/ns)^2
    //
    constexpr double c_light   = 2.99792458e+8 * m / s;
    constexpr double c_squared = c_light * c_light;

    //
    // h     = 4.13566e-12 MeV*ns
    // hbar  = 6.58212e-13 MeV*ns
    // hbarc = 197.32705e-12 MeV*mm
    //
    constexpr double h_Planck      = 6.62607015e-34 * joule * s;
    constexpr double hbar_Planck   = h_Planck / twopi;
    constexpr double hbarc         = hbar_Planck * c_light;
    constexpr double hbarc_squared = hbarc * hbarc;

    constexpr double electron_charge = -eplus; // see SystemOfUnits.h
    constexpr double e_squared       = eplus * eplus;

    //
    // amu_c2 - atomic equivalent mass unit
    // amu    - atomic mass unit
    //
    constexpr double electron_mass_c2 = 0.510998910 * MeV;
    constexpr double proton_mass_c2   = 938.272013 * MeV;
    constexpr double neutron_mass_c2  = 939.56536 * MeV;
    constexpr double amu_c2           = 931.494028 * MeV;
    constexpr double amu              = amu_c2 / c_squared;

    //
    // permeability of free space mu0    = 2.01334e-16 Mev*(ns*eplus)^2/mm
    // permittivity of free space epsil0 = 5.52636e+10 eplus^2/(MeV*mm)
    //
    constexpr double mu0      = 4 * pi * 1.e-7 * henry / m;
    constexpr double epsilon0 = 1. / ( c_squared * mu0 );

    //
    // electromagnetic coupling = 1.43996e-12 MeV*mm/(eplus^2)
    //
    constexpr double elm_coupling            = e_squared / ( 4 * pi * epsilon0 );
    constexpr double fine_structure_const    = elm_coupling / hbarc;
    constexpr double classic_electr_radius   = elm_coupling / electron_mass_c2;
    constexpr double electron_Compton_length = hbarc / electron_mass_c2;
    constexpr double Bohr_radius             = electron_Compton_length / fine_structure_const;

    constexpr double alpha_rcl2 = fine_structure_const * classic_electr_radius * classic_electr_radius;

    constexpr double twopi_mc2_rcl2 = twopi * electron_mass_c2 * classic_electr_radius * classic_electr_radius;

    constexpr double k_Boltzmann = 8.617333e-11 * MeV / kelvin;

    constexpr double STP_Temperature = 273.15 * kelvin;
    constexpr double STP_Pressure    = 1. * atmosphere;
    constexpr double kGasThreshold   = 10. * mg / cm3;

    constexpr double universe_mean_density = 1.e-25 * g / cm3;

  } // namespace Units
} // namespace Gaudi

#endif /* GAUDI_PHYSICAL_CONSTANTS_H */
