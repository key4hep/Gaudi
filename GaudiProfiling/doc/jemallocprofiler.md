Profiling Gaudi jobs with Jemalloc
===============================================================================

Integration of Jemalloc within gaudi jobs.
Uses the Jemalloc library from http://www.canonware.com/jemalloc/

It is possible to profile the memory used by Gaudi jobs, using the jemalloc library 
as documented in https://github.com/jemalloc/jemalloc/wiki

To run the profiler, it is necessary to:
  * preload libjemalloc.so, using the LD_PRELOAD environment variable
  * set the MALLOC_CONF environment variable to enable profiling

gaudirun.py has been updated to set the environment accordingly (a prerequisite is however
that libjemalloc.so has to be available in the library path).

A Gaudi algorithm has also been developped to perform memory heap dumps at various event,
and is configured using the StartFromEventN, StopAtEventN and DumpPeriod, as described 
in the example below.

Run
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
$> lb-run Brunel vXrY gaudirun.py --profilerName=jemalloc --run-info-file=runinfo.json myoptions.py 
~~~~~~~~

Please note the the --profilerName=jemalloc to enbale the profiling, and the run-info-file that produces
a file containing information useful to interpret the results (process id of the Gaudi job, and the absolute path
of the executable, necessary to run the pprof analysis tool).

Analyze
--------------------------------------------------------------------------------
### With text output

The pprof analysis tool from the Google performances tools (http://goog-perftools.sourceforge.net/)
is necessary to analyze the heap files.

It can be used to comapre the memory in two heap files in the following way:

~~~~~~~~{.sh}
$>  pprof -text --base=<firstheap>.heap <executable name> <comparewith>.heap
~~~~~~~~

### To produce a postscript file

~~~~~~~~{.sh}
$>  pprof -gv --base=<firstheap>.heap <executable name> <comparewith>.heap
~~~~~~~~


