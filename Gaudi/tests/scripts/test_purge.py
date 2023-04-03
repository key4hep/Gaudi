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
from Gaudi.Configuration import configurationDict, importOptions
from GaudiKernel.Configurable import purge

option_file = "test_purge1.py"

importOptions(option_file)
first = configurationDict()

purge()
purged = configurationDict()

importOptions(option_file)
second = configurationDict()

from pprint import PrettyPrinter

pp = PrettyPrinter()

print("first =", end=" ")
pp.pprint(first)

print("purged =", end=" ")
pp.pprint(purged)

print("second =", end=" ")
pp.pprint(second)
