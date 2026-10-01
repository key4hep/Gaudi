.. include:: ../old/global.rst

|newpage|

.. _dev-scripting:

Scripting and interactivity
===========================

Gaudi jobs are configured in Python, but the application itself runs
from C++. For interactive work — inspecting a store, stepping through
events, debugging a configuration — the framework ships the
``GaudiPython`` package. Two layers need to be distinguished:

* the **job configuration**, which is regular Python
  (:ref:`dev-starting`), and
* ``GaudiPython``, an embedded Python interpreter inside (or driving) a
  Gaudi application, giving interactive access to the application
  manager, the stores and the services.

Running interactively with GaudiPython
--------------------------------------

``gaudipython`` starts a Python interpreter with the Gaudi bootstrap
loaded. The central object is the ``AppMgr`` class of the
``GaudiPython.Bindings`` module, which exposes the application
manager:

.. code-block:: python

    from GaudiPython.Bindings import AppMgr

    theApp = AppMgr()             # create the application manager
    theApp.config()               # run configure
    theApp.initialize()
    theApp.run( 10 )              # process 10 events, then return to the prompt!

    evtSvc = theApp.evtSvc()      # the event store of last event
    print( evtSvc.nodes() )

Because the interpreter stays alive after ``run``, one can inspect the
stores, query services and modify the configuration event by event —
indispensable for debugging.

Python-written algorithms
-------------------------

It is possible to implement an algorithm in Python, by inheriting from
``GaudiPython.Bindings`` ``PyAlgorithm``: the C++ side
(``GaudiPython/Algorithm.h``) forwards the state machine calls
(``initialize``, ``execute``, ``stop``, ...) to the corresponding
Python object. It is convenient for quick interactive sessions and
prototyping; production algorithms should remain C++.

Auditors
--------

*Auditors* are framework components (interface ``IAuditor``) invoked
before/after calls to algorithms — the standard way to add
non-intrusive instrumentation: timing, memory profiling, logging of
algorithm calls. They are attached through the ``AuditorSvc``:

.. code-block:: python

    C.AuditorSvc("AuditorSvc", Auditors=["ChronoAuditor", "AlgContextAuditor"])

and are a job-configuration choice, not a code change. The standard
auditors live in ``GaudiAud`` (``ChronoAuditor``, ``AlgContextAuditor``,
``MemStatAuditor``, ...).

Python-implemented auditors (``GaudiPython.Auditors``) are the
interactive counterpart of GaudiPython algorithms: they allow
prototyping an audit in Python (dumping algorithm call traces,
collecting statistics in interactive sessions) without recompiling —
the configuration registers them with ``AuditorSvc`` like any C++
auditor.

.. note::

   The old ``GaudiPython``-driven mode of running full production jobs
   (a Python driver ``run`` loop) is superseded by ``gaudirun.py`` and
   GaudiConfig2 (see :ref:`dev-starting`); GaudiPython remains the tool
   for *interactive* sessions.
