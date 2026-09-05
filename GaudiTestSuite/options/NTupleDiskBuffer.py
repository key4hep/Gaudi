#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
Ntuples of every shape the column-wise converter handles, so that the same job run
with and without NTupleSvc.DiskBuffer can be compared. The output stream is set by
the caller (NTupleSvc.Output) so both runs can share a directory.
"""

from Configurables import (
    ApplicationMgr,
    HistogramPersistencySvc,
    NTupleAlgorithm,
    WideNTupleAlgorithm,
)

# many trees, few entries: the shape the disk buffer is for
wide = WideNTupleAlgorithm(
    "Wide",
    NTuples=6,
    Columns=36,
    EntriesPerTuple=50,
    MaxArraySize=5,
    Directory="MyTuples/wide",
)
# one tree with more entries than fit in a basket
deep = WideNTupleAlgorithm(
    "Deep",
    NTuples=1,
    Columns=9,
    EntriesPerTuple=3000,
    MaxArraySize=8,
    Directory="MyTuples/deep/er",
)
# booked but never written
empty = WideNTupleAlgorithm(
    "Empty", NTuples=1, Columns=9, EntriesPerTuple=0, Directory="MyTuples/empty"
)

ApplicationMgr(
    # NTupleAlgorithm adds a row-wise tuple, which the disk buffer must leave alone
    TopAlg=[NTupleAlgorithm(), wide, deep, empty],
    EvtMax=3000,
    EvtSel="NONE",
    HistogramPersistency="ROOT",
)
HistogramPersistencySvc(OutputFile="")
