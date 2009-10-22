#!/usr/bin/env python

""" 
  This is the script to run all the examples
"""
import cmt, cdiff
import os, sys, string, getopt, popen2

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

# if sys.platform != 'win32' : examples += ['GaudiMT',
#                                          'ColorMsg' ,
#                                          'DataListener']

if sys.platform != 'win32' : examples += ['GaudiMT',
                                          'ColorMsg' ,
                                          'DataListener']


pyexamples = [ 'CounterEx',\
               'EvtColWrite','EvtColRead',\
               'HistoEx','TupleEx',\
               'HistoEx1','HistoEx2',
               'TupleEx1','TupleEx2','TupleEx3','TupleEx4' ]#,\
               # 'Aida2RootEx']

def usage(argv) :
  print """
  %s [--valgrind] [--makeref] [--diff] [--pyonly] [--exeonly]
  """%os.path.basename(argv[0])
  
  sys.exit(0)
def main() :
  valgrind, makeref, diff = 0, 0, 1
  pyonly, exeonly = 0, 0
  #--- Parse command options
  try:
    opts, args = getopt.getopt(sys.argv[1:], '', ('valgrind','makeref','diff','pyonly','exeonly'))
  except getopt.GetoptError:
    usage(sys.argv)
    sys.exit(2)
  for o, a in opts:
    if o in ('--valgrind',): valgrind = 1
    if o in ('--makeref',): makeref = 1
    if o in ('--notdiff',)   : diff = 0
    if o in ('--pyonly'): pyonly = 1
    if o in ('--exeonly'): exeonly = 1
  #--- Supersedes standard examples if more args are found on the command line.
  global examples
  global pyexamples
  if args:
    opt_examples = []
    opt_pyexamples = []
    for a in args:
      if (a.find(".exe")!=-1):
        str_a = a[:-4]
        for ex in examples:
          if (ex == str_a):
            opt_examples.append(str_a)
      elif (a.find(".py")!=-1):
        str_a = a[:-3]
        for ex in pyexamples:
          if (ex == str_a):
            opt_pyexamples.append(str_a)
      else:
        str_a = a[:]  
        for ex in examples:
          if (ex == str_a):
            opt_examples.append(str_a)
        for ex in pyexamples:
          if (ex == str_a):
            opt_pyexamples.append(str_a)
      examples = []
      pyexamples = []
      if opt_examples : examples = opt_examples
      if opt_pyexamples : pyexamples = opt_pyexamples
  #--- Setup CMT the environment
  cmt.use('GaudiExamples')
  cmt.showuses()
  cmt.setup()
  os.chdir('../home')
  #--- Run the examples in a loop
  if not pyonly :
    for ex in examples :
      exex = os.environ["GAUDIEXE"]
      exex = string.replace(exex, '/',os.sep)
      if   os.path.exists(exex) :
        if sys.platform == 'win32' : refname = ex+'.winref'
        else :                       refname = ex+'.ref'
        outname = ex+'.out'
        if makeref : out = open(refname, 'w')
        else :       out = open(outname, 'w')
        if   os.path.exists('../options/%s.opts' % ex): opts = '../options/%s.opts' % ex
        elif os.path.exists('../options/%s.py' % ex)  : opts = '../options/%s.py' % ex
        else : opts = ''
        cmd = '%s %s' % (exex, opts)
        cmd = string.replace(cmd, '/',os.sep)
        if (valgrind) :
          cmd = 'valgrind --tool=memcheck --leak-check=yes --num-callers=20 ' + cmd 
        print '-----Running %12s [%60s]' % (ex,cmd),
        (now, bla) = popen2.popen4(cmd)
        for line in now.readlines():
#          if (valgrind and string.find(line,'==')>0) : print line
          out.write(line)
        status = out.close()
        if not status : print 'OK'
        if makeref : 
          print '-----New reference %s created' % refname
        elif valgrind :
          print '-----Valgrind output is in ', out.name
        elif diff :
          if (ex!="GaudiMT"):
            if os.path.exists(refname) :
              print '-----Comparing %s with %s' %(outname,refname),
              cdiff.pdiff(outname,refname)
              if not cdiff.status() : print 'OK'
              else                  :
                print 'FAIL\n' + cdiff.output()
            else :
              print '-----Reference output not available for %s' % ex
      else :
        print 'FAIL\n could not find example %s' % exex 
  #--- Run the python examples in a loop
  if not exeonly :
    for ex in pyexamples :
      exex = '../python/%s.py' % ex
      exex = string.replace(exex, '/',os.sep)
      if   os.path.exists(exex) :
        if sys.platform == 'win32' : refname = ex+'.pywinref'
        else :                       refname = ex+'.pyref'
        outname = ex+'.pyout'
        if makeref : out = open(refname, 'w')
        else :       out = open(outname, 'w')
        cmd = 'python %s' % exex
        cmd = string.replace(cmd, '/',os.sep)
        if (valgrind) :
          cmd = 'valgrind --tool=memcheck --leak-check=yes --num-callers=20 ' + cmd 
        print '-----Running %12s [%60s]' % (ex,cmd),
        (now, bla) = popen2.popen4(cmd)
        for line in now.readlines():
          out.write(line)
        status = out.close()
        if not status : print 'OK'
        if makeref : 
          print '-----New reference %s created' % refname
        elif valgrind :
          print '-----Valgrind output is in ', out.name
        elif diff :
          if os.path.exists(refname) :
            print '-----Comparing %s with %s' %(outname,refname),
            cdiff.pdiff(outname,refname)
            if not cdiff.status() : print 'OK'
            else                  :
              print 'FAIL\n' + cdiff.output()
          else :
            print '-----Reference output not available for %s' % ex
      else :
        print 'FAIL\n could not find example %s' % exex 
#---------------------------------------------------------------------
if __name__ == "__main__": main()

