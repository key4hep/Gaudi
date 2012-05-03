#!/usr/bin/env python
# =============================================================================
# $Id: EvtColRead.py,v 1.10 2008/01/30 17:06:51 marcocle Exp $
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrate the usage of useful                         *
* algorithm  base class for N-Tuple manipulations                             *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr'
# =============================================================================

import GaudiPython, math

SUCCESS = GaudiPython.SUCCESS

# =============================================================================
# job configuration
# =============================================================================
def configure( gaudi = None  ) :
    """ Configuration of the job """

    gaudi.HistogramPersistency = 'ROOT'

    gaudi.DLLs   += [ 'GaudiAlg'  , 'RootHistCnv', ]
    gaudi.ExtSvc += [ 'TagCollectionSvc/EvtTupleSvc' ]

    evtSel = gaudi.evtSel()
    evtSel.open( 'PFN:EvtTags1.root'      ,
                 collection = 'Fill/COL1' ,
                 sel = 'binom==6'         )

    gaudi.config()

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
    gaudi.run(-1)
    gaudi.exit()
