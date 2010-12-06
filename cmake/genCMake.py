"""
  genCMake is simple script that is able to generate the CMakeLists.tst file required for a CMT package.
  usage:
        ---for a single package----
     cd <mypackage>/cmt
     python genCMake

        ---for a complete project---
     cd <project>/XXXXSys/cmt
     cmt br "python genCMake > ../CMakeLists.txt"
"""

import os, pickle, copy

#---Useful global valiables-------------------------------------------------------------------------- 
package = os.getcwd().split(os.sep)[-2]
project = os.popen('cmt show macro_value project').readlines()[0].strip()
project_home = os.popen('cmt show macro_value %s_home' % project ).readlines()[0].strip()
picklefile = '/tmp/%s/%s_libraries.pkl'% (os.environ['USER'], '%s')
root = os.path.dirname(os.getcwd())+'/'

#---Useful functions---------------------------------------------------------------------------------
ignorelibs = []
def getlibs(library):
  libs = []
  linkopts = os.popen('cmt show macro_value %s_use_linkopts' % library).readlines()[0][:-1]
  for opt in linkopts.split():
    if opt[:2] == '-l':
      if opt[2:] not in ignorelibs and opt[2:] not in libs: libs.append(opt[2:])
  return libs

replacetable = [('boost_system', '${Boost_LIBRARIES}'),
                ('boost_filesystem', ''),
                ('boost_regex', '${Boost_regex_LIBRARY}'),
                ('boost_thread', ''),
                ('boost_program_options', ''),
                ('boost_date_time', '${Boost_date_time_LIBRARY}'),
                ('=Core', '${ROOT_LIBRARIES}'),
                ('=Cint', ''),
				('=Reflex', ''),
                ('=dl', ''),
                ('=Tree', '${ROOT_Tree_LIBRARY}'),
                ('=RIO', ' ${ROOT_RIO_LIBRARY}'),
                ('=Matrix','${ROOT_Matrix_LIBRARY}'),
                ('=MathCore', '${ROOT_MathCore_LIBRARY}'),
                ('=MathMore', '${ROOT_MathMore_LIBRARY}'),
                ('=GenVector', '${ROOT_GenVector_LIBRARY}'),
                ('=Hist', '${ROOT_Hist_LIBRARY}'),
                ('=util', ''),
                ('=xerces-c', '${XercesC_LIBRARIES}'),
                ('=uuid', '${uuid_LIBRARIES}'),
                ('=pthread', ''),
                ('=gslcblas', '${GSL_LIBRARIES}'),
                ('=gsl', ''),
                ('=CLHEP-Cast-1.9.4.4', '${CLHEP_LIBRARIES}'),
                ('CLHEP', ''),
                ('=python2.5', '${Python_LIBRARIES}'),
                ('=fftw3', '${fftw_LIBRARIES}'),
                ('=HepPDT', '${HepPDT_LIBRARIES}'),
                ('=HepPID', '')
                ]
                
def prettylibs(libraries):
  libs = []
  for lib in libraries:
    for p,v in replacetable:
      if p[0] == '=' :
        if p[1:] == lib :
          lib = v
          break
      else :
        if p in lib :
          lib = v
          break
    if lib: libs.append(lib)
  return ' '.join(libs)

#---Get the list of dependent proejcts---------------------------------------------------------------
projects = []
gbl_libraries = {}
loc_libraries = {}
for l in os.popen('cmt show projects').readlines():
  p = l.split()[0]
  if p not in projects : projects.append(p)
for p in projects:
  #---Read Global libraries ---------------------------------------------------------------------------
  if os.path.exists(picklefile % p):
    input = open(picklefile % p, 'rb')
    libs = pickle.load(input)
    gbl_libraries.update(libs)
    if p == project : loc_libraries.update(libs)
    input.close()

print """############################################################################
# CMakeLists.txt file for building %s
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################""" % package


#---Find dependent packages--------------------------------------------------------------------------
gaudipackages = ['GaudiPolicy', 'GaudiPython', 'GaudiPoolDb', 'RootHistCnv', 
                 'GaudiKernel', 'GaudiAlg', 'GaudiUtils', 'GaudiObjDesc',
                 'GaudiSvc']
#if (project == 'GAUDI' ):
#  outsidepackages = ['GaudiSvc']
#else :
#  outsidepackages = gaudipackages
outsidepackages = []
  
ignorepackages = ['GaudiPolicy', 'LCG_Interfaces/Reflex', 'LCG_Interfaces/oracle',
                  'LCG_Interfaces/lfc','LCG_Interfaces/sqlite', 'LCG_Interfaces/mysql',
                  'LCG_Interfaces/pyqt', 'Det/SQLDDDB', 'FieldMap']

dependentpackages = []
lines = os.popen('cmt show uses').readlines()
#---Find remote packages-----------------------------------------------------------------------------
for l in lines:
  if l[:4] == 'use ':
    items = l[4:].split()
    p = items[0]
    if items[2][0] != '(' :p = items[2]+'/'+p
    if items[2] == 'LCG_Interfaces' :
      if not 'no_auto_imports' in l and not p in ignorepackages:
        dependentpackages.append(p)
        continue
    if project_home in l : continue
    outsidepackages.append(p)   
    
for l in lines:
  if l[:5] == '# use':
    p = l[5:].split()[0]
    if  l[5:].split()[2][0] != '(': p = l[5:].split()[2]+'/'+p
    if p in outsidepackages :
      pass
    elif p in ignorepackages :
      pass
    elif p in dependentpackages :
      pass
    else :
      dependentpackages.append(p)

for p in dependentpackages:
  print 'GAUDI_USE_PACKAGE(%s)' % p.replace('LCG_Interfaces/','')

#---Include Dirs-------------------------------------------------------------------------------------
dirs = os.popen('cmt show include_dirs').readlines()[0]
for d in dirs.split():
  if root in d:
    print 'include_directories(${CMAKE_CURRENT_SOURCE_DIR}/%s)' % d.replace(root,'')

#---Find Constinuents--------------------------------------------------------------------------------

reflex_dictionaries = []
install_python = False
install_scripts = False
install_headers = []
install_joboptions = []
libraries = {}
executables = {}
genconfig = []
god_headers = []
god_dictionaries = []
god_customdicts = []

lines = os.popen('cmt show constituents').readlines()
for l in lines:
  t = l.split()
  if t[0] == 'document' :
    if t[1] == 'install_python':
      if os.path.exists('python') : install_python = True
    elif t[1] == 'install_scripts':
      if os.path.exists('scripts') : install_scripts = True
    elif t[1] == 'data_installer'and t[2] == 'install_joboptions':
      for f in t[3:] :
        if f.find('prefix=') != 0 : install_joboptions.append(f.replace('../',''))
    elif t[1] == 'reflex_dictionary_generator' :
      name =  t[2][:-3]
      sel = os.popen('cmt show macro_value %s_reflex_selection_file' % name).readlines()[0][:-1]
      if t[3][0] == '-' : hfile = t[4]
      else :              hfile = t[3]
      reflex_dictionaries.append([name, hfile.replace(root,'').replace('../',''), sel.replace(root,'').replace('../','') ])
    elif t[1] == 'genconfig' :
      genconfig.append(t[2][:-4])
    elif t[1] == 'install_more_includes':
      dir1 = t[3].replace('more=','')
      dir2 = t[4].replace('offset=','')
      dirs = []
      if dir1 and os.path.exists('../'+dir1) : dirs.append(dir1)
      if dir2 and os.path.exists('../'+dir2) : dirs.append(dir2)
      if dirs : install_headers.append(dirs)
    elif t[1] == 'obj2doth' :
      god_headers.append([t[3].replace('../','')])
    elif t[1] == 'obj2dict' :
      god_dictionaries.append([t[3].replace('../','')])
    elif t[1] == 'customdict' :
      god_customdicts.append([t[2], t[3].replace('../','')])
  elif t[0] == 'library' :
    sources = []
    test = False
    for s in t[2:]:
      if s[0] != '-' : sources.append(s)
      if s == '-group=tests' : test = True
    libraries[t[1]] = [sources, getlibs(t[1]), test]
  elif t[0] == 'application' :
    sources = []
    test = False
    for s in t[2:]:
      if s[0] != '-' : sources.append(s)
      if s == '-group=tests' : test = True
    executables[t[1]] = [sources, getlibs(t[1]), test]
  else :
    print 'ERROR ---- constituents unknown', t[0]

#---Clean libraries and executables dependent libraries---------------
for k, l in libraries.items():
  if k[-4:] == 'Dict' : continue
  if k not in genconfig : loc_libraries[k] = copy.deepcopy(l)

for i, iv in libraries.items() +  executables.items() :
 for j, jv in gbl_libraries.items() :
   if i is not j :
     if j in iv[1]:
       for k in jv[1]:
         if k in iv[1] : iv[1].remove(k)
         
if god_headers:
  print '\nINCLUDE(GaudiObjDesc)'
  print '\n#---GOD Headers-------------------------------------------------------------'
  dest = ''
  lines = os.popen('cmt show macro_value %sObj2Doth_GODflags' % package).readlines()
  if lines:
    option = lines[0].strip()
    if '-s' in option :
      dest = option[option.find('../')+3:]
      if dest[-1] == '/' : dest = dest[:-1]
for h in god_headers :
  if dest :
    print 'GOD_BUILD_HEADERS(%s %s)'%(h[0],dest)
  else : 
    print 'GOD_BUILD_HEADERS(%s)'%(h[0],)


if libraries:
  print '\n#---Libraries---------------------------------------------------------------'
for k,l in libraries.items():
  if k[-4:] == 'Dict' : continue
  if len(l[0]) > 1:
    sources = ' \n'.join([s.replace('../src/','')  for s in l[0]])  
    print 'SET( %s_srcs %s )' % (k, sources)
    srcs = '${%s_srcs}' % k
  else:
    srcs = l[0][0].replace('../src/','')
  if l[2] : print 'if(BUILD_TESTS)\n  ',
  if k in genconfig :
    print 'GAUDI_COMPONENT_LIBRARY(%s %s LIBRARIES %s)'%(k, srcs, prettylibs(l[1]))
  else :
    print 'GAUDI_LINKER_LIBRARY(%s %s LIBRARIES %s)'%(k, srcs, prettylibs(l[1]))
  if l[2] : print 'endif()'

if executables:
  print '\n#---Executables-------------------------------------------------------------'
for k,l in executables.items():
  if len(l[0]) > 1:
    sources = ' \n'.join([s.replace('../src/','')  for s in l[0]])  
    print 'SET( %s_srcs %s )' % (k, sources)
    srcs = '${%s_srcs}' % k
  else:
    srcs = l[0][0].replace('../src/','')
  if l[2] :
    print 'GAUDI_UNIT_TEST(%s %s LIBRARIES %s)'%(k, srcs, prettylibs(l[1]))
  else :
    print 'GAUDI_EXECUTABLE(%s %s LIBRARIES %s)'%(k, srcs, prettylibs(l[1]))


if god_customdicts or god_dictionaries:
  print '\n#---GOD Dictionary----------------------------------------------------------'
  dest = ''
  lines = os.popen('cmt show macro_value %sObj2Dict_GODflags' % package).readlines()
  if lines:
    option = lines[0].strip()
    if '-s' in option :
      dest = option[option.find('../')+3:]
      if dest[-1] == '/' : dest = dest[:-1]
for d in  god_customdicts :
  print 'GOD_CUSTOM_DICTIONARY(%s %s)'%(d[1], 'xml/lcgdict/lcg_selection.xml')
for d in  god_dictionaries :
  if dest :
    print 'GOD_BUILD_DICTIONARY(%s %s)'%(d[0], dest)
  else :
    print 'GOD_BUILD_DICTIONARY(%s)'%(d[0],)
  libs = []
  for i in libraries[package+'Dict'][1]:
    if i != package : libs.append(i)
  if libs :
    print 'target_link_libraries(%s %s)'%(package+'Dict',prettylibs(libs))
      
    
if not god_dictionaries and reflex_dictionaries:
  print '\n#---Dictionaries------------------------------------------------------------'
  for d in reflex_dictionaries:
    libs = ''
    for k,l in libraries.items() :
      if k == d[0]+'Dict' : libs = prettylibs(l[1]) 
    print 'REFLEX_BUILD_DICTIONARY(%s %s %s LIBRARIES %s)'% (d[0],d[1],d[2], libs)
  
print '\n#---Installation------------------------------------------------------------'

for lst in install_headers:
  print 'GAUDI_INSTALL_HEADERS(%s)' % ' '.join(lst)

if install_python:
  print 'GAUDI_INSTALL_PYTHON_MODULES()'

if install_scripts:
  print 'GAUDI_INSTALL_SCRIPTS()'

if install_joboptions:
  print 'GAUDI_INSTALL_JOBOPTIONS(%s)' % ' '.join(install_joboptions)


output = open(picklefile % project, 'wb')
pickle.dump(loc_libraries, output)
output.close()


