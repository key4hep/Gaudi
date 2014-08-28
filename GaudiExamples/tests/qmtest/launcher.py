# -*- coding: utf-8 -*-

from subprocess import *
import subprocess
import os

#-------------------------------------------------------------------------#

def main():

    print 'starting'
    import dependencyAnalysis as DA

    #dir = "./newFormat"
    dir = 'gaudiexamples.qms'
    fileList = sorted([ l for l in os.listdir(dir) if not l.startswith('.')])
    for l in fileList:
        n=fileList.index(l)
        l = dir+'/'+l
        fileList[n]=l

    testList = ''
    for l in fileList :
        testList+= l+" "

#     print DA.sniffer('/afs/cern.ch/user/v/valentin/workspace/Gaudi/GaudiExamples/tests/qmtest')\
    print fileList
    proc= Popen(['python', 'TestLauncher.py'] + fileList[:2])
    proc.wait()

    print 'end'

main()
