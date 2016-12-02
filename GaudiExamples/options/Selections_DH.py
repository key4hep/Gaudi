#!/usr/bin/env gaudirun.py
# =============================================================================
from Gaudi.Configuration import ApplicationMgr
from Gaudi.Configuration import MessageSvc

from Configurables import Gaudi__Examples__SelCreateDH as SelCreate
from Configurables import Gaudi__Examples__SelReadDH as SelRead
from Configurables import EvtDataSvc

EvtDataSvc().ForceLeaves=True
ApplicationMgr(
    EvtMax = 1 ,
    EvtSel = 'NONE',
    TopAlg = [
    SelCreate ('Create1')  ,
    SelRead   ('Read1'),
    #SelFilter ('Filter1', Input = 'Create1' ) ,
    #SelFilter ('Filter2', Input = 'Filter1' ) ,
    #SelFilter ('Filter3', Input = 'Filter2' ) ,
    #SelFilter ('Filter4', Input = 'Filter3' ) ,
    #SelFilter ('Filter5', Input = 'Filter4' )
    ] )
