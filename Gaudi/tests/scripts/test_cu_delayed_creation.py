from Gaudi.Configuration import *


class DelayedInstance(ConfigurableUser):
    __slots__ = {"Property": 0, "Applied": False}

    def __apply_configuration__(self):
        print "Applying", self.getName()
        print self
        self.Applied = True


class Application(ConfigurableUser):
    __slots__ = {"Property": 10, "Applied": False}
    __used_configurables__ = []

    def __apply_configuration__(self):
        print "Applying", self.getName()
        print self
        self.Applied = True
        # This is instantiated late
        DelayedInstance()


# Instantiate the application configurable
Application()

# apply all ConfigurableUser instances
from GaudiKernel.Configurable import applyConfigurableUsers
applyConfigurableUsers()

assert Application().Applied
assert DelayedInstance().Applied

print "Done."
