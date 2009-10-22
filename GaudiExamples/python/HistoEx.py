#!/usr/bin/env python
# =============================================================================
# $Id: HistoEx.py,v 1.6 2007/10/29 14:29:47 mato Exp $
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrate the usage of useful algorithm  base class   *
* HistoAlgo (python version of C++ GaudiHistoAlg) for "easy" histogramming.   *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr'
# =============================================================================

import GaudiPython

from   GaudiPython.GaudiAlgs   import HistoAlgo

SUCCESS = GaudiPython.SUCCESS

# =============================================================================
# Simple algorithm which book&fill 3 histograms
# =============================================================================
class HistoEx(HistoAlgo) :
    """ Simple algorithm which implicitely book&fill three histograms """
    def __init__ ( self , name ) :
        """ Constructor """
        HistoAlgo.__init__( self , name )

    def execute( self ) :
        """ The major method 'execute', it is invoked for each event """
        for i in range(0,10) :
            self.plot1D( i
                         , ' 1D histo '
                         , 0 , 20 , 20 )
            for j in range(0,10) :
                self.plot2D( i   , j
                             ,' 2D histo '
                             ,  0 , 20
                             ,  0 , 20
                             , 20 , 20 )
                for k in range(0,10) :
                    self.plot3D( i , j , k
                                 , ' 3D histo '
                                 ,  0 , 20
                                 ,  0 , 20
                                 ,  0 , 20
                                 , 20 , 20 , 20 )

        return SUCCESS


# =============================================================================
# job configuration
# =============================================================================
def configure( gaudi = None  ) :
    """ Configuration of the job """

    if not gaudi : gaudi = GaudiPython.AppMgr()

    gaudi.JobOptionsType       = 'NONE'
    gaudi.EvtSel               = 'NONE'
    gaudi.HistogramPersistency = 'ROOT'

    gaudi.config()

    gaudi.DLLs = [ 'GaudiAlg', 'RootHistCnv', ]

    alg = HistoEx('HistoEx')
    gaudi.setAlgorithms( [alg] )
    alg.HistoPrint = True

    hsvc = gaudi.service('HistogramPersistencySvc')
    hsvc.OutputFile = "histo1.root"

    # This does not harm and tests bug #50389
    getMyalgBack = gaudi.algorithm ( 'HistoEx' )

    return SUCCESS


# =============================================================================
# The actual job excution
# =============================================================================
if '__main__' == __name__ :
    print __doc__ , __author__
    gaudi = GaudiPython.AppMgr()
    configure( gaudi )
    gaudi.run(20)
