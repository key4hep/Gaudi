#!/usr/bin/env python
# =============================================================================
# $Id:$ 
# =============================================================================
# @file Mix.py
# Simple example to illustrate the problem for task #13911
# @author Vanya BELYAEV Ivan.Belyaev@itep.ru
# @date   2010-04-24
# =============================================================================
"""
Simple example to illustrate the problem for task #13911
https://savannah.cern.ch/task/?13911
"""
# =============================================================================
__author__  = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
__version__ = "CVS tag $Name:$, version $Revision:$"
__date__    = "2010-04-24"
# =============================================================================

from Gaudi.Configuration   import *
from Configurables         import HelloWorld, GaudiSequencer, Sequencer

from GaudiPython.Bindings  import AppMgr, setOwnership  
from GaudiPython.Bindings  import gbl as cpp 
from GaudiPython.GaudiAlgs import GaudiAlgo, SUCCESS

# =============================================================================
## @class SimpleAlgo
#  very simple (empty) python algorith
# @author Vanya BELYAEV Ivan.Belyaev@itep.ru
# @date   2010-04-24
class SimpleAlgo(GaudiAlgo) :

    def execute ( self ) :
        
        print 'I am SimpleAlgo.execute!  ', self.name()
        
        return SUCCESS
    
# =============================================================================
# configure the application :
def configure() :
        
    importOptions('Common.opts')
    
    ApplicationMgr (
        TopAlg = [
        HelloWorld() ,
        GaudiSequencer ( 'MySequencer'      ,
                         MeasureTime = True , 
                         Members = [ HelloWorld ('Hello1') ,
                                     HelloWorld ('Hello2') ] )
        ] , 
        # do not use any event input
        EvtSel = 'NONE'
        )

    
    gaudi = AppMgr()
    
    ## create two "identical" algorithms:
    
    myAlg1 = SimpleAlgo ( 'Simple1' )
    myAlg2 = SimpleAlgo ( 'Simple2' )

    ## Adding something into TopAlg-sequence is OK:
    gaudi.setAlgorithms( [ myAlg1 ] + gaudi.TopAlg )
    
    ## Extending of "other"-sequences causes failures: 
    seq = gaudi.algorithm('MySequencer')    

    seq.Members += [ 'HelloWorld/Hello3']   ## it is ok 
    seq.Members += [ myAlg2.name() ]        ## it fails 
    
    cpp.StatusCode.enableChecking ()
    

if '__main__' == __name__ :

    ## make printout of the own documentations 
    print '*'*120
    print                      __doc__
    print ' Author  : %s ' %   __author__    
    print ' Version : %s ' %   __version__
    print ' Date    : %s ' %   __date__
    print '*'*120

    configure()

    gaudi = AppMgr()

    gaudi.run ( 4 ) 

    # add 'late' algorithms
    
    myAlg3 = SimpleAlgo ( 'Simple3' )
    
    seq = gaudi.algorithm('MySequencer')    
    seq.Members += [ 'HelloWorld/Hello4']   

    gaudi.run ( 8 ) 

    seq.Members += [ myAlg3.name() ]        

    gaudi.run ( 3 ) 


# =============================================================================
# The END 
# =============================================================================
