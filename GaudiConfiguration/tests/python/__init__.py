#####################################################################################
# (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os

from GaudiKernel.GaudiHandles import PrivateToolHandle, PrivateToolHandleArray

os.environ["GAUDICONFIG2_DB"] = __name__ + "._DB"


class FooProperty:
    """Property type to test DefaultSemantics"""

    def __init__(self, value=""):
        self.AString = value

    def __opt_value__(self):
        return self.AString

    def __eq__(self, other):
        return type(self) is type(other) and self.AString == other.AString


_DB = {
    "MyAlgorithm": {},
    "MyOtherAlg": {},
    "Gaudi::SomeAlg": {},
    "Gaudi::TestSuite::Test1": {},
    "Gaudi::Foundation::Service": {},
    "TestConf::MyAlg": {
        "__component_type__": "Algorithm",
        "__doc__": "this is My Class",
        "properties": {
            "NoValidProp": ("no-validated-type", "null"),
            "AStringProp": ("std::string", "text", "some doc string"),
            "AnIntProp": ("int", -1),
            "AnULongProp": ("unsigned long", 0),
            "AFloatProp": ("double", 0),
            "ABoolProp": ("bool", False),
        },
    },
    "TestConf::SimpleOptsAlgTool": {
        "__component_type__": "AlgTool",
        "__interfaces__": ("IToolType1", "IToolType2", "Gaudi::Interfaces::INSTool"),
        "__doc__": "configurable for testing options strings",
        "properties": {
            "Bool": ("bool", False),
        },
    },
    "TestConf::SimpleOptsAlg": {
        "__component_type__": "Algorithm",
        "__doc__": "configurable for testing options strings",
        "properties": {
            "Int": ("int", 0),
            "String": ("std::string", "default"),
            "Tool": ("AlgTool:IToolType1", "TestConf::SimpleOptsAlgTool"),
        },
    },
    "TestConf::AlgWithVectors": {
        "__component_type__": "Algorithm",
        "__doc__": "configurable for testing vector options",
        "properties": {
            "VS": ("std::vector<std::string,std::allocator<std::string> >", []),
            "VVS": (
                "std::vector<std::vector<std::string,std::allocator<std::string> >,std::allocator<std::vector<std::string,std::allocator<std::string> > > >",
                [],
            ),
        },
    },
    "TestConf::AlgWithSets": {
        "__component_type__": "Algorithm",
        "__doc__": "configurable for testing set options",
        "properties": {
            "SetOfInt": ("std::unordered_set<int>", set()),
            "SetOfString": ("std::unordered_set<std::string>", set()),
        },
    },
    "TestConf::AlgWithMaps": {
        "__component_type__": "Algorithm",
        "__doc__": "configurable for testing map options",
        "properties": {
            "MSS": ("std::map<std::string,std::string>", {}),
            "MIV": (
                "std::map<int, std::vector<std::string,std::allocator<std::string> >>",
                {},
            ),
        },
    },
    "TestConf::MyTool": {
        "__component_type__": "AlgTool",
        "__doc__": "this is MyTool",
        "properties": {
            "AString": ("std::string", "text", "some doc string"),
        },
    },
    "TestConf::TemplatedAlg<int, std::vector<std::string, std::allocator<std::string> > >": {},
    "TestConf::AlgWithComplexProperty": {
        "__component_type__": "Algorithm",
        "__doc__": "configurable for testing default semantics on non trivial properties",
        "properties": {
            "DefProp": ("FooProperty", FooProperty("Foo")),
            "TH": ("PrivateToolHandle", PrivateToolHandle("TestConf::MyTool/SomeTool")),
            "EmptyTH": ("PrivateToolHandle", PrivateToolHandle()),
            "THA": ("PrivateToolHandleArray", PrivateToolHandleArray()),
            "VS": ("std::vector<std::string,std::allocator<std::string> >", []),
            "OS": (
                "std::vector<std::string,std::allocator<std::string> >",
                [],
                "",
                "OrderedSet<string>",
            ),
        },
    },
}
