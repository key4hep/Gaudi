#--Initialize ------------------------------------------------
execfile('../options/AnalysisInit.py')
#-------------------------------------------------------------

#-------------------------------------------------------------
def electron(c):
#-------------------------------------------------------------
	if c : return 1
	return 0

#-------------------------------------------------------------
def initialize():
#-------------------------------------------------------------
	global h0, h1, h2
	try:
		h0 = his.histo('h0')
		h1 = his.histo('h1')
		h2 = his.histo('h2')
	except:
		h0 = his.book('h0', 'Histogram 1D for random tests', 35, -10., 10.) 
		h1 = his.book('h1', 'Histogram 1D for tests', 20, 0., 40.) 
		h2 = his.book('h2', 'Histogram 2D for test2', 20, 0., 40., 20, 0., 10.) 

#-------------------------------------------------------------
def execute(): 
#-------------------------------------------------------------
  global cands
	cands = evt['Anal/AxParticleCandidates']

	electrons = select(cands,electron)
	print 'MyTracks collection contains ' + `len(tks)` + ' tracks and ' + `len(electrons)` + ' electrons'


#-------End of declarative part-----------------------------------------------------

#g.run(10)
#excel.plot(h1)

g.run(1)
sel = evt['Phys/PhysSel']
sel.decayIsInMCTree('B0PP')





