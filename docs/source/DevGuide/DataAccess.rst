.. include:: ../old/global.rst

|newpage|

.. _dev-dataaccess:

Accessing data
==============

.. warning::

   This chapter replaces the data access chapter of the legacy guide,
   which described the direct use of the event data service
   (``evtSvc()->retrieve(...)``, ``evtSvc()->registerObject(...)``) and
   the ``DataObject`` class hierarchy. That style is now obsolete: user
   algorithms should never talk to the stores directly, and ``DataObject``
   must be considered an internal implementation detail of the transient
   data stores (and not even of all of them: some stores hold payloads
   that are not ``DataObject``-based, wrapped internally for the
   framework).

This chapter describes direct access to data by user code.
Data access is actually handled automatically when using the functional framework
(see :ref:`dev-algorithms`). Thus it is recommended to use the functional
framework and not deal with data handles directly.
The rest of the chapter (selections, ranges, conditions, ...) also applies
to the functional framework.

The transient event store
-------------------------

Historically, per-event data lives in the *event data store* or *transient Event store* (aka *TES*).
As any Gaudi component, the *TES* can have, and thus does have, multiple implementations.
The most used one is probably the "whiteboard",
``HiveWhiteBoard``, configured as ``HiveWhiteBoard("EventDataSvc",
EventSlots=N)``. The whiteboard keeps a separate store per slot when
more than one event slot is used and the framework activates the right one
for the current event, so algorithms never see data of another event.

Anyway the stores are opaque to user code. All access goes through **data
handles**. They may even not be used by certain schedulers implementing
alternative ways of dealing with data.

Data handles
------------

A data handle is a member of an algorithm (or tool, or service) that
declares *which* product it accesses and *how*:

.. code-block:: cpp

    #include <GaudiKernel/DataObjectHandle.h>

    class MyAlg : public Gaudi::Algorithm {
      // "InputTracks" is the property name, "/Event/Tracks" the location
      DataObjectReadHandle<MyTrack::Container>  m_in{ this, "InputTracks", "/Event/Tracks" };
      DataObjectWriteHandle<MyTrack::Selection> m_out{ this, "OutputTracks", "/Event/SelectedTracks" };
      ...
    };

What the declaration buys you:

* the framework knows the inputs and outputs of the algorithm — this
  is what the scheduler uses to build the data flow (see
  :ref:`dev-architecture`);
* read-only access is checked at compile time (a ``ReadHandle`` only
  gives ``const`` access);
* collisions are detected at run time, trying to overwrite on existing
  location will fail
* the store location is a ``Gaudi::Property`` member of the component: users can
  redirect it from the job configuration, e.g.
  ``C.MyAlg(InputTracks="/Event/Sim/Tracks")``. The third parameter in the
  declaration is only the default value.

Reading and writing:

.. code-block:: cpp

    StatusCode execute( const EventContext& ) const override {
      const auto* tracks = m_in.get();          // throws if the product is missing
      const auto* maybe  = m_in.getIfExists();  // returns nullptr if missing
      ...
      m_out.put( std::make_unique<MyTrack::Selection>( ... ) );
      return StatusCode::SUCCESS;
    }

Ownership of the object passed to ``put`` is taken over by the store;
publish with ``std::unique_ptr`` and do not keep a pointer for later
modification.

In the functional framework the same handles are generated from the
algorithm's signature; only the locations are declared explicitly:

.. code-block:: cpp

    Transformer( name, pSvc,
                 { "InputData", "MyTracks" },     // a DataObjectReadHandle<MyTracks>
                 { "OutputData", "MyOutTracks" } )

Selections and ranges
---------------------

Several handle-based conveniences exist to read containers:

* a ``DataObjectHandle<Gaudi::Range_<T::ConstVector>>`` gives a *range*
  over the content of a container in the store, which the functional
  transformers typically take as argument;
* the range read transparently adapts whether the store holds a full
  container or a ``Selection`` of it (see ``transformer.cpp`` in
  ``GaudiTestSuite/src/FunctionalAlgorithms`` — the same transformer
  accepts both).

Detector conditions
-------------------

Conditions data (calibration, alignment, ...) has a lifetime spanning
several events and is *not* stored in the event whiteboard. It is
published by dedicated condition services/loader algorithms and accessed
through handles declared with the *condition data* mode (see
``GaudiKernel/DataHandle.h`` and the ``DataObjID`` family). Details are
framework-level; the essential rule is the same: declare the dependency,
do not look the data up.


Persistency
-----------

The persistency of event data is **not** part of the generic algorithm
interface any more: there is no "converter per data type" machinery to
extend as in the old guide. Reading and writing events is achieved with
dedicated, experiment-specific algorithms that extract data from the
transient store and hand it over to the I/O backend of choice (ROOT,
HDF5, ...), or the other way around. The Gaudi repository ships an
example in ``RootCnv`` (a ROOT-based conversion service) and
``GaudiTestSuite/options/ROOT_IO`` shows its usage.
