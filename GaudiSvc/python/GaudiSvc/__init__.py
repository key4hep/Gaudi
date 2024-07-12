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
def getMetaData(path):
    """
    Extract job metadata recorded by Gaudi::MetaDataSvc from a file.
    """
    import re

    import ROOT
    from ROOT import TFile

    version = tuple(int(i, 10) for i in re.split(r"\D", ROOT.__version__))

    f = TFile.Open(path)
    if not f:
        raise RuntimeError("cannot open {0}".format(path))
    try:
        if version < (6, 32):
            return dict(f.info)
        else:
            return {k.decode("utf-8"): v.decode("utf-8") for k, v in f["info"]}
    except AttributeError:
        return {}
