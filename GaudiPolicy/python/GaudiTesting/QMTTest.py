# -*- coding: utf-8 -*-
from BaseTest import *

class QMTTest(BaseTest):

    def __init__(self, path=None):
        BaseTest.__init__(self)
        self.validator = ''
        if path:
            self.XMLParser(path)

    def XMLParser(self, path) :
        import xml.etree.ElementTree as ET

        dic = self.__dict__

        tree = ET.parse(RationalizePath(path))
        root = tree.getroot()

        for child in root:
            type = child.attrib['name']
            dic['name']=path
            if type in dic :
                if type == 'args' or type == 'unsupported_platforms':
                    textList = child[0].findall('text')
                    for tl in textList :
                        dic[type]+=[tl.text]
                elif type == 'environment':
                     dic[type]={}
                     envList= child[0].findall('text')
                     for el in envList :
                         indexDictPart= el.text.rfind("=")
                         dic[type][el.text[:indexDictPart]]=el.text[indexDictPart+1:]
                else:
                    dic[type] = child[0].text
                    if child[0].tag == 'integer':
                        dic[type] = int(dic[type])


    def ValidateOutput(self, stdout, stderr, result):
        if self.validator != '' :
            class CallWrapper(object):
                """
                    Small wrapper class to dynamically bind some default arguments
                    to a callable.
                    """
                def __init__(self, callable, extra_args = {}):
                    self.callable = callable
                    self.extra_args = extra_args
                    # get the list of names of positional arguments
                    from inspect import getargspec
                    self.args_order = getargspec(callable)[0]
                    # Remove "self" from the list of positional arguments
                    # since it is added automatically
                    if self.args_order[0] == "self":
                        del self.args_order[0]
                def __call__(self, *args, **kwargs):
                    # Check which positional arguments are used
                    positional = self.args_order[:len(args)]

                    kwargs = dict(kwargs) # copy the arguments dictionary
                    for a in self.extra_args:
                        # use "extra_args" for the arguments not specified as
                        # positional or keyword
                        if a not in positional and a not in kwargs:
                            kwargs[a] = self.extra_args[a]
                    return apply(self.callable, args, kwargs)

            # local names to be exposed in the script
            exported_symbols = {"self":self,
                                "stdout":stdout,
                                "stderr":stderr,
                                "result":result,
                                "causes":self.causes,
                                "findReferenceBlock":
                                    CallWrapper(self.findReferenceBlock, {"stdout":stdout,
                                                                     "result":result,
                                                                     "causes":self.causes}),
                                "validateWithReference":
                                    CallWrapper(self.validateWithReference, {"stdout":stdout,
                                                                        "stderr":stderr,
                                                                        "result":result,
                                                                        "causes":self.causes}),
                                "countErrorLines":
                                    CallWrapper(self.countErrorLines, {"stdout":stdout,
                                                                  "result":result,
                                                                  "causes":self.causes}),
                                "checkTTreesSummaries":
                                    CallWrapper(self.CheckTTreesSummaries, {"stdout":stdout,
                                                                       "result":result,
                                                                       "causes":self.causes}),
                                "checkHistosSummaries":
                                    CallWrapper(self.CheckHistosSummaries, {"stdout":stdout,
                                                                       "result":result,
                                                                       "causes":self.causes})
                                }
            exec self.validator in globals(), exported_symbols
        else:
            if self.stderr=='':
                self.validateWithReference(stdout, stderr, result, self.causes)
            elif stderr!=self.stderr:
                self.causes.append("DIFFERENT STDERR THAN EXPECTED")

        return result,self.causes
