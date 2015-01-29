# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.options="""
from Gaudi.Configuration import *
from Configurables import Gaudi__ParticlePropertySvc as ParticlePropertySvc
from Configurables import GaudiExamples__GaudiPPS as GaudiPPS

importOptions("Common.opts")

ParticlePropertySvc( ParticlePropertiesFile = "../data/ParticleTable.txt" )

ApplicationMgr( EvtMax = 1,
                EvtSel = 'NONE',
                TopAlg = [GaudiPPS()] )
"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        expected = "Opened particle properties file : ../data/ParticleTable.txt"
        if not expected in stdout:
            causes.append("missing signature")
            result["GaudiTest.expected_line"] = result.Quote(expected)
        if 'ERROR' in stdout or 'FATAL' in stdout:
            causes.append("error")