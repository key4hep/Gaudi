'''
Define common configuration for the StatusCodeSvc tests.
'''


def setup():
    from Gaudi.Configuration import VERBOSE
    from Configurables import (ApplicationMgr, GaudiExamplesCommonConf,
                               StatusCodeSvc, SCSAlg)

    GaudiExamplesCommonConf(DummyEvents=5)

    StatusCodeSvc(OutputLevel=VERBOSE)

    ApplicationMgr(TopAlg=[SCSAlg()])
