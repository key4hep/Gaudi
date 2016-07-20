Profiling Gaudi jobs with Jemalloc {#profiling-jemalloc}
===============================================================================

Integration of Jemalloc within Gaudi jobs.
Uses the Jemalloc library from http://www.canonware.com/jemalloc/

It is possible to profile the memory used by Gaudi jobs, using the jemalloc library
as documented in https://github.com/jemalloc/jemalloc/wiki

To run the profiler, it is necessary to:
  * preload libjemalloc.so, using the LD_PRELOAD environment variable
  * set the MALLOC_CONF environment variable to enable profiling

gaudirun.py has been updated to set the environment accordingly (a prerequisite is however
that libjemalloc.so has to be available in the library path).

A Gaudi algorithm has also been developed to perform memory heap dumps at various event,
and is configured using the StartFromEventN, StopAtEventN and DumpPeriod, as described
in the example below.

A Gaudi service is also available to provide the same functionality, with the advantage
of being able to profile without modifying the algorithm sequence.

Running with the JemallocProfile algorithm
--------------------------------------------------------------------------------

### Change Options File

Simple example of using the JemallocProfile algorithm in a Gaudi configurable:

~~~~~~~~{.py}
#!/usr/bin/env gaudirun.py
from Configurables import JemallocProfile
#...
jp = JemallocProfile()
jp.StartFromEventN = 49
jp.StopAtEventN = 99
jp.DumpPeriod = 10

GaudiSequencer("PhysicsSeq").Members += [ jp ]
~~~~~~~~

### Run the job

~~~~~~~~{.sh}
$> gaudirun.py --profilerName=jemalloc --run-info-file=runinfo.json myoptions.py
~~~~~~~~

Please note the the `--profilerName=jemalloc` to enbale the profiling, and the `--run-info-file` that produces
a file containing information useful to interpret the results (process id of the Gaudi job, and the absolute path
of the executable, necessary to run the jeprof analysis tool).

Running with the JemallocProfileSvc service
--------------------------------------------------------------------------------

### Change Options File

Simple example of using the JemallocProfileSvc service in a Gaudi configurable:

~~~~~~~{.py}
#!/usr/bin/env gaudirun.py
from Configurables import JemallocProfileSvc
from Gaudi.Configuration import DEBUG
#...
jps = JemallocProfileSvc(StartFromEventN=3,
                         StopAtEventN=7,
                         OutputLevel=DEBUG)
ApplicationMgr().ExtSvc.append(jps)
~~~~~~~

It is also possible to trigger the profiling using incidents:

~~~~~~~{.py}
from Configurables import JemallocProfileSvc
jps = JemallocProfileSvc(StartFromIncidents=['MyStartIncident1',
                                             'MyStartIncident2'],
                         StopAtIncidents=['MyStopIncident'])
ApplicationMgr().ExtSvc.append(jps)
~~~~~~~

### Run the job
This is identical as for the algorithm version.


Analyze
--------------------------------------------------------------------------------
### With text output

The jeprof analysis tool from the Google performances tools (http://goog-perftools.sourceforge.net/)
is necessary to analyze the heap files.

It can be used to compare the memory in two heap files in the following way:

~~~~~~~~{.sh}
$> jeprof -text --base=<firstheap>.heap <executable name> <comparewith>.heap
~~~~~~~~

### To produce a postscript file

~~~~~~~~{.sh}
$> jeprof -gv --base=<firstheap>.heap <executable name> <comparewith>.heap
~~~~~~~~
