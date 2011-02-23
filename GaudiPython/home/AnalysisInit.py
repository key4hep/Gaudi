""" Simple Analysis Tools
"""
from math import *
from random import *
from gaudiextra import PartSvc

#-------------------Algorithm-----------------------------------------------
class PhysAnalAlg(PyAlgorithm):
	def initialize(self):
		print 'Initializing User Analysis...'
		global evt, his, det, pdt
		his  = g.histoSvc()
		evt  = g.evtSvc()
		det  = g.detSvc()
		pdt  = PartSvc()
		initialize()
		print '....User Analysis Initialized'
		return 1
	def execute(self):
		execute()
		return 1 
	def finalize(self):
		print 'Finalizing User Analysis...'
		finalize()
		print '....User Analysis Finalized'
		return 1 

#-------------------Global utility fucntions--------------------------------
def select(list,sel):
	r = []
	for i in list:
		if apply(sel,[i]): r.append(i)
	return r

#-------------------Configuration of Appliaction ---------------------------

physalg  = PhysAnalAlg(g,'PhysAnalAlg')
g.topAlg = g.topAlg + ['PhysAnalAlg']
g.DLLs   = g.DLLs   + ['GaudiIntrospection']
g.ExtSvc = g.ExtSvc + ['IntrospectionSvc', 'ParticlePropertySvc']







