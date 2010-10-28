'''
Created on 29/mag/2010

@author: Marco Clemencic
'''
import unittest

from GaudiConfigurables import *
from GaudiConfigurables.Properties import *
from GaudiConfigurables.Validators import *
from CppTypeDecoding import Type, parse

class TestAlgorithm(Algorithm):
    __properties__ = (BaseProperty("OutputLevel",
                               CTypesValidator("int"),
                               3,
                               "something"),
                      VectorProperty("Items",
                                     CTypesValidator("int"),
                                     doc = "A vector of ints"),
                      VectorProperty("Tools",
                                     StringValidator(),
                                     doc = "A vector of strings"))

class MyAlgorithm(TestAlgorithm):
    __properties__ = (BaseProperty("OutputLevel",
                               StringValidator(),
                               "ciao",
                               "something new"),)

class Test(unittest.TestCase):


    def setUp(self):
        pass


    def tearDown(self):
        # Remove all instances
        Configurable._instances.clear()

    def test_000_ValidatorError(self):
        "ValidatorError"
        str(ValidatorError("int", "value"))

    def test_000_defaultValidator(self):
        "defaultValidator"
        defaultValidator("anything")

#    def test_000_CppTypes(self):
#        "Decoding of C++ types"
#        test_strings = [
#                        ("std::string",
#                            Type("std::string")),
#                        ("std ::  basic_string",
#                            Type("std::basic_string")),
#                        ("map<std::string , int>",
#                            Type("map", ["std::string","int"])),
#                        ("std::vector < std::pair<int, double> >",
#                            Type("std::vector",[Type("std::pair", ["int","double"])]),
#                        ("std::map < std::pair<int, double>, std:: basic_string<char> >",
#                            Type("std::map", [Type("std::pair", ["int", "double"]),
#                                              Type("std:: basic_string", "char")])),
#                        ("string<int, pair<a,b>>",
#                            Type("string", [Type("int"),
#                                            Type("pair", ["a","b"])])),
#                        ]


    def test_000_validators(self):
        "Validators"
        i32 = CIntValidator(32)
        i32(-100)
        i32(0)
        i32(100)
        self.failUnlessRaises(ValidatorError, i32, 1 << 31)
        self.failUnlessRaises(ValidatorError, i32, "test")
        self.failUnlessRaises(ValidatorError, i32, [])

        ui32 = CIntValidator(32, signed = False)
        self.failUnlessRaises(ValidatorError, ui32, -100)
        ui32(0)
        ui32(100)
        self.failUnlessRaises(ValidatorError, ui32, 1 << 32)

        fp = CFPValidator()
        fp(100)
        fp(1000L)
        fp(1e10)
        self.failUnlessRaises(ValidatorError, fp, [])
        self.failUnlessRaises(ValidatorError, fp, "test")

        s = StringValidator()
        s("test")
        self.failUnlessRaises(ValidatorError, s, [])
        self.failUnlessRaises(ValidatorError, s, 10)

    def test_010_singleton(self):
        "Named singleton"
        a = TestAlgorithm("a")
        b = TestAlgorithm("b")
        self.assert_(a is TestAlgorithm("a"))
        self.assert_(b is TestAlgorithm("b"))
        self.assert_(a is not b)

        # fail to get the singleton with the wrong type
        self.failUnlessRaises(TypeError, MyAlgorithm, "a")

    def test_020_simple_type_check(self):
        "Simple type checking"
        self.failUnlessRaises(TypeError, TestAlgorithm, OutputLevel = "abc")
        self.failUnlessRaises(TypeError, TestAlgorithm, Items = ["abc"])
        self.failUnlessRaises(TypeError, TestAlgorithm, Items = 10)
        self.failUnlessRaises(TypeError, TestAlgorithm, Items = "abc")
        self.failUnlessRaises(TypeError, TestAlgorithm, Tools = [10])
        self.failUnlessRaises(TypeError, TestAlgorithm, Tools = 10)
        self.failUnlessRaises(TypeError, TestAlgorithm, Tools = "abc")
        a = TestAlgorithm()
        a.OutputLevel = 100
        a.Items = range(3)
        a.Tools = ["one", "two", "three"]
        self.assertEquals(a.OutputLevel, 100)
        self.assertEquals(a.Items, range(3))
        self.assertEquals(a.Tools, ["one", "two", "three"])

    def test_021_const_class(self):
        "Class constantness"
        try:
            TestAlgorithm.OutputLevel = 1
            self.fail("modified class object")
        except AttributeError:
            pass

    def test_022_instance_name(self):
        "Instance name"
        a = TestAlgorithm("name1")
        self.assert_("name1" in Configurable._instances)
        self.assert_(Configurable._instances["name1"] is a)
        # rename
        a.name = "name2"
        self.assert_("name1" not in Configurable._instances)
        self.assert_("name2" in Configurable._instances)
        self.assert_(Configurable._instances["name2"] is a)

        # rename to an already used name
        TestAlgorithm("name3")
        try:
            a.name = "name3"
            self.fail("I managed to rename an instance using an already used name")
        except ValueError:
            pass

        # try to unset the name
        try:
            del a.name
            self.fail("I managed unset the name of an instance")
        except AttributeError:
            pass

    def test_025_default_assignment(self):
        "Assignment to default"
        a = TestAlgorithm()

        self.assert_(not a.isSet("OutputLevel"))
        a.OutputLevel = a.OutputLevel
        self.assert_(a.isSet("OutputLevel"))

        self.assert_(not a.isSet("Items"))
        a.Items = a.Items
        self.assert_(a.isSet("Items"))

    def test_025_default_retrieval(self):
        "Default retrieval"
        a = TestAlgorithm()
        self.assert_(not a.isSet("OutputLevel"))
        self.assertEquals(a.getDefault("OutputLevel"), 3)
        a.OutputLevel = 10
        self.assert_(a.isSet("OutputLevel"))
        self.assertEquals(a.getDefault("OutputLevel"), 3)
        self.failUnlessRaises(AttributeError, a.getDefault, "Undefined")

        self.assert_(not a.isSet("Items"))
        self.assertEquals(a.getDefault("Items"), [])

    def test_030_vector_semantics(self):
        "Vector semantics"
        a = TestAlgorithm("a")
        self.assert_(not a.isSet("Items"))
        a.Items += [1]
        self.assert_(a.isSet("Items"))
        self.assertEquals(a.Items, [1])

        b = TestAlgorithm("b")
        self.assert_(not b.isSet("Items"))
        b.Items.append(2)
        self.assert_(b.isSet("Items"))
        self.assertEquals(b.Items, [2])

    def test_040_properties(self):
        "Properties"
        a = TestAlgorithm()
        self.assertEquals(set(a.propertyNames()), set(["OutputLevel", "Items", "Tools"]))
        self.assertEquals(a.propertyDict(), {})
        self.assertEquals(a.propertyDict(defaults = True), {"OutputLevel": 3,
                                                            "Items": [],
                                                            "Tools": []})
        a.OutputLevel = 5
        self.assertEquals(a.propertyDict(), {"OutputLevel": 5})
        self.assertEquals(a.propertyDict(defaults = True), {"OutputLevel": 5,
                                                            "Items": [],
                                                            "Tools": []})
        a.Items = range(3)
        self.assertEquals(a.propertyDict(), {"OutputLevel": 5, "Items": range(3)})
        self.assertEquals(a.propertyDict(defaults = True), {"OutputLevel": 5,
                                                            "Items": range(3),
                                                            "Tools": []})

    def test_041_inherited_properties(self):
        "Inherited properties"
        a = MyAlgorithm()
        self.assertEquals(set(a.propertyNames()), set(["OutputLevel", "Items", "Tools"]))
        self.assertEquals(a.propertyDict(), {})
        self.assertEquals(a.propertyDict(defaults = True), {"OutputLevel": "ciao",
                                                            "Items": [],
                                                            "Tools": []})
        a.OutputLevel = "test"
        self.assertEquals(a.propertyDict(), {"OutputLevel": "test"})
        self.assertEquals(a.propertyDict(defaults = True), {"OutputLevel": "test",
                                                            "Items": [],
                                                            "Tools": []})
        a.Items = range(3)
        self.assertEquals(a.propertyDict(), {"OutputLevel": "test", "Items": range(3)})
        self.assertEquals(a.propertyDict(defaults = True), {"OutputLevel": "test",
                                                            "Items": range(3),
                                                            "Tools": []})

    def test_050_pickling(self):
        "Pickling"
        import pickle
        TestAlgorithm(OutputLevel = 20, Items = range(5))
        TestAlgorithm("Prova")
        MyAlgorithm("ATest")
        MyAlgorithm(OutputLevel = "ciao")
        MyAlgorithm(Items = range(2))
        before = dict(Configurable._instances)

        s = pickle.dumps(Configurable._instances, 2)
        Configurable._instances.clear()
        pickle.loads(s)
        after = dict(Configurable._instances)

        bk = before.keys()
        ak = after.keys()
        bk.sort()
        ak.sort()
        self.assertEquals(ak, bk)
        for k in bk:
            self.assertEquals(before[k].propertyDict(), after[k].propertyDict())

    def test_051_representation(self):
        "Repr format"
        TestAlgorithm(OutputLevel = 20, Items = range(5))
        TestAlgorithm("Prova")
        MyAlgorithm("ATest")
        MyAlgorithm(OutputLevel = "ciao")
        MyAlgorithm(Items = range(2))
        before = dict(Configurable._instances)

        s = "\n".join(map(repr, Configurable._instances.values()))
        Configurable._instances.clear()
        exec s
        after = dict(Configurable._instances)

        bk = before.keys()
        ak = after.keys()
        bk.sort()
        ak.sort()
        self.assertEquals(ak, bk)
        for k in bk:
            self.assertEquals(before[k].propertyDict(), after[k].propertyDict())

    def test_080_changetype_of_instance(self):
        "Change the type of an instance"
        NewType = MyAlgorithm
        old_instance = TestAlgorithm("test", Items = range(3))
        # Quoted from the documentation
        name = old_instance.name
        old_instance.name = name + "_old"
        new_instance = NewType(name, **old_instance.propertyDict())

        self.assertEquals(new_instance.propertyDict(), {"Items": range(3)})

    def _test_300_class_alias(self):
        "Aliases for configurable classes"
        class TestAlgAlias(TestAlgorithm):
            pass
        # I do not know what to expect yet

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.test_010_singleton']
    unittest.main()
