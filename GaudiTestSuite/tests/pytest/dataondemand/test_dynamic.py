#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting import GaudiExeTest


class TestDynamic(GaudiExeTest):
    command = ["gaudirun.py", "../../../options/DataOnDemand.py"]
    reference = "../refs/DataOnDemandDyn.yaml"

    def options(self):
        # modify the DataOnDemandSvc configuration to use the dynamic mapping.
        from Configurables import DataOnDemandSvc, DODBasicMapper

        from Gaudi.Configuration import ApplicationMgr, ToolSvc

        dod = DataOnDemandSvc()
        mapper = DODBasicMapper()

        mapper.Algorithms = dod.AlgMap
        dod.AlgMap = {}

        mapper.Nodes = dod.NodeMap
        dod.NodeMap = {}

        import re

        obsolete = re.compile(r"DATA='([^']*)'\s*TYPE='([^']*)'")

        for m in filter(None, map(obsolete.match, dod.Algorithms)):
            mapper.Algorithms[m.group(1)] = m.group(2)
        dod.Algorithms = []

        for m in filter(None, map(obsolete.match, dod.Nodes)):
            mapper.Nodes[m.group(1)] = m.group(2)
        dod.Nodes = []

        dod.NodeMappingTools = [mapper]
        dod.AlgMappingTools = [mapper]

        # Ensure that the ToolSvc is finalized after the DOD
        ApplicationMgr().ExtSvc.insert(
            0, ToolSvc()
        )  # equivalent to '= [ToolSvc(), dod]'
