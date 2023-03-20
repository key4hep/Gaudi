#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# File: AthenaCommon/share/PhysicalConstants.py
# Author: Wim Lavrijsen (LBNL, WLavrijsen@lbl.gov)
# Created: 01/21/04
# Last: 10/15/04

# This script is a direct adaptation of CLHEP/Units/PhysicalConstants.h
# and the following is the originial CLHEP comment:
#
# -----
# This file has been provided by Geant4 (simulation toolkit for HEP).
#
# The basic units are :
#               millimeter
#               nanosecond
#               Mega electron Volt
#               positon charge
#               degree Kelvin
#               amount of substance (mole)
#               luminous intensity (candela)
#               radian
#               steradian
#
# Below is a non exhaustive list of Physical CONSTANTS,
# computed in the Internal HEP System Of Units.
#
# Most of them are extracted from the Particle Data Book :
#        Phys. Rev. D  volume 50 3-1 (1994) page 1233
#
#        ...with a meaningful (?) name ...
#
# You can add your own constants.
#
# Author: M.Maire
#
# History:
#
# 23.02.96 Created
# 26.03.96 Added constants for standard conditions of temperature
#          and pressure; also added Gas threshold.
# 08.07.20 Updated
# -----

import GaudiKernel.SystemOfUnits as Units

#
#
#
pi = 3.14159265358979323846
twopi = 2 * pi
halfpi = pi / 2
pi2 = pi * pi

#
#
#
Avogadro = 6.0221367e23 / Units.mole

#
# c   = 299.792458 mm/ns
# c^2 = 898.7404 (mm/ns)^2
#
c_light = 2.99792458e8 * Units.m / Units.s
c_squared = c_light * c_light

#
# h     = 4.13566e-12 MeV*ns
# hbar  = 6.58212e-13 MeV*ns
# hbarc = 197.32705e-12 MeV*mm
#
h_Planck = 6.62606896e-34 * Units.joule * Units.s
hbar_Planck = h_Planck / twopi
hbarc = hbar_Planck * c_light
hbarc_squared = hbarc * hbarc

#
#
#
electron_charge = -Units.eplus  # see SystemOfUnits.h
e_squared = Units.eplus * Units.eplus

#
# amu_c2 - atomic equivalent mass unit
# amu    - atomic mass unit
#
electron_mass_c2 = 0.510998910 * Units.MeV
proton_mass_c2 = 938.272013 * Units.MeV
neutron_mass_c2 = 939.56536 * Units.MeV
amu_c2 = 931.494028 * Units.MeV
amu = amu_c2 / c_squared

#
# permeability of free space mu0    = 2.01334e-16 Mev*(ns*eplus)^2/mm
# permittivity of free space epsil0 = 5.52636e+10 eplus^2/(MeV*mm)
#
mu0 = 4 * pi * 1.0e-7 * Units.henry / Units.m
epsilon0 = 1.0 / (c_squared * mu0)

#
# electromagnetic coupling = 1.43996e-12 MeV*mm/(eplus^2)
#
elm_coupling = e_squared / (4 * pi * epsilon0)
fine_structure_const = elm_coupling / hbarc
classic_electr_radius = elm_coupling / electron_mass_c2
electron_Compton_length = hbarc / electron_mass_c2
Bohr_radius = electron_Compton_length / fine_structure_const

alpha_rcl2 = fine_structure_const * classic_electr_radius * classic_electr_radius

twopi_mc2_rcl2 = (
    twopi * electron_mass_c2 * classic_electr_radius * classic_electr_radius
)
#
#
#
k_Boltzmann = 8.617343e-11 * Units.MeV / Units.kelvin

#
#
#
STP_Temperature = 273.15 * Units.kelvin
STP_Pressure = 1.0 * Units.atmosphere
kGasThreshold = 10.0 * Units.mg / Units.cm3

#
#
#
universe_mean_density = 1.0e-25 * Units.g / Units.cm3
