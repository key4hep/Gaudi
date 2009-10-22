from GaudiKernel.ProcessJobOptions import InstallRootLoggingHandler
import logging
InstallRootLoggingHandler("# ", level = logging.INFO)

from Gaudi.Configuration import *

applied_order = []

class CommonConf(ConfigurableUser):
    __slots__ = { "Property1": 10,
                  "Property2": 10 }
    def __apply_configuration__(self):
        global applied_order
        applied_order.append(self.name())

class Application(ConfigurableUser):
    __slots__ = { "Property1": 10 }
    __queried_configurables__ = [ CommonConf ]
    def __apply_configuration__(self):
        global applied_order
        applied_order.append(self.name())

# Instantiate the application configurable
Application()
CommonConf()

# apply all ConfigurableUser instances
from GaudiKernel.Configurable import applyConfigurableUsers
applyConfigurableUsers()
print "Done."

expected_order = ["CommonConf", "Application"]
if applied_order != expected_order:
    import sys
    sys.stderr.write(("Error: wrong order of apply!\n"
                      "\texpected %r\n"
                      "\tfound %r\n")
                     % (expected_order, applied_order) )
    sys.exit(1)
