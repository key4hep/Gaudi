#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
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
    from ROOT import TFile

    f = TFile.Open(path)
    if not f:
        raise RuntimeError("cannot open {0}".format(path))
    try:
        return dict(f.info)
    except AttributeError:
        return {}
