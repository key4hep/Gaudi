Profile GaudiHive

Different scripts are available to ease the performance measurements of the GaudiHive application.

o BrunelWrapper.py
  Poor's man cmsDriver (for the CMS connaisseurs, a script to crank configuration files using a
command line tool for the others) that "sed"s the content of the BrunelScenario.py configuration.
Has a nice -h that shows the options: change algos in flight, thread pool size,  events in flight,
verbosity via command line. Dumps command line. Must be called from options directory.
Use it with bash/python scripts to generate your configurations in bulk.

o plotBacklogPyRoot.py
  Plots the backlog starting from 2 outputs, intended to be w/ and w/o cloning. Color-blind,
ROOT standard plot with a TLatex to explain the evts in flight red line.
It has a usage that is displayed. In the file itself one finds some details about the
implementation (very trivial: parse, fill a list of lists, fill a graph with it and maquillage)

o plotClonesPyRoot.py
  Plots the number of clones as a function of the time spent in the algorithm starting from 1
single output. Color-blind, ROOT std plot with tlatex-es showing how many algos there are in
total and how many per # of clones.

o plotSpeedupsPyRoot.py
 Plots the speedup wrt the scalar version as a function of the size of the thread-pool from many
logfiles (one per point). As explained in the docu in the script, the file name format is fixed
and is the same of the one produced by the BrunelWrapper. The way in which the files are opened
(which ones) is regulated by 2 lists: one of thread pool sizes, one of evts in flight.
The duplicity for clone/not clone is automatically taken care of.
