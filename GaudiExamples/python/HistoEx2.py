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

import GaudiPython

from   GaudiPython.GaudiAlgs   import HistoAlgo

Rndm    = GaudiPython.gbl.Rndm
SUCCESS = GaudiPython.SUCCESS 

# =============================================================================
# Simple algorithm which book&fill two profile histograms
# =============================================================================
class HistoEx2 (HistoAlgo) :
    """ Simple algorithm which explicitely book&fill profile histograms """
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

    if not gaudi : gaudi = GaudiPython.AppMgr()

    HistoEx1.configure( gaudi ) 

    alg = HistoEx2('HistoEx2')
    gaudi.addAlgorithm( alg )
    
    alg.HistoPrint = True
    
    return SUCCESS


# =============================================================================
# The actual job excution 
# =============================================================================
if '__main__' == __name__ :
    print __doc__ , __author__
    gaudi = GaudiPython.AppMgr()
    configure( gaudi )
    gaudi.run(20)

    
    
# =============================================================================
# The END
# =============================================================================
