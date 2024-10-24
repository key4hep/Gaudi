#####################################################################################
# (c) Copyright 2021-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import pytest

# make sure asserts in GaudiExeTest and SubprocessBaseTest are enhanced by pytest
# (see https://docs.pytest.org/en/stable/how-to/writing_plugins.html#assertion-rewriting)
pytest.register_assert_rewrite("GaudiTesting.GaudiExeTest")
pytest.register_assert_rewrite("GaudiTesting.SubprocessBaseTest")

# allow from GaudiTesting import SKIP_RETURN_CODE
from .BaseTest import SKIP_RETURN_CODE  # noqa: F401

# allow from GaudiTesting import GaudiExeTest
from .GaudiExeTest import NO_ERROR_MESSAGES, GaudiExeTest  # noqa: F401

# allow from GaudiTesting import platformMatches
from .utils import platform_matches  # noqa: F401
