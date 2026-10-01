.. include:: ../old/global.rst

|newpage|

.. _dev-starting:

Getting started
===============

This chapter shows how to configure and run a minimal Gaudi job with
the modern interfaces. It assumes you already have a built Gaudi
environment; see :doc:`../HowTos/building` for how to get one.


Job configuration: GaudiConfig2
-------------------------------

Modern Gaudi jobs are configured in Python, but not with the legacy
``Configurables``/``gaudiconf`` machinery used in the old guide. The
modern configuration layer is ``GaudiConfig2``, which brings:

* strict validation of property values in Python, before the job
  starts — a wrong type or a misspelled property name is caught
  immediately instead of being silently ignored;
* explicit, trackable instances: there is no implicit global registry of
  "named singletons", so a configuration fragment only contains the
  components that were put in it (see ``mergeConfigs`` below);
* extensible *property semantics*: custom property types can carry
  their own Python-side validation and C++ string conversion,
  without changes to the core framework (see :ref:`dev-services`).

Components are instantiated through facade classes exposed by
``GaudiConfig2.Configurables``, organized in modules following the C++
namespaces of the components. A configuration is simply a function
returning a list of instances:

.. code-block:: python

    from GaudiConfig2 import Configurables as C


    def config(nevt=100):
        algorithm = C.JSONPropertyAlg(JSON={"threshold": 2.5})
        return [
            C.ApplicationMgr(EvtMax=nevt, EvtSel="NONE", TopAlg=[algorithm]),
            algorithm,
        ]

Returning a component instance is what puts it in the job: a
component that is only constructed but never returned (directly or as a
property of another component) does not exist in the job.

Non-trivial configurations are assembled from fragments with
``mergeConfigs``, each fragment returning the configurables it owns.
This example is adapted from the Gaudi test suite
(``GaudiTestSuite/python/GaudiTestSuite/TestConfig2.py``):

.. code-block:: python

    from GaudiConfig2 import Configurables as C
    from GaudiConfig2 import mergeConfigs


    def setUpAlgorithms():
        algorithms = [
            C.Gaudi.TestSuite.Counter.GaudiHistoAlgorithm("SimpleCounterHistos"),
        ]
        app = C.ApplicationMgr(TopAlg=algorithms)
        return algorithms + [app]


    def histogramWriting(filename):
        return [
            C.ApplicationMgr(HistogramPersistency="ROOT"),
            C.RootHistCnv.PersSvc("RootHistSvc", OutputFile=filename),
            C.HistogramSvc("HistogramDataSvc"),
            C.Gaudi.Histograming.Sink.Root(),
        ]


    def main(nevt=50000, outfile="histo-c2.root"):
        return mergeConfigs(
            setUpAlgorithms(),
            histogramWriting(outfile),
            [C.ApplicationMgr(EvtMax=nevt, EvtSel="NONE")],
        )


A first algorithm
-----------------

The preferred way to write an algorithm in modern Gaudi is the
*functional framework* (see :ref:`dev-algorithms`): the algorithm is a
function, its inputs and outputs are declared in its signature, and
counters/histograms are declared as members. A minimal algorithm that
just counts events (adapted from
``GaudiTestSuite/src/FunctionalAlgorithms/EventCounter.cpp``):

.. code-block:: cpp

    #include <Gaudi/Accumulators.h>
    #include <Gaudi/Functional/Consumer.h>

    namespace Gaudi::TestSuite {
      struct EventCounter final : Functional::Consumer<void()> {
        using Consumer::Consumer;

        void operator()() const override { ++m_count; }

        mutable Accumulators::Counter<> m_count{ this, "count" };
      };
      DECLARE_COMPONENT( EventCounter );
    } // namespace Gaudi::TestSuite

and one more complex that transforms data (adapted from
``GaudiTestSuite/src/FunctionalAlgorithms/transformer.cpp``) and uses a Property :

.. code-block:: cpp

    #include <Gaudi/Functional/Transformer.h>
    #include <GaudiTestSuite/MyTrack.h>

    namespace Gaudi::TestSuite {
      struct SelectTracks final : Functional::Transformer<
              MyTrack::Selection( const Range_<MyTrack::ConstVector>& )> {

        SelectTracks( const std::string& name, ISvcLocator* pSvc )
            : Transformer( name, pSvc,
                           { "InputData", "MyTracks" },
                           { "OutputData", "MyOutTracks" } ) {}

        MyTrack::Selection
        operator()( const Range_<MyTrack::ConstVector>& in ) const override {
          MyTrack::Selection out;
          out.insert( in.begin(), in.end(),
                      []( const MyTrack* t ) { return t->px() >= m_threshold; } );
          return out;
        }

        Property<float> m_threshold{ this, "Threshold", 10.0f, "Minimum px" };
      };
      DECLARE_COMPONENT( SelectTracks )
    } // namespace Gaudi::TestSuite

Note the modern style even in these few lines:

* operator() is const, ensuring thread safety
* extra configuration is declared with ``Gaudi::Property<T>`` members (described in
  detail in :ref:`dev-services`). The configuration of the algorithm in the
  python job options is then simply using the property name (given as second
  argument), e.g. ``C.SelectTracks(Threshold=20.5, ...)``;
* Inputs and outputs are also declared and given Property names and default
  values in the constructor. The values can then be overwritten from the python
  job options, e.g. ``C.SelectTracks(InputData="MyOtherTracks", OutputData="MyOtherOutput")``;
* the counter is a ``Gaudi::Accumulators::Counter``, registered and
  persisted automatically — nothing needs to be booked, saved or
  released by hand. By default counters are thread safe and thus should
  be declared mutable so that one can change them in operator()

An example of the old, non-functional way of writing an algorithm can
be found in the legacy guide (see the algorithms chapter of
:doc:`../old/GDG`); it is kept there as a reference for existing code.


Running the job
---------------

A GaudiConfig2 configuration is executed with ``gaudirun.py``, pointing
at a ``module:callable`` (or ``path/to/file.py:callable``) that returns
the list of configurables:

.. code-block:: bash

    gaudirun.py mypackage.myconfig:main

``gaudirun.py --help`` lists the handling options; a useful one is
``-o options.json`` which dumps the job options actually sent to the
application, handy to check what your Python code produced.


Job execution and multi-threading
---------------------------------

The list in ``ApplicationMgr.TopAlg`` is **not** executed top to
bottom. It declares the set of algorithms of the job; the *order* of
execution is decided at run-time by the scheduler from the data
dependencies declared by the algorithms (see :ref:`dev-architecture`):

* algorithm B runs after algorithm A because B declares as input a
  product published by A — not because B appears after A in a list;
* algorithms with no mutual dependency may run in any order, or
  concurrently on different threads;
* with several event slots, several *events* are processed concurrently.

The concurrency is a property of the job, not of the code. For example
(adapted from
``GaudiExamples/TinyExperiment/python/GaudiExamples/TinyExperiment/FullExperiment.py``):

.. code-block:: python

    whiteboard = C.HiveWhiteBoard("EventDataSvc", EventSlots=12)
    scheduler = C.AvalancheSchedulerSvc(ThreadPoolSize=10)
    eventloop = C.HiveSlimEventLoopMgr(SchedulerName=scheduler.name())
    C.ApplicationMgr(EventLoop=eventloop, ...)

The same algorithm code runs sequentially with a single-threaded
scheduler or concurrently with ten threads. A practical consequence:
when debugging, reduce ``ThreadPoolSize`` and ``EventSlots`` to 1 to
recover a strictly sequential (hence reproducible) execution order.
