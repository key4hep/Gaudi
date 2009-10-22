""" 
  This script allows you to run Gaudi Applications from python and use
"""

#--- CMT interface

import os, sys, string
import atexit
import warnings

#--- Module variables
cmtpath = os.getenv('CMTPATH').split(os.pathsep)
cmtroot = os.getenv('CMTROOT')
cmtbin  = os.getenv('CMTBIN')
cmtcmd  = 'cmt'
if cmtbin and cmtroot :
    cmtcmd = cmtroot + os.sep + cmtbin + os.sep + 'cmt'

warnings.filterwarnings('ignore', 'tempnam', RuntimeWarning)
tmppath   = os.tempnam()
tmpcmtdir = tmppath+'/cmttemp/v1/cmt/'
uses    = []
environ = {}

#-----------------------------------------------------------------------------
def cleanup() :
#-----------------------------------------------------------------------------
    if os.path.exists(tmpcmtdir+'requirements') :
        #os.remove( tmpcmtdir+'requirements')
        pass
    #os.removedirs( tmpcmtdir )
    pass
    
os.makedirs(tmpcmtdir)
atexit.register(cleanup)

#-----------------------------------------------------------------------------
def use( package, version='*',path='') :
#-----------------------------------------------------------------------------
    elements = package.split('/')
    if len(elements) == 1 :
        pac = elements[0]
        pat = path
    else :
        pac = elements[-1]
        if( path != '' ) :
            pat = path+'/'+'/'.join(elements[:-1])
        else :
            pat = '/'.join(elements[:-1])
    uses.append((pac,version,pat))

#-----------------------------------------------------------------------------
def setup():
#-----------------------------------------------------------------------------
    global cmtcmd
    reqfile = open(tmpcmtdir+'requirements','w')
    for use in uses :
        reqfile.write('use '+use[0]+' '+use[1]+' '+use[2]+'\n')
    reqfile.close()
    # execute cmt setup
    savdir = os.getcwd()
    os.chdir(tmppath+'/cmttemp/v1/cmt')
    command = cmtcmd + ' show sets' 
    for line in os.popen(command).readlines():
        env = line[:line.find('=')]
        val = line[line.find('=')+2:-2]
        if env :
            environ[env] = val
    os.chdir(savdir)
    for key in environ.keys() :
        # replace all occurences of %...% by their value
        while environ[key].find('%') != -1 :
            value = environ[key]
            var = value[value.find('%')+1:value.find('%',value.find('%')+1)]
            if environ.has_key(var) :
                val = environ[var]
            else :
                val = os.getenv(var)
                if not val : val = ''
            environ[key] = value.replace('%'+var+'%',val)
        #replace all occurences of ${...} by their value
        while environ[key].find('${') != -1 :
            value = environ[key]
            var = value[value.find('${')+2:value.find('}',value.find('${')+2)]
            if environ.has_key(var) :
                val = environ[var]
            else :
                val = os.getenv(var)
                if not val : val = ''
            environ[key] = value.replace('${'+var+'}',val)
        # set the environment
        os.environ[key]=environ[key]
    if 'PYTHONPATH' in environ.keys() :
        sys.path += environ['PYTHONPATH'].split(os.pathsep)
    # re-calculate basic variables
    # this only works if the variables have been set through os.environ
    # and *not* with os.putenv
    cmtpath = os.getenv('CMTPATH').split(os.pathsep)
    cmtroot = os.getenv('CMTROOT')
    cmtbin  = os.getenv('CMTBIN')
    if cmtbin and cmtroot : cmtcmd = cmtroot + os.sep + cmtbin + os.sep + 'cmt'


#-----------------------------------------------------------------------------
def showuses() :
#-----------------------------------------------------------------------------
    reqfile = open(tmpcmtdir+'requirements','w')
    for use in uses :
        reqfile.write('use '+use[0]+' '+use[1]+' '+use[2]+'\n')
    reqfile.close()
    command = cmtcmd + ' show uses -pack=cmttemp -version=v1 -path='+ tmppath
    usedict = {}
    for line in os.popen(command).readlines():
        if( line[0] != '#' ) :
            elem =  string.split(line[:-1])
            if len(elem) <= 3 : continue
            if elem[1] == 'CMT' : continue
            if elem[3][0] != '(' :
                pack = elem[3]+'/'+elem[1]
                path = elem[4]
            else :
                pack = elem[1]
                path = elem[3]
            usedict[pack] = (elem[2],path)
    packs = usedict.keys()
    packs.sort()
    for p in packs :
        print '%-20s %-10s %s' % ( p, usedict[p][0], usedict[p][1] )
 
    
            

        

