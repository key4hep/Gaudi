'''
Module for parsing the C++ template identifiers.

@author: Marco Clemencic
'''
# Pure python module to easily write parsers
from pyparsing import (Word, ZeroOrMore, Forward, Optional,
                       alphas, alphanums,
                       delimitedList, Group, Suppress, Combine, Dict, Or)

# identifier
ident = Word(alphas+"_", alphanums+"_")
# type name or class bare name
name = Combine(ident + ZeroOrMore("::" + ident), adjacent=False)
# template class declaration
template = Forward()
template << name + Optional(Suppress('<') 
                            + Group(template + ZeroOrMore(Suppress(',') + template))
                            + Suppress('>'))
#template << name + Group(Optional(Suppress('<') 
#                            + template + ZeroOrMore(Suppress(',') + template)
#                            + Suppress('>')))

#test_strings = [
#    "std::string",
#    "std ::  basic_string",
#    "map<std::string, int>",
#    "std::vector < std::pair<int, double> >",
#    "std::map < std::pair<int, double>, std:: basic_string<char> >",
#    "string<int, pair<a,b>>",
#    ]

class Type(object):
    def __init__(self, name, template_args = None):
        self.name = name
        self.templated = template_args is not None
        if self.templated and not hasattr(template_args, "__iter__"):
            template_args = [template_args]
        self.args = template_args
    def __str__(self):
        result = name
        if self.templated:
            result += "<%s>" % (",".join(map(str, self.args)))
            while ">>" in result:
                result = result.replace(">>", "> >")
        return result
    def __repr__(self):
        args = self.name
        if self.templated:
            args += repr(self.args)
        result = "%s(%s)" % (self.__class__.__name__, args)
        return result

# [ str, str, [] ]
def makeType(result):
    d = []
    while result:
        k = result.pop(0)
        if result and type(result[0]) is not str:
            v = makeType(result.pop(0))
            d.append(Type(k, v))
        else:
            d.append(Type(k))
    if len(d) == 1:
        d = d.pop()
    return d

def parse(s):
    return makeType(template.parseString(s))
