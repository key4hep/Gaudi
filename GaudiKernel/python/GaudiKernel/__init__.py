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
import os


def ROOT6WorkAroundEnabled(id=None):
    """
    Helper function to easily exclude ROOT6 work-arounds for testing.

    >>> os.environ['ROOT6_WORK_AROUND'] = 'all'
    >>> ROOT6WorkAroundEnabled()
    True
    >>> os.environ['ROOT6_WORK_AROUND'] = 'none'
    >>> ROOT6WorkAroundEnabled('JIRA-XYZ')
    False
    >>> os.environ['ROOT6_WORK_AROUND'] = 'JIRA-X'
    >>> ROOT6WorkAroundEnabled('JIRA-X')
    True
    >>> ROOT6WorkAroundEnabled('JIRA-Y')
    True
    >>> os.environ['ROOT6_WORK_AROUND'] = 'JIRA-X,-JIRA-Y'
    >>> ROOT6WorkAroundEnabled('JIRA-X')
    True
    >>> ROOT6WorkAroundEnabled('JIRA-Y')
    False
    >>> os.environ['ROOT6_WORK_AROUND'] = '-JIRA-Y'
    >>> ROOT6WorkAroundEnabled('JIRA-X')
    True
    >>> ROOT6WorkAroundEnabled('JIRA-Y')
    False
    """
    enabled = os.environ.get("ROOT6_WORK_AROUND", "all").lower()
    if enabled == "all":
        return True
    if enabled in ("none", "no", "off", "false", "0"):
        return False
    if id is None:  # unnamed work-arounds can only be disabled globally
        return True
    enabled = set(map(str.strip, enabled.split(",")))
    disabled = set([e[1:] for e in enabled if e.startswith("-")])
    id = id.lower()
    # the w-a is enabled if in the enabled list or not explicitly disabled
    return id in enabled or id not in disabled
