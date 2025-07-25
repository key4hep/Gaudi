#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""Load the GaudiPython::Helper struct to check for Cling warnings."""


def test(capfd):
    # Workaround for ROOT-10769
    import warnings

    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        import cppyy

    cppyy.gbl.GaudiPython.Helper

    stderr = capfd.readouterr().err.strip()
    assert not stderr, "stderr must be empty"
