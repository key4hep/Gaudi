from Gaudi.Configuration import *

class SubModule1(ConfigurableUser):
    __slots__ = { "Property1": 0 }
    def __apply_configuration__(self):
        print "Applying", self.getName()
        print self

class SubModule2(ConfigurableUser):
    __slots__ = { "Property1": 2 }
    def __apply_configuration__(self):
        print "Applying", self.getName()
        print self

class SubModule3(ConfigurableUser):
    __slots__ = { "Property1": 3 }
    def __apply_configuration__(self):
        print "Applying", self.getName()
        print self

class Application(ConfigurableUser):
    __slots__ = { "Property1": 10 }
    __used_configurables__ = [ SubModule1, SubModule2, SubModule3 ]
    def __apply_configuration__(self):
        print "Applying", self.getName()
        print self
        if hasattr(self,"Property1"):
            val = self.Property1
        else:
            val = self.getDefaultProperty("Property1")
        SubModule1(Property1 = val)
        SubModule2(Property1 = val)
        # SubModule3 is not instantiated explicitly, so not enabled

class Action(object):
    def __init__(self, msg):
        self.msg = msg
    def __call__(self):
        print self.msg

def ActionFunction():
    print "Action Function"

appendPostConfigAction(Action("Action Object One"))
appendPostConfigAction(ActionFunction)
appendPostConfigAction(Action("Action Object Two"))

# Instantiate the application configurable
Application()

# apply all ConfigurableUser instances
from GaudiKernel.Configurable import applyConfigurableUsers
applyConfigurableUsers()
print "Done."
