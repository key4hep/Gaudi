.. include:: ../old/global.rst

|newpage|

.. _dev-architecture:

The framework architecture
==========================

This chapter gives an overview of the architecture of Gaudi as it is
used today. Older architectural documents (e.g. the LHCb notes
referenced in the legacy guide, :doc:`../old/GDG`) are still valuable
as background reading, but the accent has shifted: the heart of the
modern framework is the management and (multi-threaded) scheduling of
algorithms, and this chapter concentrates on it.


The big picture
---------------

A Gaudi application processes a sequence of **events** (or, more
generally, *time slices* identified by an ``EventContext``) through a
set of **algorithms** supplied by the user and plugged into the
framework as component libraries. The framework's job is to:

#. configure and initialize all components from a job description;
#. feed events to the algorithms, deciding at each moment which
   algorithms can execute and on which CPU/GPU;
#. provide algorithms with data and services through handles, without
   exposing the underlying implementation;
#. finalize everything cleanly (flushing histograms, ntuples, logs,
   ...).

From the point of view of the user the framework is *declarative*: an
algorithm declares its inputs, outputs, properties and sub-algorithms
(its "dependencies") and the framework derives from those declarations
both the data flow and the execution schedule. There is no need (and it
is discouraged) for user code to call framework internals directly.


The main components
-------------------

Application manager
    ``ApplicationMgr`` (GaudiKernel) is the top-level component. It
    owns all the others: it creates the configured services, declares
    the algorithms, and drives the application through a finite state
    machine (``configure`` -> ``initialize`` -> ``start`` ->
    event loop -> ``stop`` -> ``finalize`` -> ``terminate``). All
    components implement the same state machine, and state transitions
    are propagated through the component hierarchy by the managers
    (``ISvcManager`` for services, ``IAlgManager`` for algorithms).

Services
    Services are components with lifetime tied to the job (created at
    ``configure``, destroyed at ``terminate``) and typically unique per
    type. They provide infrastructure: the event data service, the
    message service, file access, etc. See :ref:`dev-services`.

Algorithms
    Algorithms implement the actual data processing. They are
    instantiated once per scheduler pool (not once per event) and must
    therefore hold **no mutable per-event state**. The modern base
    class is ``Gaudi::Algorithm``; the legacy ``Algorithm`` class still
    exists for backward compatibility and is now implemented as a thin
    adapter (``LegacyAlgorithmAdapter``) on top of ``Gaudi::Algorithm``.
    The functionally-styled algorithms of the *functional framework*
    (``Gaudi::Functional``, see :ref:`dev-algorithms`) are the preferred
    interface for new code.

Event loop
    The event loop manager (``HiveSlimEventLoopMgr``) drives the
    processing of events. For each event it builds an ``EventContext``
    and hands the work to the scheduler. With multiple event slots,
    several events are in flight at the same time.


Scheduling
----------

Why a scheduler
^^^^^^^^^^^^^^^

Modern Gaudi no longer executes the ``TopAlg`` list top to bottom.
Algorithms are treated as **tasks** with declared data dependencies,
and a scheduler service decides when each task runs. This is what makes
intra-event and inter-event parallelism possible: two algorithms that
do not depend on each other's products can run at the same time, and
several events can be processed concurrently in different *event slots*.

The default scheduler is ``AvalancheSchedulerSvc`` (GaudiHive), a
TBB-based, graph-driven scheduler. A task becomes runnable when:

- the control flow graph traversal reaches it;
- all its **data flow dependencies** (its declared inputs) are
  available;
- a free algorithm instance and a free computation resource exist.

The scheduler explores the pool of data-flow-ready tasks with several
prioritization strategies (based on the structure of the precedence
rules graph) that aim at maximizing the average number of in-flight
tasks, hence the "avalanche" name.

The building blocks of the scheduling machinery are:

PrecedenceSvc
    Maintains the precedence rules of the application: a *control-flow*
    (CF) view built from the sequencing tree, and a *data-flow* (DF)
    view built from the input/output data dependencies declared by the
    algorithms. It can dump both graphs (``dumpControlFlow`` /
    ``dumpDataFlow`` / ``dumpPrecedenceRules``) for debugging.

AlgResourcePool
    Keeps the pool of algorithm instances, providing a free instance to
    the scheduler when a task becomes runnable.

ThreadPoolSvc
    Provides the worker threads. Its size is controlled by the
    ``ThreadPoolSize`` property of the scheduler; with a size of one
    the same machinery runs sequentially.

Data broker
    Reads the data handles declared by the algorithms (see
    :ref:`dev-dataaccess`) and delivers to the scheduler the set of
    input dependencies, i.e. the products an algorithm waits for.

Event slots
    With more than one event slot (and enough threads), several events
    progress concurrently. Each slot holds the slot-local data
    (whiteboard, algorithm states), guaranteeing isolation between
    events.

As for any Gaudi component, one can write his/her own scheduler and replace
the default ``AvalancheSchedulerSvc``. As an example, LHCb wrote the
`HLTControlFlowMgr <https://gitlab.cern.ch/lhcb/LHCb/-/blob/master/Hlt/HLTScheduler/src/HLTControlFlowMgr.cpp>`__
which aims at maximizing the throughput in the context of a high level
trigger and thus precompute a sequence of algorithms respecting the data
dependencies and runs it for one event at a time, mapping one different
event on every available core. This also optimizes the usage of the CPU
caches as a given event is kind of pinned to a given core.


Data dependencies, not sequences
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Classical Gaudi sequencing (an ordered list of algorithms) is still
supported — the control-flow tree is built from it for backward
compatibility — but it is **no longer the primary way to express
dependencies**. The execution order of the physics algorithms follows
from their declared inputs and outputs: if algorithm B declares the
product of algorithm A as input, B is guaranteed to run after A, in
whatever order or concurrency the scheduler picks. User-defined
sequences (``Gaudi::Algorithm::Sequence`` and related sequencers) are
still used by the ``AvalancheSchedulerSvc`` to express control flow
(AND/OR aggregation, filtering, branching), not data dependencies.
Other schedulers may of course use different ways, e.g. the LHCb
``HLTControlFlowMgr`` uses a notion of ``ContolFlowNodes``.

.. note::

   Because the scheduler reorders execution, **no algorithm may rely on
   implicit ordering** (e.g. "histogram booked by an earlier
   algorithm"), nor on any shared mutable state. Data produced during
   one event must only be exchanged through the transient stores with
   declared handles.


The functional framework
------------------------

On top of ``Gaudi::Algorithm``, Gaudi provides a *functional framework*
(``Gaudi::Functional``) in which an algorithm is written as a function:
it declares its inputs and outputs as data handles, its tools as
function arguments, and its ``operator()`` implements the computation.
The framework instantiates the algorithm from the function signature.

This style is the preferred interface for new code:

* inputs and outputs are explicit in the signature — the data
  dependencies needed by the scheduler come for free;
* there is no per-event state to protect, hence thread safety by
  construction;
* tools, counters and histograms are available either as arguments or
  as members of the algorithm class, so no framework look-up is needed
  in the function body.

See :ref:`dev-algorithms` for the practical details.


Concurrency configuration at a glance
-------------------------------------

Multi-threaded execution is controlled from the job configuration: the
number of worker threads (``ThreadPoolSize``), the number of events in
flight (event slots), and the scheduler type are properties of the
scheduler and event-loop components, for example (see
:ref:`dev-starting` for the configuration syntax):

.. code-block:: python

    AvalancheSchedulerSvc(ThreadPoolSize=8)

The same C++ code runs unchanged from a single-threaded debugging
session to a full multi-threaded production job; only the schedule of
the algorithms changes. Techniques that break this property — mutable
member state with no protection in algorithms, reliance on execution order, event-related
incidents — must be avoided (see the corresponding chapters).
