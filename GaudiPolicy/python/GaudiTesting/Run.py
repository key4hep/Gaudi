# -*- coding: utf-8 -*-


import os
import xml.etree.ElementTree as ET
import sys
import xml.sax.saxutils as XSS
import BaseTest as GT
import re
import time
import logging


#-------------------------------------------------------------------------#


# XMLwriter parse the results of the tests in a single xml file
# The 3 elements Name, Status and Measurment must not be checked during the NamedMeasurment generation loop, that's why they are deleted
# The keys and text are formated to be html safe
def XMLwriter(resultDic, fileName):
    if resultDic is not None:

        if not os.path.exists(fileName):
            # create the container directory, if needed
            if not os.path.exists(os.path.dirname(fileName)):
                os.makedirs(os.path.dirname(fileName))
            # create initial XML file
            site_el = ET.Element('Site', siteAttrib())
            testing_el = ET.SubElement(site_el, 'Testing')
            start_time = int(time.time())
            time_str = time.strftime("%b %d %H:%M %Z",
                                     time.localtime(start_time))
            ET.SubElement(testing_el, 'StartTestTime').text = str(start_time)
            ET.SubElement(testing_el, 'StartDateTime').text = time_str
            tree = ET.ElementTree(site_el)

        else:
            # use the existing XML file
            try:
                tree = ET.parse(fileName)
            except:
                cleanXml(fileName)
                tree = ET.parse(fileName)

        #root = tree.getroot()
        root = tree.find('Testing')

        #Test is the root
        Test = ET.Element('Test')
        Test.set('Status',resultDic['Status'])
        del resultDic['Status']
        Name = ET.SubElement(Test,'Name')
        test_file_name = resultDic['Name']
        test_name = test_file_name.replace('.qmt', '').replace('.qms' + os.path.sep, '.')
        Name.text= XSS.escape(test_name)
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

        # replace the previous results of the test we are adding
        for t in [t for t in root.findall('Test[Name]')
                  if t.find('Name').text == test_name]:
            root.remove(t)
        root.append(Test)

        end_time = time.time()

        EndTestTime = root.find('EndTestTime')
        if EndTestTime is not None:
            root.remove(EndTestTime)
        ET.SubElement(root, 'EndTestTime').text = str(int(end_time))

        EndDateTime = root.find('EndDateTime')
        if EndDateTime is not None:
            root.remove(EndDateTime)
        ET.SubElement(root, 'EndDateTime').text = time.strftime("%b %d %H:%M %Z", time.localtime(end_time))

        tree.write(fileName, encoding='utf-8')


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
def siteAttrib():
    import socket
    #keys = ["Generator", "Hostname", "BuildName",
    #        "OSName", "Name", "OSRelease", "OSVersion", "OSPlatform"]
    #values = ([__file__, socket.gethostname(), os.getenv("CMTCONFIG")] +
    #          os.uname())
    #return dict(zip(keys, values))
    return {
            "Generator":  __file__,
            "Hostname":   socket.gethostname(),
            "BuildName":  os.getenv("CMTCONFIG"),
            "OSName":     os.uname()[0],
            "Name":       os.uname()[1],
            "OSRelease":  os.uname()[2],
            "OSVersion":  os.uname()[3],
            "OSPlatform": os.uname()[4],
            }

def main():
    from optparse import OptionParser, OptionGroup
    parser = OptionParser()

    parser.add_option('--output',
                      help='name of the output file [default: %default]')


    verbosity_opts = OptionGroup(parser, 'Verbosity Level',
                                 'set the verbosity level of messages')
    verbosity_opts.add_option('--silent',
                      action='store_const', dest='log_level',
                      const=logging.CRITICAL,
                      help='only critical error messages')
    verbosity_opts.add_option('--quiet',
                      action='store_const', dest='log_level',
                      const=logging.ERROR,
                      help='error messages')
    verbosity_opts.add_option('--warning',
                      action='store_const', dest='log_level',
                      const=logging.WARNING,
                      help='warning and error messages')
    verbosity_opts.add_option('--verbose',
                      action='store_const', dest='log_level',
                      const=logging.INFO,
                      help='progress information messages')
    verbosity_opts.add_option('--debug',
                      action='store_const', dest='log_level',
                      const=logging.DEBUG,
                      help='debugging messages')
    parser.add_option_group(verbosity_opts)


    parser.set_defaults(log_level=logging.WARNING,
                        output='Results_Test.xml')


    opts, fileList = parser.parse_args()

    logging.basicConfig(level=opts.log_level)


    #fileList=["/afs/cern.ch/user/v/valentin/workspace/Gaudi/GaudiExamples/tests/qmtest/gaudiexamples.qms/event_timeout_abort.qmt"]

    #fileList = ['./gaudiexamples.qms/root_io.qms/read.qmt']

    # Testing the file begining with "Test" or if it is a qmt file and doing the test
    for file in fileList :
        logging.debug('processing %s', file)
        if file.endswith('_test.py') :
            indexFilePart= file.rfind("/")
            fileToImport = file[indexFilePart+1:]
            sys.path.append(GT.RationalizePath(file)[:-len(fileToImport)-1])
            imp = __import__(fileToImport[:-3])
            fileToExec = imp.Test()
            XMLwriter(fileToExec.run(), opts.output)
        if file.endswith(".qmt"):
            from QMTTest import QMTTest
            fileToTest = QMTTest(file)
            XMLwriter(fileToTest.run(), opts.output)
    cleanXml(opts.output)


if __name__ == '__main__':
    main()
