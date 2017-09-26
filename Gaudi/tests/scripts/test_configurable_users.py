from Gaudi.Configuration import *

appliedConf = []


class ConfigurableUserTest(ConfigurableUser):
    __slots__ = {}

    def __apply_configuration__(self):
        print "Applying", self.getName()
        print self
        appliedConf.append(self.getName())


class NotActivelyUsed(ConfigurableUserTest):
    __slots__ = {"Property": -1}


class SubModule1(ConfigurableUserTest):
    __slots__ = {"Property1": 0}


class SubModule2(ConfigurableUserTest):
    __slots__ = {"Property1": 2}


class SubModule3(ConfigurableUserTest):
    __slots__ = {"Property1": 3}
    __used_configurables__ = [NotActivelyUsed]


class MultiInstance(ConfigurableUserTest):
    __slots__ = {"Property": 0}
    __used_configurables__ = [(SubModule1, None)]

    def __apply_configuration__(self):
        super(MultiInstance, self).__apply_configuration__()
        SubModule1(self._instanceName(SubModule1))


class Application(ConfigurableUserTest):
    __slots__ = {"Property1": 10}
    __used_configurables__ = [SubModule1, SubModule2, SubModule3,
                              (MultiInstance, None), (MultiInstance, "TestInstance")]

    def __apply_configuration__(self):
        super(Application, self).__apply_configuration__()
        if hasattr(self, "Property1"):
            val = self.Property1
        else:
            val = self.getDefaultProperty("Property1")
        SubModule1(Property1=val)
        SubModule2(Property1=val)
        # SubModule3 is not instantiated explicitly, so not enabled
        MultiInstance(self._instanceName(MultiInstance))
        ti = self.getUsedInstance("TestInstance")
        ti.Property = 1
        NotActivelyUsed(Property=-5)


calledActions = []


class Action(object):
    def __init__(self, msg):
        self.msg = msg

    def __call__(self):
        calledActions.append(self.msg)
        print self.msg


def ActionFunction():
    calledActions.append("Action Function")
    print "Action Function"


appendPostConfigAction(Action("Action Object One"))
appendPostConfigAction(ActionFunction)
appendPostConfigAction(Action("Action Object Two"))

# Instantiate the application configurable
Application()

# apply all ConfigurableUser instances
from GaudiKernel.Configurable import applyConfigurableUsers
applyConfigurableUsers()

# check that everything has been applied
expected = {'Application':               ('Property1', 10),
            'TestInstance':              ('Property', 1),
            'Application_MultiInstance': ('Property', 0),
            'SubModule1':                ('Property1', 10),
            'SubModule2':                ('Property1', 10),
            'TestInstance_SubModule1':   ('Property1', 0),
            'Application_MultiInstance_SubModule1': ('Property1', 0),
            'NotActivelyUsed':           ('Property', -5)}
assert set(appliedConf) == set(expected)

# check the order of application
assert appliedConf.index('Application') < appliedConf.index('SubModule1')
assert appliedConf.index('Application') < appliedConf.index('SubModule2')
assert appliedConf.index('Application') < appliedConf.index(
    'Application_MultiInstance')
assert appliedConf.index('Application') < appliedConf.index('TestInstance')
assert appliedConf.index('Application') < appliedConf.index('NotActivelyUsed')

assert appliedConf.index('TestInstance') < appliedConf.index(
    'TestInstance_SubModule1')

assert appliedConf.index('Application_MultiInstance') < appliedConf.index(
    'Application_MultiInstance_SubModule1')

# check that the actions has been called in the right order
expectedActions = ["Action Object One", "Action Function", "Action Object Two"]
assert calledActions == expectedActions

# check property values
allConfs = Configurable.allConfigurables
for name, (prop, value) in expected.items():
    assert allConfs[name].getProp(
        prop) == value, "%s.%s != %s" % (name, prop, value)

print "Success."
