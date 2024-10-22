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
"""
## Use Case
Prevent jobs to never end because of a problem in an algorithm

## Description
Sometimes, because of a bug, an algorithm enters an infinite loop or a dead lock
occurs. In these cases the application will never terminate.

## Solution
To prevent such cases, in particular for batch jobs that could waste a lot of
resources before the problem is detected, it is possible to use the special algorithm
`Gaudi::EventWatchdogAlg`.

`Gaudi::EventWatchdogAlg` starts a secondary thread that sleeps until a timeout is
reached. At that point it prints a warning message and optionally a stack trace of
the process on stderr, then it sleeps for another timeout period unless it's
configured to abort the process when the timeout occurs.
"""

# Run this configuration with `gaudirun.py <path_to_this_file>:config`

from GaudiConfig2 import Configurable
from GaudiConfig2 import Configurables as C


def base_config() -> list[Configurable]:
    """
    Example configuration of a job with no input and a algorithm that looks stuck.
    """
    algorithms = [C.GaudiTesting.SleepyAlg("StuckAlg", SleepTime=3600)]
    app = C.ApplicationMgr(
        EvtSel="NONE", TopAlg=[C.Gaudi.Sequencer("MainSequence", Members=algorithms)]
    )
    return [app] + list(app.TopAlg) + algorithms


def add_event_timeout(
    conf: list[Configurable], timeout_seconds: int
) -> list[Configurable]:
    """
    Take a configuration and adds a check on events reaching a timeout.
    """
    # find the ApplicationMgr as we have to tweak its configuration
    app = next(c for c in conf if c.name == "ApplicationMgr")
    # configure a Gaudi::EventWatchdogAlg
    watchdog = C.Gaudi.EventWatchdogAlg(
        EventTimeout=timeout_seconds,  # sleep for this number of seconds
        StackTrace=True,  # print a stack trace when we wake up
        AbortOnTimeout=True,  # kill the process on timeout
    )
    # wrap original list of algorithms into a sequence to ensure the watchdog
    # is executed before everything else
    wrapping_seq = C.Gaudi.Sequencer(
        "SequenceWithTimeout", Sequential=True, Members=[watchdog] + list(app.TopAlg)
    )
    # reset the main list of algorithms
    app.TopAlg = [wrapping_seq]
    # return the tweaked configuration
    return conf + [watchdog, wrapping_seq]


def config():
    # get the normal job configuration
    conf = base_config()
    # make sure we stop if an event takes more than 2s
    conf = add_event_timeout(conf, 2)
    return conf
