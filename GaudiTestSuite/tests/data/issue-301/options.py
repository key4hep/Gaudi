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
from pathlib import Path

# Test exposing https://gitlab.cern.ch/gaudi/Gaudi/-/issues/301
from Configurables import EventSelector, GaudiPersistency

test_file_path = Path(__file__).parent / "test-file.dst"

GaudiPersistency()
EventSelector(
    Input=[f"DATAFILE='PFN:{test_file_path}' SVC='Gaudi::RootEvtSelector' OPT='READ'"]
)
