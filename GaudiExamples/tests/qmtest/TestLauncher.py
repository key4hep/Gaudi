# -*-coding:utf8 -*


import os
import xml.etree.ElementTree as ET
import sys
import xml.sax.saxutils as XSS
import BaseTest as GT
import QMTTest as QT
import re
import time


#-------------------------------------------------------------------------#


# XMLwriter parse the results of the tests in a single xml file
# The 3 elements Name, Status and Measurment must not be checked during the NamedMeasurment generation loop, that's why they are deleted
# The keys and text are formated to be html safe
def XMLwriter(resultDic, fileName):
    if resultDic is not None:

        #Creating the xml tree
        try :
            tree = ET.parse(fileName)
        except :
            cleanXml(fileName)
            tree = ET.parse(fileName)
        root = tree.getroot()

        #Test is the root
        Test = ET.Element('Test')
        Test.set('Status',resultDic['Status'])
        del resultDic['Status']
        Name = ET.SubElement(Test,'Name')
        Name.text= XSS.escape(resultDic['Name'][:-4])
        del resultDic['Name']

        # Branch containing all the measurments
        Results = ET.SubElement(Test,'Results')

        NamedMeasurement = dict()
        Value = dict()

        # Storing the measurments in dictionnaries, if not a String type, add an "if" condition
        for key in resultDic :
            if resultDic[key] !='' and key!='Measurement':
                NamedMeasurement[key] = ET.SubElement(Results,'NamedMeasurement')
                NamedMeasurement[key].set('name',key)

                # Setting type
                if key=="Execution Time" : NamedMeasurement[key].set('type',"numeric/float")
                if key=="exit_code" : NamedMeasurement[key].set('type',"numeric/integer")
                else : NamedMeasurement[key].set('type',"String")

                # Setting value
                Value[key] = ET.SubElement(NamedMeasurement[key],'Value')
                if key=='Environment' :
                    env = resultDic[key]
                    Value[key].text = XSS.escape('\n'.join('{0}={1}'.format(k, env[k]) for k in sorted(env)))
                elif key=='Causes':
                    Value[key].text = XSS.escape('\n'.join(resultDic[key]))
                elif key=='Validator results':
                    valres= resultDic[key]
                    Value[key].text = XSS.escape('\n'.join('{0}={1}'.format(k, valres[k]) for k in valres)).encode("ascii", "xmlcharrefreplace")
                elif key=='Unsupported platforms':
                    Value[key].text = XSS.escape('\n'.join(resultDic[key]))
                else :
                    Value[key].text = XSS.escape(str(resultDic[key]))



        Measurement = ET.SubElement(Results,'Measurement')
        Value['Measurement'] = ET.SubElement(Measurement,'Value')
        Value['Measurement'].text = XSS.escape(resultDic['Measurement'])



        root.append(Test)
        tree.write(fileName,encoding='utf-8')


#----------------------------------------------------------------------------------------#

def cleanXml(xmlFileName):
        """
        Removes xml illegal characters from a file.
        @param xmlFileName: The name of the xml file.
        """
        _illegal_xml_chars_Re = re.compile(u'[\x00-\x08\x0b\x0c\x0e-\x1F\uD800-\uDFFF\uFFFE\uFFFF]')
        xmlFile = open(xmlFileName,'r')
        data = xmlFile.read()
        xmlFile.close()
        xmlFile = open(xmlFileName,'w')
        xmlFile.write(_illegal_xml_chars_Re.sub("", data))
        xmlFile.close()

#-------------------------------------------------------------------------#

def main(fileList):
    if not os.path.exists("./results"):
        os.makedirs("./results")
    DOB = time.localtime()
    dateOfBegining = str(DOB[1])+"-"+str(DOB[2])+"-"+str(DOB[0])+"_"+str(DOB[3])+":"+str(DOB[4])
    # Preparing the result file
    nameResult = "./results/results_"+dateOfBegining+"_"+str(len(fileList))+".xml"
    file = open(nameResult,"w+")
    file.close()
    Doc = ET.Element('Doc')
    tree = ET.ElementTree(Doc)
    tree.write(nameResult)



    #fileList=["/afs/cern.ch/user/v/valentin/workspace/Gaudi/GaudiExamples/tests/qmtest/gaudiexamples.qms/event_timeout_abort.qmt"]

    #fileList = ['./gaudiexamples.qms/root_io.qms/read.qmt']

    # Testing the file begining with "Test" or if it is a qmt file and doing the test
    for file in fileList :
        if file.endswith('_test.py') :
            indexFilePart= file.rfind("/")
            fileToImport = file[indexFilePart+1:]
            sys.path.append(GT.RationalizePath(file)[:-len(fileToImport)-1])
            imp = __import__(fileToImport[:-3])
            fileToExec = imp.Test()
            XMLwriter(fileToExec.runTest(),nameResult)
        if file.endswith(".qmt"):
            fileToTest = QT.QMTTest()
            fileToTest.XMLParser(file)
            XMLwriter(fileToTest.runTest(),nameResult)
    cleanXml(nameResult)

main(sys.argv)
