#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from ast import literal_eval
from itertools import dropwhile, islice, takewhile
from subprocess import check_output

import pytest
from GaudiTesting import GaudiExeTest
from GaudiTesting.preprocessors import (
    RegexpReplacer,
    normalizeTestSuite,
)


@pytest.mark.ctest_fixture_required("gauditestsuite.ntuples")
@pytest.mark.shared_cwd("ntuples")
class Test(GaudiExeTest):
    command = [
        "gaudirun.py",
        "../../options/NTuples.opts",
        "--option",
        "ApplicationMgr(EvtMax=10).ExtSvc.append('Gaudi::MetaDataSvc')",
        "--option",
        "ApplicationMgr().ExtSvc.append('Gaudi::Monitoring::MessageSvcSink')",
        "--option",
        "from Configurables import Gaudi__MetaDataSvc; Gaudi__MetaDataSvc(OutputLevel=DEBUG)",
        "--option",
        "NTupleSvc().Output = [\"MyTuples DATAFILE='ntuple_with_metadata.root' OPT='NEW' TYP='ROOT'\"]",
    ]
    reference = "refs/MetaDataSvc.yaml"
    environment = ["GAUDIAPPNAME=", "GAUDIAPPVERSION="]

    preprocessor = normalizeTestSuite + RegexpReplacer(
        r"JobOptionsSvc\.SEARCHPATH:.*", "JobOptionsSvc.SEARCHPATH:..."
    )

    def test_metadata(self, stdout, cwd):
        metadata_in_stdout = filter(
            lambda x: x.strip(),
            takewhile(
                lambda x: x.strip(),
                islice(
                    dropwhile(
                        lambda l: "Metadata collected:" not in l,
                        stdout.decode().splitlines(),
                    ),
                    1,
                    None,
                ),
            ),
        )
        metadata_in_stdout = sorted(metadata_in_stdout)

        metadata_in_root = [
            f"{key}:{value}"
            for key, value in literal_eval(
                check_output(
                    ["dumpMetaData", cwd / "ntuple_with_metadata.root"], text=True
                )
            ).items()
        ]
        metadata_in_root.sort()

        assert metadata_in_stdout == metadata_in_root
