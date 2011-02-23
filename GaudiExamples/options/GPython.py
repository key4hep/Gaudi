#///////////////////////////////////////////////////////////
# Job Options File in Python

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

NTSvc = Service('NTupleSvc')
NTSvc.Output = ["FILE1 DATAFILE='NTuple.root' OPT='NEW' TYP='ROOT'"]
HPSvc = Service('HistogramPersistencySvc')
HPSvc.OutputFile = 'histo.root'

theApp.run(theApp.EvtMax)
theApp.exit()
