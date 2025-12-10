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
from GaudiTesting.preprocessors import LineSkipper


class Test(GaudiExeTest):
    command = ["gaudirun.py", "GaudiExamples.TinyExperiment.FullExperiment:config"]
    reference = "../refs/FullExperiment.yaml"
    # FIXME: this is needed until we start using an LCG view that contains
    #        Gaudi >= 40.0 in Gitlab-CI jobs
    preprocessor = GaudiExeTest.preprocessor + LineSkipper(["AuditAlgorithms"])
