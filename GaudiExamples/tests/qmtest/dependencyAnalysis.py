# -*- coding: utf-8 -*-
import os
import xml.etree.ElementTree as ET
import sys


def QMTsniffer(path) :

    tree = ET.parse(RationalizePath(path))
    root = tree.getroot()
    finalList=[]
    for child in root:
        type = child.attrib['name']
        if type == 'prerequisites' :
            tupleList = child[0].findall('tuple')
            textList= [l.findall("text") for l in tupleList]
            for tl in textList :
                finalList.append(tl[0].text)
    return finalList

def QMSsniffer(path):
    tree = ET.parse(RationalizePath(path))
    root = tree.getroot()
    finalList=[]
    for child in root:
        type = child.attrib['name']
        if type == 'suite_ids' or type=='test_ids':
            setList = child[0].findall('set')
            textList = [l.findall('text') for l in tupleList]
            for tl in texList:
                finalList.append(tl[0].text)
    return finalList

def RationalizePath(p) :

    newPath = os.path.normpath(os.path.expandvars(p))
    if os.path.exists(newPath) :
        p = os.path.realpath(newPath)
    p = os.path.realpath(newPath)
    return p


def sniffer(dir,originDirectory='',package=''):
    dir = RationalizePath(dir)
    os.chdir(dir)
    finalList = dict()
    newPackage=package
    fileList = [ l for l in os.listdir(dir) if not l.startswith('.')]
    if dir.endswith(".qms"):
        t,p=os.path.split(dir)
        newPackage+=p+"/"
    for file in fileList:
        if file.endswith(".qmt"):
            finalList[newPackage+os.path.basename(file)]=QMTsniffer(file)
        elif file.endswith(".qms") and os.path.isfile(file):
            finalList[newPackage+os.path.basename(file)]=QMSsniffer(file)
        elif file.endswith(".qms"):
            file=RationalizePath(file)
            newResult=sniffer(file,dir,package=newPackage)
            finalList=dict(finalList.items()+newResult.items())
            finalList[newPackage+os.path.basename(file)]=[ ld for ld in os.listdir(file) if (not l.startswith('.') and (l.endswith('.qmt') or not l.endswith('.qms')))]
        elif os.path.isdir(file):
            file=RationalizePath(file)
            newResult=sniffer(file,dir,package=newPackage)
            finalList=dict(finalList.items()+newResult.items())
    if originDirectory!='' :
        os.chdir(originDirectory)
    return finalList
