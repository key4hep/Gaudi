'''
Created on Jun 27, 2011

@author: mplajner
'''
import xmlModule
import os
import platform
from time import gmtime, strftime
import Variable

class Environment():
    '''object to hold settings of environment'''

    def __init__(self, loadFromSystem = True, useAsWriter=False, reportLevel = 1):
        '''Initial variables to be pushed and setup

        append switch between append and prepend for initial variables.
        loadFromSystem causes variable`s system value to be loaded on first encounter.
        If useAsWriter == True than every change to variables is recorded to XML file.
        reportLevel sets the level of messaging.
        **kwargs contain initial variables.
        '''
        self.report = xmlModule.Report(reportLevel)

        if platform.system() != 'Linux':
            self.sysSeparator = ';'
        else:
            self.sysSeparator = ':'
        self.separator = ':'

        self.posActions = ['append','prepend','set','unset','remove', 'remove-regexp', 'declare']
        self.variables = {}
        self.loadFromSystem = loadFromSystem
        self.asWriter = useAsWriter
        if useAsWriter:
            self.writer = xmlModule.XMLFile()
            self.startXMLinput()


    def vars(self, strings=True):
        '''returns dictionary of all variables optionally converted to string'''
        if strings:
            vars = self.variables.copy()
            for item in vars:
                vars[item] = self.var(item).value(True)
            return vars
        else:
            return self.variables


    def var(self, name):
        '''Gets a single variable. If not available then tries to load from system.'''
        if name in self.variables.keys():
            return self.variables[name]
        else:
            self._loadFromSystem(name, '', 'append')
            return self.variables[name]


    def search(self, varName, expr, regExp = False):
        '''Searches in a variable for a value.'''
        return self.variables[varName].search(expr, regExp)


    def append(self, name, value):
        '''Appends to an existing variable.'''
        if self.asWriter:
            self._writeVarToXML(name, 'append', value)
        else:
            if name in self.variables.keys():
                self.variables[name].append(value, self.separator, self.variables)
            else:
                self.declare(name, 'list', False)
                self.append(name, value)


    def prepend(self, name, value):
        '''Prepends to an existing variable, or create a new one.'''
        if self.asWriter:
            self._writeVarToXML(name, 'prepend', value)
        else:
            if name in self.variables.keys():
                self.variables[name].prepend(value, self.separator, self.variables)
            else:
                self.declare(name, 'list', False)
                self.prepend(name, value)

    def declare(self, name, type, local):
        '''Creates an instance of new variable. It loads values from the OS if the variable is not local.'''
        if self.asWriter:
            self._writeVarToXML(name, 'declare', '', type, local)

        if not isinstance(local, bool):
            if str(local).lower() == 'true':
                local = True
            else:
                local = False

        if name in self.variables.keys():
            if self.variables[name].local != local:
                raise Variable.EnvironmentError(name, 'redeclaration')
            else:
                if type.lower() == "list":
                    if not isinstance(self.variables[name],Variable.List):
                        raise Variable.EnvironmentError(name, 'redeclaration')
                else:
                    if not isinstance(self.variables[name],Variable.Scalar):
                        raise Variable.EnvironmentError(name, 'redeclaration')

        if type.lower() == "list":
            a = Variable.List(name, local, report=self.report)
        else:
            a = Variable.Scalar(name, local, report=self.report)

        if self.loadFromSystem and not local:
            if name in os.environ.keys():
                a.set(os.environ[name], self.sysSeparator, environment=self.variables, resolve=False)
        self.variables[name] = a

    def set(self, name, value):
        '''Sets a single variable - overrides any previous value!'''
        name = str(name)
        if self.asWriter:
            self._writeVarToXML(name, 'set', value)
        else:
            if name in self.variables:
                self.variables[name].set(value, self.separator, self.variables)
            else:
                self.declare(name, 'list', False)
                self.set(name, value)


    def searchFile(self, file, varName):
        '''Searches for appearance of variable in a file.'''
        XMLFile = xmlModule.XMLFile()
        variable = XMLFile.variable(file, name=varName)
        return variable


    def unset(self, name, value=None):
        '''Unsets a single variable to an empty value - overrides any previous value!'''
        if self.asWriter:
            self._writeVarToXML(name, 'unset', '')
        else:
            if name in self.variables:
                self.variables[name].set([], self.separator)
            else:
                a = Variable.List(name, report=self.report)
                self.variables[name] = a


    def remove(self, name, value, regexp=False):
        '''Remove value from variable.'''
        if self.asWriter:
            self._writeVarToXML(name, 'remove', value)
        else:
            if name in self.variables:
                self.variables[name].remove(value, self.separator, regexp)
            elif self.loadFromSystem:
                self.declare(name, 'list', False)


    def remove_regexp(self, name, value):
        self.remove(name, value, True)

    def loadXML(self, fileName = None, namespace = 'EnvSchema'):
        '''Loads XML file for input variables.'''
        XMLfile = xmlModule.XMLFile()
        variables = XMLfile.variable(fileName, namespace = namespace)
        i = 0
        for variable in variables:
            i += 1
            if variable[1] not in self.posActions:
                self.report.addError('Node '+str(i)+': No action taken with var "' + variable[0] + '". Probably wrong action argument: "'+variable[1]+'".')
            elif variable[1] == 'declare':
                self.declare(str(variable[0]), str(variable[2]), variable[3])
            else:
                if variable[1] == 'remove-regexp':
                    variable[1] = 'remove_regexp'
                eval('self.'+variable[1]+'(str(variable[0]), str(variable[2]))')


    def startXMLinput(self):
        '''Renew writer for new input.'''
        self.writer.resetWriter()


    def finishXMLinput(self, outputFile = ''):
        '''Finishes input of XML file and closes the file.'''
        self.writer.writeToFile(outputFile)


    def writeToFile(self, fileName, shell='sh'):
        '''Creates an output file with a specified name to be used for setting variables by sourcing this file'''
        f = open(fileName, 'w')
        if shell == 'sh':
            f.write('#!/bin/bash\n')
            for variable in self.variables:
                if not self[variable].local:
                    f.write('export ' +variable+'='+self[variable].value(True, self.sysSeparator)+'\n')
        elif shell == 'csh':
            f.write('#!/bin/csh\n')
            for variable in self.variables:
                if not self[variable].local:
                    f.write('setenv ' +variable+' '+self[variable].value(True, self.sysSeparator)+'\n')
        else:
            f.write('')
            f.write('REM This is an enviroment settings file generated on '+strftime("%a, %d %b %Y %H:%M:%S\n", gmtime()))
            for variable in self.variables:
                if not self[variable].local:
                    f.write('set '+variable+'='+self[variable].value(True, self.sysSeparator)+'\n')

        f.close()


    def writeToXMLFile(self, fileName):
        '''Writes the current state of environment to a XML file.

        NOTE: There is no trace of actions taken, variables are written with a set action only.
        '''
        writer = xmlModule.XMLFile()
        for var in self.variables.keys():
            writer.writeVar(var, 'set', self.variables[var].value(True, self.separator))
        writer.writeToFile(fileName)


    def loadAllSystemVariables(self):
        '''Loads all variables from the current system settings.'''
        for val in os.environ.keys():
            if not val in self.variables.keys():
                self.declare(val, 'list', False)


    def _concatenate(self, value):
        '''Returns a variable string with separator separator from the values list'''
        stri = ""
        for it in value:
            stri += it + self.separator
        stri = stri[0:len(stri)-1]
        return stri


    def _writeVarToXML(self, name, action, value, type='list', local='false'):
        '''Writes single variable to XML file.'''
        if isinstance(value, list):
            value = self._concatenate(value)
        self.writer.writeVar(name, action, value, type, local)


    def __getitem__(self, key):
        return self.variables[key]

    def __setitem__(self, key, value):
        if value in self.variables.keys():
            self.report.addWarn('Addition canceled because of duplicate entry. Var: "' + self.varName + '" value: "' + value + '".')
        else:
            self.append(key, value)

    def __delitem__(self, key):
        del self.variables[key]

    def __iter__(self):
        for i in self.variables:
            yield i

    def __contains__(self, item):
        return item in self.variables.keys()

    def __len__(self):
        return len(self.variables.keys())

