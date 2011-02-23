from Gaudi.Configuration import *
from GaudiKernel.Configurable import purge

option_file = "test_purge1.py"

importOptions(option_file)
first = configurationDict()

purge()
purged = configurationDict()

importOptions(option_file)
second = configurationDict()

from pprint import PrettyPrinter
pp = PrettyPrinter()

print "first =",
pp.pprint(first)

print "purged =",
pp.pprint(purged)

print "second =",
pp.pprint(second)
