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
# ///////////////////////////////////////////////////////////
# Job Options File in Python

theApp.DLLs = ["RootHistCnv", "HbookCnv", "GaudiAlg", "GaudiAud"]
theApp.TopAlg = ["RandomNumberAlg"]

# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
MessageSvc = Service("MessageSvc")
MessageSvc.OutputLevel = 3

# --------------------------------------------------------------
# Event related parameters
# --------------------------------------------------------------
theApp.EvtMax = 100
theApp.EvtSel = "NONE"

# --------------------------------------------------------------
# Other Service Options
# --------------------------------------------------------------
# Histogram output file
theApp.HistogramPersistency = "ROOT"

HPSvc = Service("RootHistCnv::PersSvc/HistogramPersistencySvc")
HPSvc.OutputFile = "histo.root"
NTSvc = Service("NTupleSvc")
NTSvc.Output = ["FILE1 DATAFILE='NTuple.root' OPT='NEW' TYP='ROOT'"]

theApp.run(theApp.EvtMax)
theApp.exit()
