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
from GaudiTesting.preprocessors import RegexpReplacer, normalizeTestSuite


class Test(GaudiExeTest):
    command = ["../../qmtest/scripts/HistoEx2.py"]
    reference = "../refs/HistoEx2.pyref.yaml"

    preprocessor = normalizeTestSuite + RegexpReplacer(
        when="AIDA",
        orig=r"ROOT\.AIDA::IProfile1D",
        repl=r"cppyy.gbl.AIDA.IProfile1D",
    )
