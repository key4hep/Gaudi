#!/usr/bin/env gaudirun.py
# =============================================================================
from Gaudi.Configuration import ApplicationMgr 

from Configurables import Gaudi__Examples__SelCreate as SelCreate 
from Configurables import Gaudi__Examples__SelFilter as SelFilter 


ApplicationMgr(
    EvtMax = 20 ,
    EvtSel = 'NONE',
    TopAlg = [
    SelCreate ('Create1')  ,
    SelFilter ('Filter1', Input = 'Create1' ) ,
    SelFilter ('Filter2', Input = 'Filter1' ) ,
    SelFilter ('Filter3', Input = 'Filter2' ) ,
    SelFilter ('Filter4', Input = 'Filter3' ) ,
    SelFilter ('Filter5', Input = 'Filter4' ) 
    ] )
