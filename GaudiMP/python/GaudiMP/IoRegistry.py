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
# @file GaudiMP.IoRegistry
# @purpose hold I/O registration informations
# @author Sebastien Binet <binet@cern.ch>

from FdsRegistry import FdsDict


class IoRegistry(object):
    """Singleton class to hold I/O registration and fds information"""
    instances = dict()  # { 'io-comp-name' : {'oldfname':'newfname',...},... }
    fds_dict = FdsDict()
    pass  # IoRegistry
