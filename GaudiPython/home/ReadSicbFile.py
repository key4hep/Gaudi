#----Load the needed DynamicLibraries
g.DLLs   = ['GaudiAlg', 'GaudiIntrospection','SicbCnv']

#----SICB stuff 
sicbcnv = g.service('SicbEventCnvSvc')
perssvc = g.service('EventPersistencySvc')
perssvc.CnvServices = [ 'SicbEventCnvSvc' ]
evtsel = g.service('EventSelector')
evtsel.Input = ["FILE='//cern.ch/dfs/Experiments/lhcb/data/mc/sicb_bpipi_v233_100ev.dst1'"]

#----Introspection Service Setup with its dictionaries
introssvc = g.service('IntrospectionSvc')
introssvc.Dictionaries = ['LHCbEventDict']


#----Application Manager configuration
g.ExtSvc = ['ParticlePropertySvc',
            'SicbEventCnvSvc', 
            'IntrospectionSvc']

g.EvtSel  = 'SICB'
g.HistogramPersistency = 'NONE'
g.topAlg  = []
g.EvtMax  = 10

print g.properties()

