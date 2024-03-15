#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
###############################################################
# Job options file
# ==============================================================

from Configurables import (
    GaudiTestSuiteCommonConf,
    TAlgDB,
    TAlgIS,
    TemplatedAlg_double_bool_,
    TemplatedAlg_int_std__vector_std__string_std__allocator_std__string_s_s_,
)
from Gaudi.Configuration import *

GaudiTestSuiteCommonConf()

is1 = TemplatedAlg_int_std__vector_std__string_std__allocator_std__string_s_s_(
    "TAlgIS1", TProperty=100, RProperty=["string1", "string2"]
)
is2 = TAlgIS("TAlgIS2", TProperty=100, RProperty=["string1", "string2"])
db1 = TemplatedAlg_double_bool_("TAlgDB1", TProperty=10.10, RProperty=True)
db2 = TAlgDB("TAlgDB2", TProperty=10.10, RProperty=True)

# -----------------------------------------------------------------
ApplicationMgr(
    TopAlg=[is1, is2, db1, db2],
    EvtMax=10,  # events to be processed (default is 10)
    EvtSel="NONE",  # do not use any event input
)
