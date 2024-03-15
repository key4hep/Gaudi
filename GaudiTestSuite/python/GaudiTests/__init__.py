#####################################################################################
# (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
def run_gaudi(*args, **kwargs):
    """
    Helper to simplify the invocation of gaudirun.py in tests.
    """
    from subprocess import run

    cmd = ["gaudirun.py"]
    cmd.extend(str(x) for x in args)

    print("running", cmd)
    return run(cmd, **kwargs)
