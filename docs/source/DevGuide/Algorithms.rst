.. include:: ../old/global.rst

|newpage|

.. _dev-algorithms:

Writing algorithms
==================

Algorithms are the components that contain the user code executed for
each event. Modern Gaudi provides two ways of writing them:

* the class-based interface, deriving from ``Gaudi::Algorithm``;
* the **functional framework**, in which the algorithm is a function
  declared together with its inputs and outputs. This is the preferred
  interface for new code.

Both are presented below, after a few words on what has changed since
the legacy guide.


What changed
------------

* The base class for algorithms is ``Gaudi::Algorithm``
  (``Gaudi/Algorithm.h``). The old ``::Algorithm`` class
  (``GaudiKernel/Algorithm.h``) still exists for backward compatibility
  and is now a thin adapter (``LegacyAlgorithmAdapter``) on top of
  ``Gaudi::Algorithm``.
* The per-event entry point now takes the event context:
  ``StatusCode execute( const EventContext& ctx ) const`` — note the
  ``const``: algorithms are expected to be reentrant and usable from
  several threads.
* Algorithms are no longer chained through sequences to define
  dependencies. The order and concurrency of execution is derived by
  the scheduler from the data dependencies declared with handles (see
  :ref:`dev-dataaccess` and :ref:`dev-architecture`). Sequencers survive
  only to express control flow (filtering, branching) in some
  schedulers, and in particular the default ``AvalancheScheduler``)


The ``Gaudi::Algorithm`` base class
-----------------------------------

A concrete algorithm typically overrides ``initialize()``, ``execute()`` and
``finalize()``:

.. code-block:: cpp

    #include <Gaudi/Algorithm.h>

    class MyAlg : public Gaudi::Algorithm {
    public:
      using Gaudi::Algorithm::Algorithm;  // constructor

      StatusCode initialize() override { /* initialization */ }
      StatusCode execute( const EventContext& ctx ) const override { /* per-event work */ }
      StatusCode finalize() override { /* cleanup */ }
    };
    DECLARE_COMPONENT( MyAlg )

The base class, configured and managed by the framework, provides:

* the finite state machine hooks ``initialize()``, ``start()``,
  ``stop()``, ``finalize()``, plus the (less commonly used)
  ``reinitialize()``/``restart()``;
* properties declared as ``Gaudi::Property<T>`` members, set from the
  job configuration before initialization (see :ref:`dev-services`);
* accessors to the standard services (``evtSvc()``, ``detSvc()``,
  ``randSvc()`` ...) and the message interface (``info()``,
  ``warning()``, ...), together with the ``Gaudi::Property<int>``
  ``OutputLevel`` of the component;
* acceptance bookkeeping: ``setFilterPassed( bool )`` /
  ``isPassed()`` allow an algorithm to act as a filter in a control
  flow sequence; ``errorCount()`` counts the failures of an algorithm.

Rule to respect:

* ``execute()`` runs concurrently with other algorithms and, with
  multiple algorithm instances or event slots, concurrently with
  itself. It must not modify member state in a non thread safe manner.
  For this purpose it is declared const.
  Note that Gaudi accumulators are thread safe and can thus be used,
  provided they are declared mutable (see :ref:`dev-histograms`).
* usage of input and outputs requires manual interaction with the
  transient event store. It is highly recommended to use DataHandles
  for this purpose (see :ref:`dev-dataaccess`). An even better way is to switch to the
  functional framework.


The functional framework (prefered way)
---------------------------------------

In the functional framework (headers under
``Gaudi/Functional/``) an algorithm is
a class deriving from a *signature* — for example
``Transformer<Out( const In& )>`` — and implementing an ``operator()``
with matching arguments. The framework derives the data handles (and
hence the scheduling dependencies) from the signature and instantiates
the algorithm accordingly; user code just reads its arguments and
returns its outputs.

Besides the ``Transformer`` used in the example of
:ref:`dev-starting`, the following algorithm families are available:

Producer
    ``Producer<Out()>`` or ``Producer<std::tuple<Out...>()>``: creates
    its outputs from nothing (event generation, reading from an
    external source).

Consumer
    ``Consumer<void( const In&... )>``: only reads data (analyses,
    checks).

Transformer
    ``Transformer<Out( const In&... )>``: reads its inputs and returns
    one output; ``MultiTransformer<std::tuple<Out...>( const In&... )>``
    produces several outputs.

FilterPredicate
    ``FilterPredicate<bool( const In&... )>``: decides whether
    downstream algorithms of the current control flow branch run.
    This is the declarative counterpart to a sequence with a filtering
    algorithm.

MergingTransformer
    ``MergingTransformer<Out( const vector_of_const_<Ins>&, ... )>``:
    combines the content of several input containers of the same type
    into one output.

SplittingTransformer
    ``SplittingTransformer<vector_of_<Out>( const In&... )>``: splits
    inputs into a set of output containers of the same type.

SplittingMergingTransformer
    ``SplittingMergingTransformer<vector_of_<Out>( const vector_of_const_<In>& )>``: splits
    recombines the content of several input containers into a set of
    output containers of the same type.

ScalarTransformer
    ``ScalarTransformer<ScalarOp, Out( const In&... )>``
    allows to work on a set of consistent containers
    (meaning having same number of items) and applies a given transformation
    on each entry to create a new container of output objects. Basically
    the equivalent of ``std::transform``.
    ``MultiScalarTransformer<ScalarOp, std::tuple<Out...>( const In&... )>``
    provides the same feature with several outputs.


The input/output locations are given to the constructor as
(set of ) *key* / *location* pairs:

.. code-block:: cpp

    Transformer( name, pSvc,
                 { "InputData", "MyTracks" },      // key and location of the input
                 { "OutputData", "MyOutTracks" } ) // key and location of the output

    MultiTransformer( name, pSvc,
                 { { "InputData", "MyTracks" },      // key and location of the inputs
                   { "OtherInput", "MyVertics" } },
                 { { "OutputData", "MyOutTracks" },  // key and location of the outputs
                   { "OtherOutput", "MyOutVertices" } } )

Caveats:

* The function signature *is* the declaration of the data flow: never
  bypass it with ``evtSvc()->retrieve`` style calls, which would hide
  dependencies from the scheduler.
* The same thread-safety rules as for ``Gaudi::Algorithm`` apply, but
  they are much harder to violate: the algorithm's ``operator()`` is
  ``const`` and there is no access to framework internals.


Control flow
------------

Filtering (stop the processing of the current branch), branching ("run
this alternative algorithm") and combination ("AND" / "OR" of filter
decisions) are control-flow, not data-flow, and are expressed in a different
way, which may even be scheduler specific.

In the case of the AvanlancheScheduler, it uses
sequencers (``Gaudi::Algorithm::Sequence`` and its variants, see
``Gaudi/Sequence.h`` and ``Gaudi/Sequencer.h``) together with filter
algorithms (``FilterPredicate`` in the functional framework, or
``setFilterPassed`` in class-based algorithms).

Keep the two concepts separate when designing an application:
*data* dependencies come from handles and are used by the scheduler;
*control* flow is expressed with sequences and filter decisions and
affects whether filters and their children run.
