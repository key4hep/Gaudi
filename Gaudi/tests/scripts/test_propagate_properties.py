from Gaudi.Configuration import *

# use cases:
#     parent, used
#   A set   , set
#   B set   , unset
#   C unset , set
#   D unset , unset

PS = "Parent Set"
PU = "Parent Unset"
CS = "Child Set"
CU = "Child Unset"

class PlainConfigurable(ConfigurableAlgTool):
    __slots__ = { "IntA": CU,
                  "IntB": CU,
                  "IntC": CU,
                  "IntD": CU,
                  "LstA": [CU],
                  "LstB": [CU],
                  "LstC": [CU],
                  "LstD": [CU] }
    def __init__(self, name = Configurable.DefaultName, **kwargs):
        super(PlainConfigurable, self).__init__(name)
        for n,v in kwargs.items():
            setattr(self, n, v)
    def getDlls( self ):
        return 'None'
    def getType( self ):
        return 'PlainConfigurable'

class Master(ConfigurableUser):
    __slots__ = { "IntA": PU,
                  "IntB": PU,
                  "IntC": PU,
                  "IntD": PU,
                  "LstA": [PU],
                  "LstB": [PU],
                  "LstC": [PU],
                  "LstD": [PU] }
    def __apply_configuration__(self):
        self.propagateProperties(others = [PlainConfigurable()])
    def getGaudiType( self ):
        return 'Test'

class SubModule(ConfigurableUser):
    __slots__ = { "IntA": CU,
                  "IntB": CU,
                  "IntC": CU,
                  "IntD": CU,
                  "LstA": [CU],
                  "LstB": [CU],
                  "LstC": [CU],
                  "LstD": [CU] }
    def getGaudiType( self ):
        return 'Test'

class SuperModule(ConfigurableUser):
    __slots__ = { "IntA": PU,
                  "IntB": PU,
                  "IntC": PU,
                  "IntD": PU,
                  "LstA": [PU],
                  "LstB": [PU],
                  "LstC": [PU],
                  "LstD": [PU] }
    __used_configurables__ = [ SubModule ]
    def __apply_configuration__(self):
        self.propagateProperties()
    def getGaudiType( self ):
        return 'Test'

pc = PlainConfigurable(IntA = CS, IntC = CS, LstA = [CS], LstC = [CS])
m = Master(IntA = PS, IntB = PS, LstA = [PS], LstB = [PS])

sub = SubModule(IntA = CS, IntC = CS, LstA = [CS], LstC = [CS])
sup = SuperModule(IntA = PS, IntB = PS, LstA = [PS], LstB = [PS])

# apply all ConfigurableUser instances
from GaudiKernel.Configurable import applyConfigurableUsers
applyConfigurableUsers()

from pprint import pprint
pprint(configurationDict())

def check(conf, prop, exp):
    v = conf.getProp(prop)
    good = v == exp
    if not good:
        print "ERROR:", 
    print "%s.%s is %r (expected %r)," % (conf.name(), prop, v, exp),
    if hasattr(conf,prop):
        print "set"
    else:
        print "unset"
    return good

good = check(pc, "IntA", PS) and \
       check(pc, "IntB", PS) and \
       check(pc, "IntC", CS) and \
       check(pc, "IntD", PU) and \
       check(sub, "IntA", PS) and \
       check(sub, "IntB", PS) and \
       check(sub, "IntC", CS) and \
       check(sub, "IntD", PU) and \
       check(pc, "LstA", [PS]) and \
       check(pc, "LstB", [PS]) and \
       check(pc, "LstC", [CS]) and \
       check(pc, "LstD", [PU]) and \
       check(sub, "LstA", [PS]) and \
       check(sub, "LstB", [PS]) and \
       check(sub, "LstC", [CS]) and \
       check(sub, "LstD", [PU])

if not good:
    sys.exit(1)
