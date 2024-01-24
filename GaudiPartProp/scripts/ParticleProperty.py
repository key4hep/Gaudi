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
# Return the property dictionary for a given Gaudi::ParticleProperty.
def properties(pid):
    return {
        "name": pid.name(),
        "threeCharge": int(round(pid.charge() * 3)),
        "hasAnti": pid.selfcc(),
    }


# Generate the PID dictionary and optionally archive.
def generate(dat=None):
    # The following is misunderstood by flake8 - the import is needed as it
    # has necessary side effects
    import GaudiPartProp.PartPropSvc  # noqa: F401
    from GaudiPython.Bindings import AppMgr

    gaudi = AppMgr()
    ppsvc = gaudi.gaudiPartProp()

    # Loop over the particles and add entries.
    pids = {}
    for pid in ppsvc:
        pids[pid.pid().pid()] = properties(pid)

    # Archive if requested.
    if dat:
        with open(dat, "w") as dat:
            dat.write(repr(pids))
    return pids


# Load the PID dictionary.
try:
    pids = eval(open("ParticleProperty.dat").read())
except FileNotFoundError:
    pids = generate("ParticleProperty.dat")
