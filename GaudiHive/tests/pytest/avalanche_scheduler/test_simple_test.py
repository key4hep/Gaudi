#####################################################################################
# (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/AvalancheSchedulerSimpleTest.py"]
    timeout = 120

    def test_graph_file(self, cwd):
        ref = """digraph datadeps {
  rankdir="LR";
  Alg_0 [label="A1";shape=box];
  obj_13600394293829281201 [label="/Event/a1";shape=box];
  Alg_0 -> obj_13600394293829281201;
  Alg_1 [label="A2";shape=box];
  obj_13600394293829281201 -> Alg_1;
  obj_1573251618719251457 [label="/Event/a2";shape=box];
  Alg_1 -> obj_1573251618719251457;
  Alg_2 [label="A3";shape=box];
  obj_13600394293829281201 -> Alg_2;
  obj_15861236635915656172 [label="/Event/a3";shape=box];
  Alg_2 -> obj_15861236635915656172;
  Alg_3 [label="A4";shape=box];
  obj_15861236635915656172 -> Alg_3;
  obj_1573251618719251457 -> Alg_3;
  obj_9576344950949101152 [label="/Event/a4";shape=box];
  Alg_3 -> obj_9576344950949101152;
  Alg_4 [label="CNT&";shape=box];
  Alg_5 [label="CNT*";shape=box];
}
"""
        graph = open(f"{cwd}/graphDump.dot").read()
        assert graph == ref
