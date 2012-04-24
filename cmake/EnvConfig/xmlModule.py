'''
Created on Jul 2, 2011

@author: mplajner
'''

from xml.dom import minidom
import Variable
import logging.config
import os

class XMLFile():
    '''Takes care of XML file operations such as reading and writing.'''

    def __init__(self):
        self.xmlResult = '<?xml version="1.0" encoding="UTF-8"?><env:config xmlns:env="EnvSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="EnvSchema ./EnvSchema.xsd ">\n'
        self.declaredVars = []
        logConf = os.path.normpath(os.path.dirname(os.path.realpath(__file__)) + '/log.conf')
        if not logging.getLogger('envLogger').handlers and os.path.exists(logConf):
            logging.config.fileConfig(logConf)
        self.logger = logging.getLogger('envLogger')


    def variable(self, path, namespace='EnvSchema' ,name=''):
        '''Returns list containing name of variable, action and value

        If no name given, returns list of lists of all variables and locals(instead of action 'local' is filled).
        '''
        if not os.path.isfile(path):
            raise IOError('No such file.')
        '''Get file'''
        self.doc = minidom.parse(path)
        if namespace == '':
            namespace = None

        '''Get all variables'''
        nodes = self.doc.getElementsByTagNameNS(namespace, "config")[0].childNodes
        variables = []
        nodeNum = 0
        for node in nodes:
            '''if it is an element node'''
            if node.nodeType == 1:
                nodeNum += 1
                if name != '':
                    if node.getAttribute('variable') != name:
                        continue

                if node.localName == 'declare':
                    variables.append([node.getAttribute('variable'), 'declare', node.getAttribute('type'), node.getAttribute('local'), nodeNum])
                else:
                    if len(node.childNodes) > 0:
                        value = node.childNodes[0].data
                    else:
                        value = ''
                    variables.append([node.getAttribute('variable'), node.localName, value, '' , nodeNum])

        return variables


    def resetWriter(self):
        '''resets the buffer of writer'''
        self.xmlResult = '<?xml version="1.0" encoding="UTF-8"?><env:config xmlns:env="EnvSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="EnvSchema ./EnvSchema.xsd ">\n'
        self.declaredVars = []

    def writeToFile(self, outputFile = ''):
        '''Finishes the XML input and writes XML to file.'''
        if(outputFile == ''):
            raise IOError("No output file given")
        self.xmlResult += '</env:config>'

        doc = minidom.parseString(self.xmlResult)
        with open(outputFile, "w") as f:
            f.write( doc.toxml() )

        f.close()
        return outputFile

    def writeVar(self, varName, action, value, type='list', local='false'):
        '''Writes a action to a file. Declare undeclared elements (not local List is default type).'''
        if action == 'declare':
            self.xmlResult += '<env:declare variable="'+varName+'" type="'+ type.lower() +'" local="'+(str(local)).lower()+'" />\n'
            self.declaredVars.append(varName)
            return

        if varName not in self.declaredVars:
            self.xmlResult += '<env:declare variable="'+varName+'" type="'+ type +'" local="'+(str(local)).lower()+'" />\n'
            self.declaredVars.append(varName)
        self.xmlResult += '<env:'+action+' variable="'+ varName +'">'+value+'</env:'+action+'>\n'


class Report():
    '''This class is used to catch errors and warnings from XML file processing to allow better managing and testing.'''

    '''Sequence of levels: warn - warning - info - error'''
    def __init__(self, level = 1, reportOutput = False):
        self.errors = []
        self.warns = []
        self.info = []
        self.warnings = []
        self.level = level

        if not reportOutput:
            self.reportOutput = False
        else:
            self.reportOutput = open(reportOutput, 'w')

        logConf = os.path.normpath(os.path.dirname(os.path.realpath(__file__)) + '/log.conf')
        if not logging.getLogger('envLogger').handlers and os.path.exists(logConf):
            logging.config.fileConfig(logConf)
        self.logger = logging.getLogger('envLogger')

    def addError(self, message, varName = '', action = '', varValue = '', procedure = ''):
        error = [message, varName, action, varValue, procedure]
        if self.level < 4:
            if not self.reportOutput:
                print 'Error: ' + error[0]
            else:
                self.reportOutput.write('Error: ' + error[0] + '\n')
        self.errors.append(error)
        self.logger.error(message)

    def addWarn(self, message, varName = '', action = '', varValue = '', procedure = ''):
        error = [message, varName, action, varValue, procedure]
        if self.level < 1:
            if not self.reportOutput:
                print 'Warn: ' + error[0]
            else:
                self.reportOutput.write('Warn: ' + error[0] + '\n')
        self.warns.append(error)
        self.logger.warn(message)

    def addWarning(self, message, varName = '', action = '', varValue = '', procedure = ''):
        error = [message, varName, action, varValue, procedure]
        if self.level < 2:
            if not self.reportOutput:
                print 'Warning: ' + error[0]
            else:
                self.reportOutput.write('Warning: ' + error[0] + '\n')
        self.warnings.append(error)
        self.logger.warning(message)

    def addInfo(self, message, varName = '', action = '', varValue = '', procedure = ''):
        error = [message, varName, action, varValue, procedure]
        if self.level < 3:
            if not self.reportOutput:
                print 'Info: ' + error[0]
            else:
                self.reportOutput.write('Info: ' + error[0] + '\n')
        self.warnings.append(error)
        self.logger.info(message)

    def clear(self):
        self.errors = []
        self.warns = []
        self.info = []
        self.warnings = []

    def closeFile(self):
        if self.reportOutput:
            self.reportOutput.close()

    def numErrors(self):
        return len(self.errors)

    def numWarnings(self):
        return len(self.warns) + len(self.warnings)

    def error(self, key):
        return self.errors[key]

    def warn(self, key):
        return self.warns[key]


class XMLOperations():
    '''This class is for checking and merging XML files.

    Variables are stored in a double dictionary with keys of names and then actions.
    '''
    def __init__(self, separator=':', reportLevel = 0, reportOutput = False):
        self.posActions = ['append','prepend','set','unset', 'remove', 'remove-regexp', 'declare']
        self.separator = separator
        self.report = Report(reportLevel, reportOutput = reportOutput)

    def errors(self):
        return self.report.numErrors()

    def warnings(self):
        return self.report.numWarnings()

    def check(self, xmlFile):
        '''Runs a check through file

        First check is made on wrong action parameter.
        All valid actions are checked after and duplicated variables as well.
        '''
        #self.local = Variable.Local()
        self.varNames = []
        self.realVariables = {}
        self.variables = {}

        '''load variables and resolve references to locals and then variables'''
        self._loadVariables(xmlFile)

        '''report'''
        if (self.warnings() > 0 or self.errors() > 0):
            self.report.addInfo('Encountered '+ (str)(self.warnings()) +' warnings and ' + (str)(self.errors()) + ' errors.')
            return [self.warnings(), self.errors()]
        else:
            return True

        self.report.closeFile()


    def merge(self, xmlDoc1, xmlDoc2, outputFile = '', reportCheck = False):
        '''Merges two files together. Files are checked first during variables loading process.

        Second file is processed first, then the first file and after that they are merged together.
        '''
        self.output = outputFile
        self.file = XMLFile()
        self.variables = {}

        variables = self.file.variable(xmlDoc1)
        self._processVars(variables)
        variables = self.file.variable(xmlDoc2)
        self._processVars(variables)

        if not reportCheck:
            self.report.level = 5

        self.file.writeToFile(outputFile)

        self.report.addInfo('Files merged. Running check on the result.')
        self.check(self.output)
        self.report.closeFile()

    def _processVars(self, variables):
        for variable in variables:
            if variable[1] == 'declare':
                if variable[0] in self.variables.keys():
                    if variable[2].lower() != self.variables[variable[0]][0]:
                        raise Variable.EnvironmentError(variable[0], 'redeclaration')
                    else:
                        if variable[3].lower() != self.variables[variable[0]][1]:
                            raise Variable.EnvironmentError(variable[0], 'redeclaration')
                else:
                    self.file.writeVar(variable[0], 'declare', '', variable[2], variable[3])
                    self.variables[variable[0]] = [variable[2].lower(), variable[3].lower()]
            else:
                self.file.writeVar(variable[0], variable[1], variable[2])


    def _checkVariable(self, varName, action, local, value, nodeNum):
        '''Tries to add to variables dict, checks for errors during process'''

        if varName not in self.variables:
            self.variables[varName] = []
            self.variables[varName].append(action)

            '''If variable is in dict, check if this is not an unset command'''
        elif action == 'unset':
            if 'unset' in self.variables[varName]:
                self.report.addWarn('Multiple "unset" actions found for variable: "'+varName+'".', varName, 'multiple unset','', 'checkVariable')
            if not('unset' in self.variables[varName] and len(self.variables[varName]) == 1):
                self.report.addError('Node '+str(nodeNum)+': "unset" action found for variable "'+varName+'" after previous command(s). Any previous commands are overridden.', varName, 'unset overwrite')

                '''or set command'''
        elif action == 'set':
            if len(self.variables[varName]) == 1 and 'unset' in self.variables[varName]:
                self.report.addWarn('Node '+str(nodeNum)+': "set" action found for variable "'+varName+'" after unset. Can be merged to one set only.')
            else:
                self.report.addError('Node '+str(nodeNum)+': "set" action found for variable "'+varName+'" after previous command(s). Any previous commands are overridden.', varName, 'set overwrite')
                if 'set' in self.variables[varName]:
                    self.report.addWarn('Multiple "set" actions found for variable: "'+varName+'".', varName, 'multiple set','', 'checkVariable')

        if action not in self.variables[varName]:
            self.variables[varName].append(action)

        try:
            if action == 'remove-regexp':
                action = 'remove_regexp'
            eval('(self.realVariables[varName]).'+action+'(value)')
        except Variable.EnvironmentError as e:
            if e.code == 'undefined':
                self.report.addWarn('Referenced variable "' +e.val+ '" is not defined.')
            elif e.code == 'ref2var':
                self.report.addError('Reference to list from the middle of string.')
            elif e.code == 'redeclaration':
                self.report.addError('Redeclaration of variable "'+e.val+'".')
            else:
                self.report.addError('Unknown environment error occured.')


    def _loadVariables(self, fileName):
        '''loads XML file for input variables'''
        XMLfile = XMLFile()
        variables = XMLfile.variable(fileName)
        for variable in variables:
            undeclared = False
            if variable[0] == '':
                raise RuntimeError('Empty variable or local name is not allowed.')

            if variable[0] not in self.realVariables.keys():
                if variable[1] != 'declare':
                    self.report.addInfo('Node '+str(variable[4])+': Variable '+variable[0]+' is used before declaration. Treated as an unlocal list furthermore.')
                    undeclared = True
                    self.realVariables[variable[0]] = Variable.List(variable[0], False, report=self.report)
                else:
                    self.varNames.append(variable[0])
                    if variable[2] == 'list':
                        self.realVariables[variable[0]] = Variable.List(variable[0], variable[3], report=self.report)
                    else:
                        self.realVariables[variable[0]] = Variable.Scalar(variable[0], variable[3], report=self.report)
                    if not undeclared:
                        continue

            if variable[1] not in self.posActions:
                self.report.addError('Node '+str(variable[4])+': Action "'+variable[1]+'" which is not implemented found. Variable "'+variable[0]+'".', variable[0], variable[1] ,variable[2])
                continue

            else:
                if variable[1] == 'declare':
                    self.report.addError('Node '+str(variable[4])+': Variable '+variable[0]+' is redeclared.')
                else:
                    self._checkVariable(variable[0], variable[1], self.realVariables[variable[0]].local, (str)(variable[2]), variable[4])
