These data files are taken from running the Tier0ChainTest q431 with RawToAll
enabled.

Source code was from the athena master branch at a3486f95b1d0c35932b97b4dafcf641a12f69b2d

Timing was performed on aibuild028.cern.ch:
   x86_slc6-gcc8-opt
   Intel(R) Xeon(R) CPU E5-2650 v2 @ 2.60GHz

Datafiles:
  data17_13TeV.00330470.physics_Main.daq.RAW._lb0310._SFO-1._0001.data
  data18_13TeV.00364485.physics_Main.daq.RAW._lb0720._SFO-1._0001.data (high mu)


Explicit data depedencies for StreamESD and StreamAOD Algorithms were extracted by
examining data objects written to output root files.


 file manifest
===============

dataDep.dat        : explicit data dependencies
cf.r2a.graphml     : control flow rules
df.r2a.graphml     : data flow rules
time.r2a.json      : timing data for regular datafile (seconds)
time.r2a.himu.json : timing data for high-mu datafile (seconds)
