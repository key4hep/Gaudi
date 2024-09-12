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
    command = ["Gaudi.exe", "../qmtest/options/Aida2Root.opts"]
    reference = "refs/Aida2Root.yaml"

    preprocessor = normalizeTestSuite + RegexpReplacer(
        when="^Aida2Root",
        orig=r"(INFO.*'(skewness|kurtosis)(Err)?'.*)\|([0-9.e+\- ]*\|){2}",
        repl=r"\1| ### | ### |",
    )
