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
# @date 21/03/2018
# @author Philip Ilten
# Return the property dictionary for a given Gaudi::ParticleID.
def properties(pid):
    from PartProp.decorators import Gaudi

    pid = Gaudi.ParticleID(pid)
    return {
        "isSM": pid.isSM(),
        "isValid": pid.isValid(),
        "isMeson": pid.isMeson(),
        "isBaryon": pid.isBaryon(),
        "isDiQuark": pid.isDiQuark(),
        "isHadron": pid.isHadron(),
        "isLepton": pid.isLepton(),
        "isNucleus": pid.isNucleus(),
        "hasUp": pid.hasUp(),
        "hasDown": pid.hasDown(),
        "hasStrange": pid.hasStrange(),
        "hasCharm": pid.hasCharm(),
        "hasBottom": pid.hasBottom(),
        "hasTop": pid.hasTop(),
        "hasBottomPrime": pid.hasBottomPrime(),
        "hasTopPrime": pid.hasTopPrime(),
        "isQuark": pid.isQuark(),
        "hasQuarks": pid.hasQuarks(),
        "Z": pid.Z(),
        "A": pid.A(),
        "nLambda": pid.nLambda(),
        "threeCharge": pid.threeCharge(),
        "jSpin": pid.jSpin(),
        "sSpin": pid.sSpin(),
        "lSpin": pid.lSpin(),
    }


# Generate the PID dictionary and optionally archive.
def generate(dat=None):
    import ParticleProperty
    import PythiaID

    pids = PythiaID.pids
    pids.update(ParticleProperty.pids)

    # Loop over the particles and update entries.
    for pid in pids:
        pids[pid] = properties(pid)

    # Archive if requested.
    if dat:
        with open(dat, "w") as dat:
            dat.write(repr(pids))
    return pids


# Load the PID dictionary.
try:
    pids = eval(open("ParticleID.dat").read())
except FileNotFoundError:
    pids = generate("ParticleID.dat")
