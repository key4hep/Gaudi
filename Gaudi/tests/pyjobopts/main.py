from Gaudi.Configuration import *

MessageSvc().OutputLevel = INFO;

# Hack needed for the test
from GaudiKernel.Configurable import ConfigurableGeneric
myAlg = ConfigurableGeneric("myAlg")
myAlg.Dict = { "a":1, "b":2,
              # "c":3 commented-out
             } # another comment

importOptions("test1.opts")
importOptions("test2.opts")
importOptions("test1.opts")

importOptions("test1.py")
