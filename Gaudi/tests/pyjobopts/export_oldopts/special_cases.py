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
from GaudiKernel.Configurable import ConfigurableAlgorithm


class MyAlg(ConfigurableAlgorithm):
    __slots__ = {"File": "filename", "Text": "text field"}

    def getDlls(self):
        return "Dummy"

    def getType(self):
        return "MyAlg"


alg = MyAlg()
alg.File = r"C:\something\vXrY\filename"
alg.Text = '''one line
\tanother "line"'''
