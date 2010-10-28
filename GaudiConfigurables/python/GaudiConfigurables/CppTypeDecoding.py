'''
Module for parsing the C++ template identifiers.

@author: Marco Clemencic
'''
# Pure python module to easily write parsers
from pyparsing import (Word, ZeroOrMore, OneOrMore, Forward, Optional,
                       alphas, alphanums, Literal,
                       Group, Suppress, Combine, Or)
import re

c_keywords = Or([ Literal(i) for i in ["int", "long", "char", "short",
                                       "signed", "unsigned",
                                       "float", "double",
                                       "void",
                                       "const"]])
c_ptr = Literal("*")
c_ref = Literal("&")
type_decoration = Combine(Optional(c_ref) + ZeroOrMore(c_ptr), adjacent=False)

c_type = Combine(Combine(OneOrMore(c_keywords), joinString=" ", adjacent=False)
                 + Optional(type_decoration), adjacent=False)
# identifier
ident = Word(alphas+"_", alphanums+"_")
# type name or class bare name
barename = Combine(ident + ZeroOrMore("::" + ident), adjacent=False)
# template class declaration
cpp_type = Forward()
template = barename + Optional(Suppress('<') 
                               + Group(cpp_type
                                       + ZeroOrMore(Suppress(',') + cpp_type))
                               + Suppress('>')) \
                    + Optional(type_decoration)
cpp_type << Or([c_type, template])
#template << name + Group(Optional(Suppress('<') 
#                            + template + ZeroOrMore(Suppress(',') + template)
#                            + Suppress('>')))

parser = cpp_type

class Type(object):
    """
    Class representing a C++ type.
    It has a name and an optional template arguments list.
    """
    __slots__ = ("name", "templated", "args", "decoration")
    def __init__(self, name, template_args = None, decoration = ""):
        """
        Construct and instance.
        
        @param name: bare name of the C++ type or class (e.g. 'int', 'std::string')
        @param template_args: template arguments, can be one Type instance a string
                              or an iterable containing Type instances or strings.
        """
        self.name = name
        self.templated = template_args is not None
        if self.templated:
            if (type(template_args) is str
                or not hasattr(template_args, "__iter__")):
                template_args = [template_args]
            self.args = []
            for t in template_args:
                if type(t) is str:
                    t = Type(t)
                self.args.append(t)
        self.decoration = decoration
    def __str__(self):
        """
        Return the C++ style type string.
        """
        result = self.name
        if self.templated:
            result += "<%s>" % (",".join([ str(i) for i in self.args ]))
            while ">>" in result:
                result = result.replace(">>", "> >")
        return result + self.decoration
    def __repr__(self):
        """
        Return a valid representation of the instance.
        """
        args = repr(self.name)
        if self.templated:
            args += "," + repr(self.args)
        if self.decoration:
            args += ","
            if not self.templated:
                args += "decoration="
            args += repr(self.decoration)
        result = "%s(%s)" % (self.__class__.__name__, args)
        return result

decorator_re = re.compile(r"^&?\**$")
def makeType(result):
    """
    Translate the result of the parsing into a Type instance.
    """
    d = []
    # Typical format: [ str, str, [] ]
    while result:
        k = str(result.pop(0))
        v = None
        dec = ""
        if result and type(result[0]) is not str:
            v = makeType(result.pop(0))
        if result:
            r = result[0]
            if type(r) is str and decorator_re.match(r):
                dec = result.pop(0)
        d.append(Type(k, v, dec))
    if len(d) == 1:
        d = d.pop()
    return d

def parse(s):
    """
    Parse the string type and return the corresponding Type instance.
    """
    return makeType(parser.parseString(s))

if __name__ == "__main__":
    test_strings = [
        "std::string",
        "std ::  basic_string",
        "map<std::string, int>",
        "std::vector < std::pair<int, double> >",
        "std::map < std::pair<int, double>, std:: basic_string<char> >",
        "string<int, pair<a,b>>",
        "std::pair<int, unsigned long long>",
        "std::unordered_map<std::string, const char *>",
        "std::unordered_map<std::string & **, const char *>&",
        ]
    for tn in test_strings:
        ti = parse(tn)
        print tn, "->"
        print "\t%s" % parser.parseString(tn)
        print "\t%s" % ti
        print "\t%r" % ti
