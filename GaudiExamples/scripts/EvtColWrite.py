#!/usr/bin/env python
# =============================================================================
# $Id: EvtColWrite.py,v 1.11 2007/10/29 14:29:47 mato Exp $
# =============================================================================
# CVS tag $Name:  $, version $Revision: 1.11 $
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrate the usage of useful                         *
* algorithm  base class for writing of EventTag Collections                   *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV ibelyaev@physics.syr.edu'
# =============================================================================

import GaudiPython, math

Rndm    = GaudiPython.gbl.Rndm
Numbers = Rndm.Numbers
SUCCESS = GaudiPython.SUCCESS

from   GaudiPython.GaudiAlgs   import TupleAlgo

# =============================================================================
# Simple algorithm which book&fill 3 histograms
# =============================================================================
class EvtColEx(TupleAlgo) :
    """ Simple algorithm which implicitely book&Fill Event Tag collection"""
    def __init__ ( self , name = 'EvtColEx' ) :
        """ Constructor """
        TupleAlgo.__init__( self , name )
        self.s_nEvt = 0
        self.s_nRun = 0


    def execute( self ) :
        """ The major method 'execute', it is invoked for each event """

        self.s_nEvt += 1
        if 1 == self.s_nEvt % 50 : self.s_nRun += 1

        rSvc    = self.randSvc()

        gauss   = Numbers ( rSvc , Rndm.Gauss        (   0.0 ,   1.0  ) )
        flat    = Numbers ( rSvc , Rndm.Flat         ( -10   ,  10    ) )
        expo    = Numbers ( rSvc , Rndm.Exponential  (   1.0          ) )
        breit   = Numbers ( rSvc , Rndm.BreitWigner  (   0.0 ,   1.0  ) )
        poisson = Numbers ( rSvc , Rndm.Poisson      (   2.0          ) )
        binom   = Numbers ( rSvc , Rndm.Binomial     (   8   ,   0.25 ) )

        address = self.get('/Event')
        address = address.registry().address()

        # get the event tag collection itself
        tup = self.evtCol( 'COL1' , 'My trivial N-tuple' )

        tup.column ( 'Address' , address     )

        tup.column ( 'evtNum'  , self.s_nEvt )
        tup.column ( 'runNum'  , self.s_nRun )

        tup.column ( 'gauss'   ,       gauss   ()   )
        tup.column ( 'flat'    ,       flat    ()   )
        tup.column ( 'expo'    ,       expo    ()   )
        tup.column ( 'poisson' , int ( poisson () ) )
        tup.column ( 'binom'   , int ( binom   () ) )
        tup.column ( 'flag'    ,   0 > gauss   ()   )

        return SUCCESS


def _evtcolsvc_(self, name='EvtTupleSvc') :
    svc = GaudiPython.Helper.service( self._svcloc, name, False )
    return GaudiPython.iNTupleSvc(name, svc)

GaudiPython.AppMgr.evtcolsvc = _evtcolsvc_

# =============================================================================
# job configuration
# =============================================================================
def configure( gaudi = None  ) :
    """ Configuration of the job """

    if not gaudi : gaudi = GaudiPython.AppMgr()

    gaudi.HistogramPersistency = "ROOT"
    gaudi.DLLs   += [ 'GaudiAlg'  , 'RootHistCnv'     ]
    gaudi.ExtSvc += [ 'RndmGenSvc'    ,
                      'NTupleSvc'      ,
                      'TagCollectionSvc/EvtTupleSvc' ]

    alg = EvtColEx('Fill')
    gaudi.setAlgorithms( [alg] )

    alg.EvtColLUN       = 'EVTTAGS'
    alg.EvtColsProduce  = True
    alg.EvtColsPrint    = True
    alg.NTupleProduce   = False
    alg.HistoProduce    = False

    gaudi.OutStream = [ 'EvtCollectionStream/TagsWriter']
    tagsWriter = gaudi.algorithm('TagsWriter')
    tagsWriter.ItemList    = [ '/NTUPLES/EVTTAGS/Fill/COL1']
    tagsWriter.EvtDataSvc  =   "EvtTupleSvc" ;

    evtColSvc = gaudi.evtcolsvc()
    evtColSvc.defineOutput( {'EVTTAGS' : 'PFN:EvtTags1.root'} , typ='Gaudi::RootCnvSvc' )
    evtColSvc.OutputLevel = 2

    evtSel = gaudi.evtSel()
    evtSel.PrintFreq = 1000
    evtSel.open( ['EvtColsEx.dst'] )

    return SUCCESS


# =============================================================================
# The actual job excution
# =============================================================================
if '__main__' == __name__ :
    print __doc__
    # configuration (options)
    from Configurables import GaudiPersistency, FileCatalog, ApplicationMgr
    GaudiPersistency()
    ApplicationMgr().ExtSvc.append(FileCatalog(Catalogs=['xmlcatalog_file:EvtColsEx.xml']))
    # execution
    gaudi = GaudiPython.AppMgr()
    configure( gaudi )
    gaudi.run(10000)
    gaudi.exit()
# =============================================================================
# The END
# =============================================================================
