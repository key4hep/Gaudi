hdr = """<?xml version="1.0" ?><!DOCTYPE extension  PUBLIC '-//QM/2.3/Extension//EN'  'http://www.codesourcery.com/qm/dtds/2.3/-//qm/2.3/extension//en.dtd'>
<extension class="GaudiTest.GaudiExeTest" kind="test">
"""
trlr = "</extension>\n"

def write_test_files(tests,deps,template,name_mapper = lambda x: x.lower().replace('.','_')):
    for n in tests:
        f = open(os.path.join("gaudiexamples.qms",name_mapper(n)+".qmt"),"w")
        nn = n
        if nn.endswith(".py"):
            nn = nn[:-3]
        f.write(hdr + template%{"name":nn})
        if n in deps:
            f.write('  <argument name="prerequisites"><set>\n')
            for d in deps[n]:
                f.write('    <tuple><text>gaudiexamples.%s</text><enumeral>PASS</enumeral></tuple>\n'%name_mapper(d))
                f.write('  </set></argument>\n')
        f.write(trlr)

################################################################################
examples = ['Properties','AlgSequencer','AlgTools',\
            'IncidentSvc', \
            'RandomNumber','Histograms', 'Aida2Root' ,'NTuples',
            'DataOnDemand', \
            'POOLIOWrite', 'POOLIORead', \
            'POOLIOCollWrite', 'POOLIOCollRead', \
            'POOLIOExtCollWrite', 'POOLIOExtCollRead', \
#            'AIDATupleWrite','AIDATupleRead', \
            'CounterEx', 'CounterSvcAlg', 'StatSvcAlg', \
            'TupleEx', \
            'EvtColsEx_Prepare','EvtColsEx_Write','EvtColsEx_Read', \
            'History','ErrorLog','THistWrite','THistRead', \
            'AlgErrAud',\
            'FastContainers', 'Maps', 'QotdAlg', \
            'GaudiCommonTests' ]

examples_deps = { 'POOLIORead' : [ 'POOLIOWrite' ],
                  'POOLIOCollWrite' : [ 'POOLIOWrite' ],
                  'POOLIOCollRead' : [ 'POOLIOCollWrite' ],
                  'POOLIOExtCollWrite' : [ 'POOLIOWrite' ],
                  'POOLIOExtCollRead' : [ 'POOLIOExtCollWrite' ],
                  'EvtColsEx_Read' : ['EvtColsEx_Write' ],
                  'EvtColsEx_Write' : [ 'EvtColsEx_Prepare' ],
                  'THistRead' : [ 'THistWrite' ],
                  }

ex_tmpl = """  <argument name="args"><set><text>../options/%(name)s.opts</text></set></argument>
  <argument name="workdir"><text>../../home</text></argument>
  <argument name="reference"><text>%(name)s.ref</text></argument>
"""  

################################################################################
no_win_examples = ['GaudiMT',
                   'ColorMsg' ,
                   'DataListener']

no_win_examples_deps= {}

no_win_tmpl = ex_tmpl + '  <argument name="unsupported_platforms"><set><text>win32</text></set></argument>\n'

################################################################################
pyexamples = [ 'CounterEx.py',\
               'EvtColWrite','EvtColRead',\
               'HistoEx','TupleEx.py',\
               'HistoEx1','HistoEx2',
               'TupleEx1','TupleEx2','TupleEx3','TupleEx4' ]

pyexamples_deps = { 'EvtColRead' : [ 'EvtColWrite' ],
                    'EvtColWrite' : [ 'EvtColsEx_Prepare' ] }

pyex_tmpl = """  <argument name="program"><text>../python/%(name)s.py</text></argument>
  <argument name="workdir"><text>../../home</text></argument>
  <argument name="reference"><text>%(name)s.pyref</text></argument>
"""

################################################################################

write_test_files(examples,examples_deps,ex_tmpl)
write_test_files(no_win_examples,no_win_examples_deps,no_win_tmpl)
write_test_files(pyexamples,pyexamples_deps,pyex_tmpl)
