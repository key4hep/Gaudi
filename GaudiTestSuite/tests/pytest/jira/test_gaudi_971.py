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
import pytest
from GaudiTesting import GaudiExeTest
from GaudiTesting.preprocessors import RegexpReplacer, normalizeTestSuite


@pytest.mark.ctest_fixture_required("root_io_base")
@pytest.mark.shared_cwd("root_io")
class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/ROOT_IO/Read.py"]

    def options(self):
        # Options of the test job
        # modify the ROOTIO.xml file catalog to add a dummy replica
        from xml.etree import ElementTree as ET

        from Gaudi.Configuration import (
            EventSelector,
            FileCatalog,
        )

        data = ET.parse("ROOTIO.xml")
        for file_el in data.getroot().iterfind("File"):
            for pfn in file_el.iterfind("physical/pfn"):
                if pfn.attrib["name"] == "ROOTIO.dst":
                    physical = file_el.find("physical")
                    dummy = physical.makeelement(
                        "pfn", {"name": "dummy.dst", "filetype": "ROOT"}
                    )
                    physical.insert(0, dummy)
                    logical = file_el.find("logical")
                    if logical is None:
                        logical = file_el.makeelement("logical")
                        file_el.append(logical)
                    lfn = logical.makeelement("lfn", {"name": "input.dst"})
                    logical.append(lfn)
                    break

        with open("GAUDI-971.xml", "wb") as catalog:
            catalog.write(b'<?xml version="1.0" encoding="utf-8" standalone="no" ?>\n')
            catalog.write(b'<!DOCTYPE POOLFILECATALOG SYSTEM "InMemory">\n')
            data.write(catalog, encoding="utf-8", xml_declaration=False)

        # print('#' * 80)
        # print(open('GAUDI-971.xml').read())
        # print('#' * 80)

        FileCatalog(Catalogs=["xmlcatalog_file:GAUDI-971.xml"])

        esel = EventSelector()
        esel.Input = [
            "DATAFILE='LFN:input.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"
        ]

    reference = "../refs/jira/gaudi_971.yaml"

    preprocessor = normalizeTestSuite + RegexpReplacer(
        when="TFile::TFile",
        orig=r"file (.*)dummy.dst does not exist",
        repl=r"file .../dummy.dst does not exist",
    )
