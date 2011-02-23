import unittest
import GaudiPython

g = GaudiPython.AppMgr(outputlevel=5)
g.initialize()

class BasicTests(unittest.TestCase):
  def setUp(self):
    pass
  def tearDown(self):
    pass

  def test01ProtoProperties(self):
    #--When not yet existing
    alg = g.algorithm('myalg')
    alg.strings = ['string1', 'string2']
    alg.string = 'word1 word2'
    alg.numbers = [1,2,3]
    alg.number = 7.8
    self.failUnless( len(alg.strings) == 2, 'strings len fail' )
    self.failUnless( alg.strings == ['string1', 'string2'], 'strings len fail' )
    self.failUnless( len(alg.numbers) == 3, 'strings fail' )
    self.failUnless( alg.numbers == [1,2,3], 'numbers fail' )
    self.failUnless( alg.number == 7.8, 'fail to set float property' )
    self.failUnless( alg.string == 'word1 word2', 'fail to set string property' )
    self.failUnless( alg.number == 7.8, 'fail to set float property' )

  def test02Properties(self):
    properties = g.properties()
    for p in properties :
      self.failUnlessEqual(properties[p].value(), getattr(g, str(p)))

  def test03Properties(self) :  
    g.Dlls   += ['GPyTest']
    g.topAlg += ['PropertyAlg']
    p = g.algorithm('PropertyAlg')
    #---Bool
    self.failUnlessEqual( p.Bool, True )
    p.Bool = False
    self.failUnlessEqual( p.Bool, False )
    #---Char
    """
    self.failUnlessEqual( p.Char, chr(100) )
    p.Char = chr(99)
    self.failUnlessEqual( p.Char, chr(99) )    
    self.failUnlessEqual( p.Schar, chr(100) )
    p.Schar = chr(99)
    self.failUnlessEqual( p.Schar, chr(99) )    
    self.failUnlessEqual( p.Uchar, chr(100) )
    p.Uchar = chr(99)
    self.failUnlessEqual( p.Uchar, chr(99) )
    """
    #---Short
    self.failUnlessEqual( p.Short, 100 )
    p.Short = -99
    self.failUnlessEqual( p.Short, -99 )
    self.failUnlessEqual( p.Ushort, 100 )
    p.Ushort = 99 
    self.failUnlessEqual( p.Ushort, 99 )
    #---Int
    self.failUnlessEqual( p.Int, 100 )
    p.Int = -999
    self.failUnlessEqual( p.Int, -999 )
    self.failUnlessEqual( p.Uint, 100 )
    p.Uint = 999 
    self.failUnlessEqual( p.Uint, 999 )
    #---Long
    self.failUnlessEqual( p.Long, 100 )
    p.Long = -9999
    self.failUnlessEqual( p.Long, -9999 )
    self.failUnlessEqual( p.Ulong, 100 )
    p.Ulong = 9999 
    self.failUnlessEqual( p.Ulong, 9999 )
    #---LongLong
    #self.failUnlessEqual( p.Longlong, 100 )
    #p.Longlong = -99999
    #self.failUnlessEqual( p.Longlong, -99999 )
    #self.failUnlessEqual( p.Ulonglong, 100 )
    #p.Ulonglong = 99999 
    #self.failUnlessEqual( p.Ulonglong, 99999 )
    #---Float
    self.failUnlessEqual( p.Float, 100. )
    p.Float = -99.00
    self.failUnlessEqual( p.Float, -99.00 )
    #---Double
    self.failUnlessEqual( p.Double, 100. )
    p.Double = 999.999 
    self.failUnlessEqual( p.Double, 999.999 )
    #---String
    self.failUnlessEqual( p.String, 'hundred' )
    p.String = 'another string' 
    self.failUnlessEqual( p.String, 'another string' )
    #---Bool Array
    self.failUnlessEqual( p.BoolArray, [] )
    p.BoolArray = [True,False]
    self.failUnlessEqual( p.BoolArray, [True,False] )
    #---Char Array
    self.failUnlessEqual( p.CharArray, [] )
    p.CharArray = [0x20, 0x21]
    #---FIXME---self.failUnlessEqual( p.CharArray, [0x20,0x21] )
    #---Short Array
    self.failUnlessEqual( p.ShortArray, [] )
    p.ShortArray = range(10)
    #---FIXME---self.failUnlessEqual( p.ShortArray, range(10) )
    #---Int Array
    self.failUnlessEqual( p.IntArray, [] )
    p.IntArray = range(10,0,-1)
    self.failUnlessEqual( p.IntArray, range(10,0,-1) )
    #---Long Array
    self.failUnlessEqual( p.LongArray, [] )
    p.LongArray = range(10,0,-1)
    self.failUnlessEqual( p.LongArray, range(10,0,-1) )
    #---LongLong Array
    #self.failUnlessEqual( p.LongLongArray, [] )
    #p.LongLongArray = range(10,0,-1)
    #self.failUnlessEqual( p.LongLongArray, range(10,0,-1) )
    #---Float Array
    self.failUnlessEqual( p.FloatArray, [] )
    p.FloatArray = [ 1., 2., 3. ]
    self.failUnlessEqual( p.FloatArray, [1., 2., 3.] )
    #---Double Array
    self.failUnlessEqual( p.DoubleArray, [] )
    p.DoubleArray = [ 1., 2., 3. ]
    self.failUnlessEqual( p.DoubleArray, [1., 2., 3.] )
    #---String Array
    self.failUnlessEqual( p.StringArray, [] )
    p.StringArray = [ 'abc', 'def', 'g' ]
    self.failUnlessEqual( p.StringArray, ['abc','def','g'] )
    p.StringArray += [ 'xyz' ]
    self.failUnlessEqual( p.StringArray, ['abc','def','g', 'xyz'] )

  def test03ReadProperties(self):
    g.Dlls   += ['GaudiAlg']
    f = open('tmp.opts','w')
    f.write('s1.members = {"Prescaler/p1","Prescaler/p2"};\n')
    f.write('p1.ErrorMax = 99;\n')
    f.write('p2.ErrorMax = 88;\n')
    f.close()
    g.readOptions('tmp.opts')
    g.topAlg += ['Sequencer/s1']
    g.HistogramPersistency = 'NONE'
    g.reinitialize()
    p1 = g.algorithm('p1')
    p2 = g.algorithm('p2')
    self.failUnless(p1)
    self.failUnless(p2)
    self.failUnlessEqual(p1.ErrorMax, 99)
    self.failUnlessEqual(p2.ErrorMax, 88)

  def test04EventStore(self):
    evt = g.datasvc('EventDataSvc')
    objs = []
    for i in range(10) : objs.append(GaudiPython.gbl.DataObject())
    for o in objs : o.addRef()    # To keep the ownership
    self.failUnless(evt)
    self.failUnless(evt.setRoot('Root',objs[1]).isSuccess())
    self.failUnless(evt.registerObject('AB',objs[2]).isSuccess())
    self.failUnless(evt.registerObject('A',objs[3]).isSuccess())
    self.failUnless(evt.registerObject('B',objs[4]).isSuccess())
    self.failUnless(evt.registerObject('A/O1',objs[5]).isSuccess())
    self.failUnless(evt.registerObject('A/O2',objs[6]).isSuccess())
    self.failUnless(evt.registerObject('B/O1',objs[7]).isSuccess())
    self.failUnless(evt.registerObject('B/O2',objs[8]).isSuccess())
    o = evt.retrieveObject('/Root/A/O1')
    self.failUnless(o)
    self.failUnlessEqual(o.registry().identifier(), '/Root/A/O1')
    self.failUnlessEqual(len(evt.leaves()), 3)
    o = evt['B']
    self.failUnlessEqual(len(evt.leaves(o)), 2)
    self.failUnlessEqual(evt.leaves(o)[0].name(), '/O1')
    self.failUnlessEqual(evt.leaves(o)[1].name(), '/O2')
    self.failUnlessEqual(evt.leaves(o)[0].identifier(), '/Root/B/O1')
    self.failUnlessEqual(evt.leaves(o)[1].identifier(), '/Root/B/O2')
    self.failUnless(evt.clearStore().isSuccess())


  def test05HistogramSvc(self):
    hsvc = g.histsvc()
    self.failUnless(hsvc)
    h1 = hsvc.book('h1', 'Histogram title 1D', 10, 0, 10)
    self.failUnless(h1)
    self.failUnlessEqual(h1.title(), 'Histogram title 1D')
    self.failUnlessEqual(h1.axis().bins(), 10)
    self.failUnlessEqual(h1.axis().lowerEdge(), 0.)
    self.failUnlessEqual(h1.axis().upperEdge(), 10.)
    self.failUnlessEqual(h1.entries(), 0)
    h1.fill(5)
    self.failUnlessEqual(h1.entries(), 1)
    h2 = hsvc.book('h2', 'Histogram title 2D', 10, 0, 10, 20, 0, 10)
    self.failUnless(h2)
    self.failUnlessEqual(h2.title(), 'Histogram title 2D')
    self.failUnlessEqual(h2.xAxis().bins(), 10)
    self.failUnlessEqual(h2.xAxis().lowerEdge(), 0.)
    self.failUnlessEqual(h2.xAxis().upperEdge(), 10.)
    self.failUnlessEqual(h2.yAxis().bins(), 20)
    self.failUnlessEqual(h2.yAxis().lowerEdge(), 0.)
    self.failUnlessEqual(h2.yAxis().upperEdge(), 10.)
    self.failUnlessEqual(h2.entries(), 0)
    for i in range(10) : h2.fill(i,i)
    self.failUnlessEqual(h2.entries(), 10)
    h3 = hsvc.book('h3', 'Histogram title 3D', 10, 0, 10, 20, 0, 10, 30, -10, 10)
    self.failUnless(h2)
    self.failUnlessEqual(h3.title(), 'Histogram title 3D')
    self.failUnlessEqual(h3.xAxis().bins(), 10)
    self.failUnlessEqual(h3.xAxis().lowerEdge(), 0.)
    self.failUnlessEqual(h3.xAxis().upperEdge(), 10.)
    self.failUnlessEqual(h3.yAxis().bins(), 20)
    self.failUnlessEqual(h3.yAxis().lowerEdge(), 0.)
    self.failUnlessEqual(h3.yAxis().upperEdge(), 10.)
    self.failUnlessEqual(h3.zAxis().bins(), 30)
    self.failUnlessEqual(h3.zAxis().lowerEdge(), -10.)
    self.failUnlessEqual(h3.zAxis().upperEdge(), 10.)
    self.failUnlessEqual(h3.entries(), 0)
    for i in range(10) : h3.fill(i,i,i)
    self.failUnlessEqual(h3.entries(), 10)
    th1 = hsvc.retrieve1D('h1')
    self.failUnless(th1)
    self.failUnlessEqual(th1.title(), 'Histogram title 1D')
    th2 = hsvc.retrieve2D('/stat/h2')
    self.failUnless(th2)
    self.failUnlessEqual(th2.title(), 'Histogram title 2D')
    th3 = hsvc.retrieve3D('/stat/h3')
    self.failUnless(th3)
    self.failUnlessEqual(th3.title(), 'Histogram title 3D')
    th = hsvc.retrieve('h2')
    self.failUnless(th)
    self.failUnlessEqual(th.title(), 'Histogram title 2D')


  def test06Config(self):
    f = open('tmp2.opts','w')
    f.write('ApplicationMgr.EvtMax = -1;')
    f.write('ApplicationMgr.Dlls += { "GaudiAlg" };')
    f.close()
    g.config( files = ['tmp2.opts'], 
              options = ['ApplicationMgr.Go = 123'] )
    self.failUnlessEqual(g.DLLs, ['GPyTest', 'GaudiAlg'])
    self.failUnlessEqual(g.Go, 123)
    
  def test07NTupleSvc(self) :
    ntsvc = g.ntuplesvc()
    self.failUnless(ntsvc)
    ntsvc.defineOutput({'L1':'file1.root','L2':'file2.root'})
    self.failUnless(ntsvc.initialize().isSuccess())
    
  def test08Buffer(self) :
    gbl = GaudiPython.gbl
    vi = gbl.std.vector(int)()
    vi.push_back(10)
    vi.push_back(20)
    vi.push_back(30)
    vi.push_back(40)
    ai = getattr(gbl.GaudiPython.Helper, 'toAddress<int>')(vi)
    self.failUnless(ai)
    bi = gbl.GaudiPython.Helper.toIntArray(ai,vi.size())
    self.failUnless(bi)
    self.failUnlessEqual(len(bi),len(vi))
    for i in range(len(vi)) : self.failUnlessEqual(bi[i],vi[i])
    #double
    vd = gbl.std.vector('double')()
    vd.push_back(10.)
    vd.push_back(20.)
    vd.push_back(30.)
    vd.push_back(40.)
    ad = getattr(gbl.GaudiPython.Helper, 'toAddress<double>')(vd)
    self.failUnless(ad)
    bd = gbl.GaudiPython.Helper.toDoubleArray(ad,vd.size())
    self.failUnless(bd)
    self.failUnlessEqual(len(bd),len(vd))
    for i in range(len(vd)) : self.failUnlessEqual(bd[i],vd[i])
    
  def test09GlobalTools(self) :
    svc = g.toolsvc()
    self.failUnless(svc)
    t1 = svc.create('SequencerTimerTool','tool1')
    self.failUnless(t1)
    self.failUnlessEqual( t1.Normalised, 0)
    self.failUnlessEqual( t1.name(), 'ToolSvc.tool1')
    self.failUnlessEqual( t1.type(), 'SequencerTimerTool')
    t2 = g.tool('ToolSvc.tool1')
    self.failUnless(t2)
    self.failUnlessEqual( t2.Normalised, 0)
    self.failUnlessEqual( t2.name(), 'ToolSvc.tool1')
    self.failUnlessEqual( t2.type(), 'SequencerTimerTool')

  def test10LocalTools(self) :
    g.DLLs   =  ['GaudiAlg']
    g.topAlg =  ['Prescaler']
    p = g.algorithm('Prescaler')
    self.failUnless(p._ialg)
    svc = g.toolsvc()
    t1 = svc.create('SequencerTimerTool','tool1',p._ialg)
    self.failUnless(t1)
    self.failUnlessEqual( t1.Normalised, False)
    self.failUnlessEqual( t1.name(), 'Prescaler.tool1')
    self.failUnlessEqual( t1.type(), 'SequencerTimerTool')
    t1.Normalised = False
    t2 = g.tool('Prescaler.tool1')
    self.failUnless(t2)
    self.failUnlessEqual( t2.Normalised, False)
    self.failUnlessEqual( t2.name(), 'Prescaler.tool1')
    self.failUnlessEqual( t2.type(), 'SequencerTimerTool')
    t3 = svc.create('SequencerTimerTool','subtool1',t1._ip)
    self.failUnless(t3)
    self.failUnlessEqual( t3.Normalised, False)
    self.failUnlessEqual( t3.name(), 'Prescaler.tool1.subtool1')
    self.failUnlessEqual( t3.type(), 'SequencerTimerTool')
    t4 = g.tool('Prescaler.tool1.subtool1')
    self.failUnless(t4)
    self.failUnlessEqual( t4.Normalised, False)
    self.failUnlessEqual( t4.name(), 'Prescaler.tool1.subtool1')
    self.failUnlessEqual( t4.type(), 'SequencerTimerTool')

  def test11ostreamcallback(self) :
    def test11do_msg(s) : self.got_it = True
    self.got_it = False
    buf  = GaudiPython.CallbackStreamBuf(test11do_msg)
    ostream = GaudiPython.gbl.basic_ostream('char','char_traits<char>')(buf)
    msgSvc = g.service('MessageSvc', GaudiPython.gbl.IMessageSvc)
    original = msgSvc.defaultStream()
    msgSvc.setDefaultStream(ostream)
    msgSvc.reportMessage('TEST',7,'This is a test message')
    msgSvc.setDefaultStream(original)
    self.failUnless(self.got_it)

  def test12pickle(self) :
    import pickle
    o = GaudiPython.gbl.TH1F('sss','aaa',10,0.,1.)
    self.failUnlessEqual(o.GetName(), pickle.loads(pickle.dumps(o)).GetName() )
    o = GaudiPython.gbl.std.vector('double')()
    o.push_back(10.)
    o.push_back(20.)
    self.failUnlessEqual([i for i in o], [i for i in pickle.loads(pickle.dumps(o))])


suite = unittest.makeSuite(BasicTests,'test')

if __name__ == '__main__':
  unittest.TextTestRunner(verbosity=2).run(suite)
  g.finalize()

