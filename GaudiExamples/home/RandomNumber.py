#///////////////////////////////////////////////////////////
# Job Options File in Python

theApp.DLLs   = [ 'RootHistCnv', 'HbookCnv', 'GaudiAlg', 'GaudiAud']
theApp.TopAlg = [ 'RandomNumberAlg' ]

# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
MessageSvc = Service('MessageSvc')
MessageSvc.OutputLevel  = 3


#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
theApp.EvtMax   = 100
theApp.EvtSel  = "NONE"

#--------------------------------------------------------------
# Other Service Options
#--------------------------------------------------------------
# Histogram output file
theApp.HistogramPersistency = 'ROOT'

HPSvc = Service('RootHistCnv::PersSvc/HistogramPersistencySvc')
HPSvc.OutputFile = 'histo.root'
NTSvc = Service('NTupleSvc')
NTSvc.Output = ["FILE1 DATAFILE='NTuple.root' OPT='NEW' TYP='ROOT'"]

theApp.run(theApp.EvtMax)
theApp.exit()
