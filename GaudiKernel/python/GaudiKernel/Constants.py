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
# File: AthenaCommon/python/Constants.py
# Author: Wim Lavrijsen (WLavrijsen@lbl.gov)
"""Useful/conventional constants."""

__all__ = [
    "ALL",
    "VERBOSE",
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL",
    "TRUE",
    "FALSE",
]

# message levels -----------------------------------------------------------
ALL = 0
VERBOSE = 1
DEBUG = 2
INFO = 3
WARNING = 4
ERROR = 5
FATAL = 6

# for job options legacy (TODO: get rid of these!) -------------------------
TRUE = True
FALSE = False

# message to notify folks that they really have to fix their codes:
error_explanation = """
     ==> After this line, a hack will be executed, attempting to rectify the
     ==> problem just reported. Since this hack may fail, and since it is a
     ==> rather temporary measure hack, this is an ERROR, not a WARNING."""
