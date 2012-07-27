#!/usr/bin/env python
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrate the extended job-properties and their       *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV ibelyaev@physics.syr.edu'
# =============================================================================
## @file
#  Simple example (identical to C++ ExtendedProperties.opts) which illustrates
#  the extended job-properties and their C++/Python intercommunication
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2007-02-13
# =============================================================================
from Gaudi.Configuration import *

from Configurables import ApplicationMgr

from Configurables import Gaudi__Examples__ExtendedProperties2 as EP2

# Flag to check if we are running on Windows, where LorentzVectors cannot be used as properties
import sys
isWin = sys.platform.startswith("win")

ep2 = EP2 (
    "xProps2"
    , Point3D = (10,40,55)
    , Points3D = [ (10,40,55) , (1,2,3) ]
    )

ApplicationMgr (
    EvtSel = "NONE"
    , TopAlg  = [ ep2 ]
    , EvtMax  = 10
    )

# =============================================================================
# The actual job excution
# =============================================================================
if '__main__' == __name__ :

    print __doc__ , __author__

    from GaudiPython.Bindings import AppMgr

    gaudi = AppMgr()

    gaudi.run(5)

    xp2 = gaudi.algorithm('xProps2')

    xp2.Point3D  = '(-10,3, Z : 24)'
    xp2.Vector3D = [-120,-30,-40]
    if not isWin:
        xp2.Vector4D = [-100,-200,-300,400]
        xp2.Vector4D = (-100,-200,-300,400)
        xp2.Vector4D = [(-100,-200,-300),400]
        xp2.Vector4D = [[-100,-200,-300],400]
        xp2.Vector4D = ((-100,-200,-300),400)
        xp2.Vector4D = ([-100,-200,-300],400)

    xp2.SVector5 = (1,2,3,4,5)

    xp2.SVector5 = [1,2,3,4,5]


    try :
        xp2.SVector5 = [1,2,3,4,5,6]
    except Exception, e :
        print ' Exception: ', e

    try :
        xp2.Point3D = (1,2,3,4)
    except Exception, e :
        print ' Exception: ', e

    if not isWin:
        try :
            xp2.Vector4D = (1,2,3)
        except Exception, e :
            print ' Exception: ', e


    xp2.Vectors3D = [ (1,2,3) , (4,5,6) , [7,8,9] ]
    if not isWin:
        xp2.Vectors4D = ( (1,2,3,4) , [4,5,6,7] )

    xp2.PropertiesPrint = True



# =============================================================================
# The END
# =============================================================================
