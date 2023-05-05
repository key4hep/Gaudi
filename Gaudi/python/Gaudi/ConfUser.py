#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiKernel.Configurable import ConfigurableUser


class GaudiPersistency(ConfigurableUser):
    """Configurable to enable ROOT-based persistency.

    Note: it requires Gaudi::RootCnvSvc (package RootCnv).
    """

    __slots__ = {}

    def __apply_configuration__(self):
        """Apply low-level configuration"""
        # aliased names
        from Configurables import (
            ApplicationMgr,
            EventPersistencySvc,
            FileCatalog,
            FileRecordDataSvc,
            IODataManager,
            PersistencySvc,
            RootCnvSvc,
        )

        cnvSvcs = [RootCnvSvc()]
        EventPersistencySvc().CnvServices += cnvSvcs
        PersistencySvc("FileRecordPersistencySvc").CnvServices += cnvSvcs
        app = ApplicationMgr()
        app.SvcOptMapping += [FileCatalog(), IODataManager(), RootCnvSvc()]
        app.ExtSvc += [FileRecordDataSvc()]
