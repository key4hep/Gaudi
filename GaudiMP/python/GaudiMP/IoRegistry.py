## @file GaudiMP.IoRegistry
## @purpose hold I/O registration informations
## @author Sebastien Binet <binet@cern.ch>

from FdsRegistry import FdsDict

class IoRegistry (object):
    """Singleton class to hold I/O registration and fds information"""
    instances = dict() # { 'io-comp-name' : {'oldfname':'newfname',...},... }
    fds_dict = FdsDict()
    pass # IoRegistry
