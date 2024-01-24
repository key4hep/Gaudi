#!/usr/bin/env python3
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
# Compare the properties from the current ParticleID, archived
# ParticleID, the Pythia particle database, and the particle property
# service.
import ParticleID
import ParticleProperty
import PythiaID

props = [prop for prop in ParticleID.properties(1)]
mthds = ["NEW", "OLD", "PY", "PP"]
for pid in sorted(ParticleID.pids):
    base = ParticleID.properties(pid)
    if not base["isSM"]:
        continue
    comps = {"NEW": base, "OLD": ParticleID.pids[pid], "PY": {}, "PP": {}}
    if pid in PythiaID.pids:
        comps["PY"] = PythiaID.pids[pid]
    if pid in ParticleProperty.pids:
        comps["PP"] = ParticleProperty.pids[pid]
    difs = [False] * len(props)
    for idx, prop in enumerate(props):
        for mthd, comp in comps.iteritems():
            if mthd in mthds and prop in comp and base[prop] != comp[prop]:
                difs[idx] = True
    if True in difs:
        print("------------------------------------")
        if "name" in comps["PY"]:
            print("PYTHIA: " + comps["PY"]["name"])
        if "name" in comps["PP"]:
            print("LHCB: " + comps["PP"]["name"])
        print("%15s" * (len(mthds) + 1) % tuple([str(pid)] + mthds))
        for prop, dif in zip(props, difs):
            if not dif:
                continue
            vals = [prop]
            for mthd in mthds:
                if mthd in comps and prop in comps[mthd]:
                    vals += [str(comps[mthd][prop])]
                else:
                    vals += [""]
            print("%15s" * (len(vals)) % tuple(vals))
