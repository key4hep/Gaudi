###############################################################
# Job options file
#==============================================================

from Gaudi.Configuration import *
from Configurables import TAlgIS, TAlgDB, TemplatedAlg_int_std__vector_std__string_std__allocator_std__string_s_s_,TemplatedAlg_double_bool_

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

is1   = TemplatedAlg_int_std__vector_std__string_std__allocator_std__string_s_s_( 'TAlgIS1', TProperty = 100, RProperty = ['string1', 'string2'] )
is2   = TAlgIS( 'TAlgIS2', TProperty = 100, RProperty = ['string1', 'string2'] )
db1   = TemplatedAlg_double_bool_( 'TAlgDB1', TProperty = 10.10, RProperty = True )
db2   = TAlgDB( 'TAlgDB2', TProperty = 10.10, RProperty = True )

#-----------------------------------------------------------------
ApplicationMgr( TopAlg = [is1, is2, db1, db2 ],
                EvtMax = 10,     # events to be processed (default is 10)
                EvtSel = 'NONE', # do not use any event input
                )

