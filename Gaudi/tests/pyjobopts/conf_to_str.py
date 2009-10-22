from Gaudi.Configuration import *

MessageSvc().OutputLevel = INFO;

class MyAlg(ConfigurableUser):
    __slots__ = {
                  "OutputLevel" : INFO,
                  "Service": "DefaultService",
                  "Members": [ "HelloWorld" ]
                 }
    def getGaudiType(self):
        return "Test" # To avoid skipping of ConfigurableUser

class MySvc(ConfigurableUser):
    __slots__ = {
                  "OutputLevel" : INFO
                 }
    def getGaudiType(self):
        return "Test" # To avoid skipping of ConfigurableUser

mySvc = MySvc("JustAName", OutputLevel = VERBOSE)

myAlg1 = MyAlg("Algorithm1", Service = mySvc)

myAlg2 = MyAlg("Algorithm2",
                Members = [ "HelloWorld", myAlg1 ] )

myAlg3 = MyAlg("Algorithm3",
                Members = [ myAlg1, "HelloWorld", myAlg2 ] )
