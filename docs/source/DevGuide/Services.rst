.. include:: ../old/global.rst

|newpage|

.. _dev-services:

Framework services
==================

Services are components managed for the whole lifetime of the job and
shared by all algorithm instances. This chapter covers how user code
*uses* services, how to *write* a service, how *properties* work in
modern Gaudi, and the status of the incident service.


Using services: ServiceHandle
-----------------------------

The modern way to use a service is to declare a ``ServiceHandle<T>``
where ``T`` is the *interface* of the service:

.. code-block:: cpp

    #include <GaudiKernel/ServiceHandle.h>

    ServiceHandle<IMyInterface> m_svc{ this, "SomeSvc", "Type[/InstanceName]" };

    // in initialize() or later:
    const auto& svc = *m_svc;          // dereferences to the interface

The handle registers itself as a property of the owning component, so
the service instance can be redirected from the job configuration
(``C.MyAlg(SomeSvc="AlternativeInstance")``). It retrieves the service
lazily, at first use. ``ServiceHandleArray<T>`` is available when a
list of services is needed.

The direct calls ``service("MySvc")``, and ``svcLoc()->service(...)``,
are legacy idioms and should not appear in new code.


Writing a service
-----------------

A service derives from the ``Service`` base class
(``GaudiKernel/Service.h``) and typically implements one *interface*
(``IWhatever``) it will be used through:

.. code-block:: cpp

    #include <GaudiKernel/Service.h>

    class MySvc : public extends<Service, IMyInterface> {
    public:
      using extends::extends;
      Gaudi::Property<int> m_maxSize{ this, "MaxSize", 1000, "..." };
    };
    DECLARE_COMPONENT( MySvc )

As for algorithms, only property declaration is allowed in the
constructor; other components may be looked up from ``initialize()``
onwards.

Services are configured like any other component from the job
configuration and made known to the application manager through its
``ExtSvc`` property (or, for services known by default like the white
board or the message service, by name).


Properties
----------

Declarative interface
^^^^^^^^^^^^^^^^^^^^^

Properties are now *members* of the component, declared with the
``Gaudi::Property<T>`` template — there is no ``declareProperty`` call
to thread through constructor or initialize steps:

.. code-block:: cpp

    Gaudi::Property<double>       m_cut{ this, "PtMin", 2.0, "minimum pT" };
    Gaudi::Property<std::string>  m_out{ this, "Output", "/Event/Out", "where to write" };

The member *is* the value: read it with ``m_cut.value()`` or implicitly
``m_cut``; write it with ``m_cut = ...``. The constructor arguments are
the owner, the job-option name, the default value, and the
documentation string (this is what the configuration machinery exposes
as defaults and help).

An **update handler** can be attached: it is invoked when the property
is set, e.g. to propagate a value change to a service or to recompute a
derived quantity:

.. code-block:: cpp

    Gaudi::Property<int> m_outputLevel{
        this, "OutputLevel", DEFAULT_LEVEL,
        [this]( Gaudi::Details::PropertyBase& ) { this->updateMsgStreamOutputLevel( m_outputLevel ); },
        "output level" };

(This is precisely how components implement ``OutputLevel``, visible in
``GaudiKernel/AlgTool.h``; the same idiom is available for any property.)

A handler can also be a member function pointer or a free function;
handlers can be disabled (``useUpdateHandler( false )``) when the
semantics requires. Legacy code uses ``SimpleProperty`` and ``declareProperty( name, memberVar )``;
both are deprecated idioms: use ``Gaudi::Property`` members directly.

Extending properties: grammars
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Custom property value types participate in the configuration machinery
by defining how they are converted between their C++ value and the
string exchanged with Python (see :ref:`dev-starting`).

On the C++ side, the string-to-value conversion is provided by a
*grammar*: a parser (based on ``boost::spirit::qi``) declared for the
type in the ``Gaudi::Parsers`` namespace. A concrete example is the
parsing of histogram axes — i.e. histogram axes become job options —, in
``Gaudi/Accumulators/AxisAsProperty.h``:

.. code-block:: cpp

    // allows using Gaudi::Accumulators::Axis as a property type:
    Gaudi::Property<Gaudi::Accumulators::Axis<double>> m_axis{
        this, "Bins", Gaudi::Accumulators::Axis<double>{ 100, 0., 100. }, "" };

which the job configuration can then set with a string like
``"(150, 0., 300., \"Energy\")"`` (bin count, minimum, maximum,
optional title — the format defined by ``AxisGrammar``).

To make a new type usable as a property:

#. provide a grammar in ``Gaudi::Parsers`` producing the type from a
   string (see ``Gaudi/Parsers/Grammars.h``, ``CommonParsers.h`` and
   ``Factory.h``);
#. provide the C++ value-to-string conversion (``Gaudi::Utils::toString``
   overloading, see ``GaudiKernel/ToStream.h``);
#. on the Python side, give the corresponding *property semantics* in
   ``GaudiConfig2`` if the Python-side representation differs from
   plain strings — this maps the type checking and validation described
   in :ref:`dev-starting`.

The incident service
--------------------

The ``IncidentSvc`` dispatches named incidents to registered listeners
(``IIncidentListener::handle( const Incident& )``). Typical uses are
file-level notifications (``BeginInputFile``, ``EndOutputFile``, ...)
and "run begin/end" transitions, see ``GaudiKernel/Incident.h``.

.. warning::

   **Event-related incidents are gone and must not be used.**
   Incidents like begin/end-of-event processing would be delivered for
   all events of all slots, in an order unrelated to the execution of a
   particular algorithm — they fundamentally break thread safety and
   have been removed from the thread-safe scheduling. If code needs to
   react to event-level states, do it in the algorithm's own
   ``execute`` or via explicit data-flow dependencies.
