import os, sys, time
from xml.etree import ElementTree

def generateOptions(counter, cmask, invmask, sampling_period, startatevent, storeresultsat, family):
    cmask = map(int, cmask)
    invmask = map(int, invmask)
    sampling_period = map(int, sampling_period)
    startatevent = int(startatevent)

    from Configurables import ApplicationMgr, AuditorSvc, PerfMonAuditor
    app = ApplicationMgr()
    app.AuditAlgorithms = 1
    pfaud = PerfMonAuditor()
    try:
        pfaud.EVENT0 = counter[0]
        pfaud.SP0 = sampling_period[0]
        pfaud.INV0 = int(invmask[0])
        pfaud.CMASK0 = int(cmask[0])
        pfaud.EVENT1 = counter[1]
        pfaud.SP1 = sampling_period[1]
        pfaud.INV1 = int(invmask[1])
        pfaud.CMASK1 = int(cmask[1])
        pfaud.EVENT2 = counter[2]
        pfaud.SP2 = sampling_period[2]
        pfaud.INV2 = int(invmask[2])
        pfaud.CMASK2 = int(cmask[2])
        pfaud.EVENT3 = counter[3]
        pfaud.SP3 = sampling_period[3]
        pfaud.INV3 = int(invmask[3])
        pfaud.CMASK3 = int(cmask[3])
    except IndexError:
        pass
    pfaud.FAMILY = family
    pfaud.PREFIX = "%s_%s" % (storeresultsat, "S" if sampling_period[0] > 0 else "C") # for <2.5 Python use: test and true_value or false_value
    pfaud.SAMPLE = int(sampling_period[0] > 0)
    pfaud.START_AT_EVENT = startatevent
    #pfaud.LEVEL = 5
    AuditorSvc().Auditors.append(pfaud)
    print pfaud


class XmlDictObject(dict):
    """
    Adds object like functionality to the standard dictionary.
    """

    def __init__(self, initdict=None):
        if initdict is None:
            initdict = {}
        dict.__init__(self, initdict)
    
    def __getattr__(self, item):
        return self.__getitem__(item)
    
    def __setattr__(self, item, value):
        self.__setitem__(item, value)
    
    def __str__(self):
        if self.has_key('_text'):
            return self.__getitem__('_text')
        else:
            return ''

    @staticmethod
    def Wrap(x):
        """
        Static method to wrap a dictionary recursively as an XmlDictObject
        """

        if isinstance(x, dict):
            return XmlDictObject((k, XmlDictObject.Wrap(v)) for (k, v) in x.iteritems())
        elif isinstance(x, list):
            return [XmlDictObject.Wrap(v) for v in x]
        else:
            return x

    @staticmethod
    def _UnWrap(x):
        if isinstance(x, dict):
            return dict((k, XmlDictObject._UnWrap(v)) for (k, v) in x.iteritems())
        elif isinstance(x, list):
            return [XmlDictObject._UnWrap(v) for v in x]
        else:
            return x
        
    def UnWrap(self):
        """
        Recursively converts an XmlDictObject to a standard dictionary and returns the result.
        """

        return XmlDictObject._UnWrap(self)

def _ConvertDictToXmlRecurse(parent, dictitem):
    assert type(dictitem) is not type([])

    if isinstance(dictitem, dict):
        for (tag, child) in dictitem.iteritems():
            if str(tag) == '_text':
                parent.text = str(child)
            elif type(child) is type([]):
                # iterate through the array and convert
                for listchild in child:
                    elem = ElementTree.Element(tag)
                    parent.append(elem)
                    _ConvertDictToXmlRecurse(elem, listchild)
            else:                
                elem = ElementTree.Element(tag)
                parent.append(elem)
                _ConvertDictToXmlRecurse(elem, child)
    else:
        parent.text = str(dictitem)
    
def ConvertDictToXml(xmldict):
    """
    Converts a dictionary to an XML ElementTree Element 
    """

    roottag = xmldict.keys()[0]
    root = ElementTree.Element(roottag)
    _ConvertDictToXmlRecurse(root, xmldict[roottag])
    return root

def _ConvertXmlToDictRecurse(node, dictclass):
    nodedict = dictclass()
    
    if len(node.items()) > 0:
        # if we have attributes, set them
        nodedict.update(dict(node.items()))
    
    for child in node:
        # recursively add the element's children
        newitem = _ConvertXmlToDictRecurse(child, dictclass)
        if nodedict.has_key(child.tag):
            # found duplicate tag, force a list
            if type(nodedict[child.tag]) is type([]):
                # append to existing list
                nodedict[child.tag].append(newitem)
            else:
                # convert to list
                nodedict[child.tag] = [nodedict[child.tag], newitem]
        else:
            # only one, directly set the dictionary
            nodedict[child.tag] = newitem

    if node.text is None: 
        text = ''
    else: 
        text = node.text.strip()
    
    if len(nodedict) > 0:            
        # if we have a dictionary add the text as a dictionary value (if there is any)
        if len(text) > 0:
            nodedict['_text'] = text
    else:
        # if we don't have child nodes or attributes, just set the text
        nodedict = text
        
    return nodedict
        
def ConvertXmlToDict(root, dictclass=XmlDictObject):
    """
    Converts an XML file or ElementTree Element to a dictionary
    """

    # If a string is passed in, try to open it as a file
    if type(root) == type(''):
        root = ElementTree.parse(root).getroot()
    elif not isinstance(root, ElementTree.Element):
        raise TypeError, 'Expected ElementTree.Element or file path string'

    return dictclass({root.tag: _ConvertXmlToDictRecurse(root, dictclass)})
