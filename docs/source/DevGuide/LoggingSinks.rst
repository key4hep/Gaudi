.. include:: ../old/global.rst

|newpage|

.. _dev-logging:

Logging and monitoring sinks
============================

Gaudi decouples *producing* messages/records from *persisting* them:

* logging is produced by components through the messaging interface
  (``info()``, ``warning()``, ``error()`` ... on every component),
  formatted and routed by the ``MessageSvc``;
* counters, histograms and other records (see :ref:`dev-histograms`)
  are “monitoring entities” registered with a central
  ``Gaudi::Monitoring::Hub``;
* **sinks** are services plugged into the hub that decide what is saved
  where: a text dump into the message stream, a ROOT file for
  histograms, etc.

This chapter describes this sink mechanism: how to configure existing
sinks and how to write a new one.


Message logging
---------------

The ``MessageSvc`` (GaudiCoreSvc) collects messages from all components
and forwards them to its *areas* (stdout, files, streams). Log level is
controlled per component with the ``OutputLevel`` property; the job
verbosity is controlled by the ``MessageSvc`` properties
(``OutputLevel``, ``Format``, ``TimeStamped`` ...). Components should
use their messaging members — never ``std::cout`` — so that the message
goes through the level filtering and the sinks.

Configuring sinks
-----------------

Sinks are services, configured like any other service. The standard
ones:

``Gaudi::Monitoring::MessageSvcSink``
    Writes the collected monitoring entities (counters, histograms,
    etc. of the types selected by its properties) to the message
    stream — this is what produces the end-of-job counter printout.
    By default it saves ``counter``, ``statentity`` and ``histogram``
    types; its properties (``TypesToSave``, ``NamesToSave``,
    ``ComponentsToSave`` — lists of regexps) filter the entities.

``Gaudi::Histograming::Sink::Root`` (and variants under ``Gaudi/Histograming/Sink``)
    Writes histograms to a ROOT file (property ``FileName``, or shared
    with other components through the ``FileSvc``). This is the modern
    replacement for the old ``HistogramPersistencySvc``/``RootHistSvc``
    persistency machinery.

Example configuration (adapted from
``GaudiTestSuite/python/GaudiTestSuite/TestConfig2.py``):

.. code-block:: python

    from GaudiConfig2 import Configurables as C

    msgSink = C.Gaudi.Monitoring.MessageSvcSink(
        HistoStringsWidth=40, NamesToSave=["Prof.*", "Gauss.*"]
    )
    rootSink  = C.Gaudi.Histograming.Sink.Root()
    C.ApplicationMgr(ExtSvc=[msgSink, rootSink])


Writing a sink
--------------

A sink is a service deriving from ``Gaudi::Monitoring::BaseSink``
(``Gaudi/BaseSink.h``), which:

#. registers itself to the hub at ``initialize()``;
#. filters incoming entities according to ``TypesToSave`` /
   ``NamesToSave`` / ``ComponentsToSave``;
#. forwards them to ``flush(bool isStop)``, which the implementation
   must define — it is called at ``stop()`` and, if the
   ``AutoFlushPeriod`` property (in seconds) is non-zero, periodically
   from a dedicated thread.

The collected entities are available through ``applyToAllEntities`` /
``applyToAllSortedEntities``, each entity carrying its component, name,
type and a JSON representation of its current value. Histogram-specific
base classes (``Gaudi/Histograming/Sink/Base.h``) additionally
translate entities into ROOT objects through registered handlers per
histogram type.
