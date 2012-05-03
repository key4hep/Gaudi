Gaudi Intel Profiler
===============================================================================

Auditor for profilng Gaudi jobs. 
Based on [Intel® VTune™ Amplifier XE ][amplifier].




Unless IntelProfiler doesn't pushed to Gaudi source repository (svn) and you 
cann't use getpack command, you need to compile package by yourself at lxplus 
(on whenever cern afs is accesible and we can link it with intel amplifier 
static library):

[A video guide to installing profiler package][video].

[amplifier]: http://software.intel.com/en-us/articles/intel-vtune-amplifier-xe/
[video]: http://youtu.be/h9tx00MGZTQ



Run
--------------------------------------------------------------------------------

(For full project example see [IntelProfilerExample][example] project).

### Change Options File

Simple example of using IntelProfilerAuditor configurables:

```python
#!/usr/bin/env gaudirun.py
from Configurables import IntelProfilerAuditor
#...
profiler = IntelProfilerAuditor()
profiler.OutputLevel = DEBUG
# We can skip some events
profiler.StartFromEventN = 100
profiler.StopAtEventN = 500
# Sequence which we need to profile. If empty, we profile all algorithms
profiler.IncludeAlgorithms = ["TopSequence"]
AuditorSvc().Auditors +=  [profiler]
ApplicationMgr().AuditAlgorithms = True
```


### Setup Intel Environment

```sh
$> . /afs/cern.ch/sw/IntelSoftware/linux/all-setup.sh
```

The script change  environment variables:

  * update `PATH`.
  * set `INTEL_LICENSE_FILE`.

### Collect Profiling Information
```sh
$> intelprofiler -o ~/profiler myoptions.py
```

, where ```~/profiler``` is a directorory where a profiling database will be 
stored. After the each run of a profiling job you will get directories 
like ~/profiler/r000hs, ~/profiler/r0001hs, ~/profiler/r_RunNumber_hs.

If script could not find intel `amplxe-cl` command, 
it will try to setup intel environment (see previous section).

* P.S. At lxplus during the profiling you can see the following messages:
  
   ```
   sh: lsmod: command not found
   ERROR: ld.so: object '/$LIB/snoopy.so' from /etc/ld.so.preload cannot be preloaded: ignored.
   ERROR: ld.so: object '/$LIB/snoopy.so' from /etc/ld.so.preload cannot be preloaded: ignored.
   ```
   You can ignore it.

Analyze
--------------------------------------------------------------------------------
### With GUI

(setup intel environment)

```sh
$> amplxe-gui ~/profiler/r000hs
```

An argument is a path to profiling database.

### From Command Line

```sh
 $> amplxe-cl -report hotspots -r ~/profiler/r000hs
 $> amplxe-cl -report hotspots -r ~/profiler/r000hs -group-by=task
 $> amplxe-cl -report hotspots -r ~/profiler/r000hs -group-by=task-function
```

* [Intel Amplifier documentation][amplxe].
* [Documentation about group-by opiton][groupby].
* [How to interpret results][interpret].



[groupby]: http://software.intel.com/sites/products/documentation/hpc/amplifierxe/en-us/lin/ug_docs/olh/cli_ref/group-by.html#group-by
[amplxe]: http://software.intel.com/sites/products/documentation/hpc/amplifierxe/en-us/lin/ug_docs/index.htm
[interpret]: http://software.intel.com/sites/products/documentation/hpc/amplifierxe/en-us/lin/ug_docs/index.htm
[example]: https://github.com/mazurov/IntelProfilerExample



