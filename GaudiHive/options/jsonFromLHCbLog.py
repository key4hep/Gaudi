#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from __future__ import print_function

from pygraph.classes.graph import graph


def load_brunel_scenario(filename):
    gr = graph()
    algs = {}
    timing = {}
    objs = []
    curr = None
    order = 0
    nodes = ("/Event", "/Event/Rec", "/Event/DAQ")
    for l in open(filename).readlines():
        if l.find("StoreTracer") == 0:
            if l.find("Executing Algorithm") != -1:
                alg = l.split()[-1]
                if alg not in algs.keys():
                    algs[alg] = (order, set(), set())
                curr = alg
                order += 1
            elif l.find("Done with Algorithm") != -1:
                curr = None
            elif l.find("[EventDataSvc]") != -1 and curr:
                obj = l.split()[-1]
                if obj in nodes:
                    continue
                if obj.find("/Event/") == 0:
                    obj = obj[7:]
                obj = obj.replace("/", "_")
                if obj not in objs:
                    objs.append(obj)
                talg = algs[curr]
                if l.find("RETRIEVE") != -1:
                    if obj not in talg[1]:
                        talg[1].add(obj)
                elif l.find("REGOBJ") != -1:
                    if obj not in talg[2]:
                        talg[2].add(obj)
        if l.find("TimingAuditor") != -1:
            algo = l.split()[2]  # .rstrip("|")
            index = 13
            if algo.endswith("|"):
                index = 12
                algo = algo.rstrip("|")
            if algo in algs.keys():
                timing[algo] = l.split()[index]
            else:
                for name in algs.keys():
                    if name.startswith(algo):
                        timing[name] = l.split()[index]

    all_algos = []
    for i, (alg, deps) in enumerate(algs.items()):
        if alg in ["PatPVOffline", "PrsADCs"]:
            continue
        if deps[1] or deps[2]:
            inputs = []
            inputs = [
                item
                for item in deps[1]
                if item not in ("DAQ_ODIN", "DAQ_RawEvent") and item not in deps[2]
            ]
            outputs = [item for item in deps[2]]
            new_algo = {
                "name": alg,
                "inputs": inputs,
                "outputs": outputs,
                "runtimes_wall": [float(timing[alg])],
            }
            all_algos.append(new_algo)
    return all_algos


#####
if __name__ == "__main__":
    json = {}
    json["algorithms"] = load_brunel_scenario("Brunel.TES.trace.log")
    print(json)
