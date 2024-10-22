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


class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/ModelDFAmbiguity.py"]
    timeout = 120
    returncode = 1

    test_signature = GaudiExeTest.find_reference_block(
        """
        PrecedenceSvc       ERROR   Conditional (C) and/or unconditional (U) topology violations found in the DF realm:

         (U): '/Event/A' <---- | UpdaterAlg2 (U) | UpdaterAlg4 (U) | ProducerAlg1 (C) | UpdaterAlg1 (C) |
         (C): '/Event/B' <---- | ProducerAlg2 (C) | UpdaterAlg3 (C) |
        """
    )
