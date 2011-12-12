#!/usr/bin/env python
# =============================================================================
# $Id: HistoEx2.py,v 1.2 2007/10/29 14:29:47 mato Exp $
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrate the usage of useful algorithm  base class   *
* HistoAlgo (python version of C++ GaudiHistoAlg) for 'easy' histogramming.   *
*                                                                             *
* This example illustrates the usage of 1D and 2D profile histograms          *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV ibelyaev@physics.syr.edu'
# =============================================================================
import os

from GaudiPython.GaudiAlgs   import HistoAlgo, SUCCESS 

from GaudiPython.Bindings    import gbl as cpp 
Rndm    = cpp.Rndm

# =============================================================================
# Simple algorithm which book&fill two profile histograms
# =============================================================================
class HistoEx2 (HistoAlgo) :
    """ Simple algorithm which explicitly book&fill profile histograms """
    def __init__ ( self , name = 'HistoEx2' ) :
        """ Constructor """
        HistoAlgo.__init__( self , name )

    def execute( self ) :
        """ The major method 'execute', it is invoked for each event """

        gauss =  Rndm.Numbers ( self.randSvc()       ,
                                Rndm.Gauss ( 0 , 1 ) )
        
        for i in range(0,10000) :
            x = gauss ()
            y = gauss ()
            self.plot2D    ( x , y     , ' x vs y    ' , -2 , 2 , -4 , 4 )
            self.plot2D    ( x , y+3*x , ' x vs y+3x ' , -2 , 2 , -4 , 4 )
            self.plot2D    ( x , y-3*x , ' x vs y-3x ' , -2 , 2 , -4 , 4 )
            self.profile1D ( x , y     , ' x vs y    (profile)' , -2 , 2 )
            self.profile1D ( x , y+3*x , ' x vs y+3x (profile)' , -2 , 2 )
            self.profile1D ( x , y-3*x , ' x vs y-3x (profile)' , -2 , 2 )
        return SUCCESS
    

# =============================================================================
# job configuration 
# =============================================================================
def configure( gaudi = None  ) :
    """ Configuration of the job """
    
    import HistoEx1

    if not gaudi :
        from GaudiPython.Bindings import AppMgr
        gaudi = AppMgr()

    HistoEx1.configure( gaudi ) 

    alg = HistoEx2('HistoEx2')
    gaudi.addAlgorithm( alg )
    
    alg.HistoPrint = True
    
    return SUCCESS


# =============================================================================
# The actual job execution 
# =============================================================================
if '__main__' == __name__ :
    print __doc__ , __author__

    from GaudiPython.Bindings import AppMgr
    import GaudiPython.HistoUtils
    
    gaudi = AppMgr()
    configure( gaudi )

    gaudi.run(20)

    # Skip the next part when running within QMTest
    for alg in ( 'HistoEx'  ,
                 'HistoEx1' ,
                 'HistoEx2' ) :
        alg = gaudi.algorithm ( alg )
        histos = alg.Histos()
        for key in histos :
            histo = histos[key]
            print " Alg='%s', ID='%s' , Histo=%s " % ( alg.name() , key , histo ) 
            if hasattr ( histo , 'dump' ) :
                print histo.dump ( 60 , 20 , True)
        
