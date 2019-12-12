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
from __future__ import print_function
# File to prepare the configs and commands to be executed for the benchmark

import commands


def prepareConfig(template_filename,
                  n_threads=10,
                  n_parallel_events=10,
                  n_parallel_algorithms=10):
    template = open(template_filename)
    new_filename = "%s_%i_%i_%i.py" % (template_filename.rstrip(".py"),
                                       n_threads, n_parallel_events,
                                       n_parallel_algorithms)
    new_config = open(new_filename, "w")
    for line in template.readlines():
        if line.startswith("n_threads"):
            line = "n_threads = %i\n" % n_threads
        elif line.startswith("n_parallel_events"):
            line = "n_parallel_events = %i\n" % n_parallel_events
        elif line.startswith("n_parallel_algorithms"):
            line = "n_parallel_algorithms = %i\n" % n_parallel_algorithms
        new_config.write(line)
    new_config.close()
    return new_filename


##########################
if __name__ == "__main__":

    n_threads = 10
    for n_algos in range(1, 11):
        for n_events in range(1, n_algos + 1):
            config = prepareConfig(
                "../options/BrunelScenario.py",
                n_threads=n_threads,
                n_parallel_events=n_events,
                n_parallel_algorithms=n_algos)
            # config.replace(".py",".log"))
            print("/usr/bin/time -f %%S -o %s.time `alias gaudirun` %s > %s" %
                  (config.replace(".py", ""), config, "/dev/null"))
