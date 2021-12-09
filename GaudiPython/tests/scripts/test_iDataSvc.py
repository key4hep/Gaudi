#####################################################################################
# (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import GaudiPython

DO = GaudiPython.gbl.DataObject

objects = [DO() for i in range(52)]

# Once the object is registered the TES will clean it up
import ROOT

for i in objects:
    ROOT.SetOwnership(i, False)

app = GaudiPython.AppMgr()
tes = app.evtsvc()

app.start()

tes.setRoot("Event", objects[51])
tes["/Event/0"] = objects[50]

path = "/Event"
for i in range(5):
    path = path + "/{}".format(i)
    for j in range(1, 10):
        p = path + "/" + str(j)
        print("Registering Object at : " + p)

        if i % 2 == 0:
            tes[p] = objects[i * 10 + j]
        else:
            tes.registerObject(p, objects[i * 10 + j])

        ret = tes[p]
        print("Access of {} yields {}".format(p, ret))
        if not ret:
            raise Exception(
                "Access of path: {}, should not result in nullptr".format(p)
            )

tes.dump()
