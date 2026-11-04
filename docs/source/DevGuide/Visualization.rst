.. include:: ../old/global.rst

|newpage|

.. _dev-visualization:

Visualization facilities
========================

The visualization model described in the legacy guide
(:doc:`../old/GDG_Visualization`) — graphical representation services,
converters producing "drawables", GUI services — was implemented on top
of the Open Scientist suite (OpenInventor, ...). **Open Scientist has
long been retired** and with it the corresponding Gaudi interfaces:
Gaudi itself does not provide visualization facilities any more, and
the converter-for-graphics approach has not been revived in the
framework.

Experiment visualization today happens *outside* of the framework,
reading framework output data:

* `Phoenix <https://github.com/HSF/phoenix>`_ is the experiment
  visualization tool of the HSF, used by LHCb, ATLAS and others. It
  reads event data from files rather than interfacing in-process with
  the framework: there is currently **no direct interface between
  Phoenix and Gaudi** — events are produced by a Gaudi job, persisted
  (see :ref:`dev-dataaccess`), then opened in Phoenix or converted with
  helper scripts provided by the experiments.

* the interactive route for inspection remains :ref:`dev-scripting`:
  GaudiPython lets one browse the transient stores and inspect event
  data at the Python prompt, which covers most "quick look at the
  event" needs.

If an experiment wants to expose event displays, the recommended
direction is to persist an event model to a backend Phoenix supports,
not to re-introduce in-process graphics converters in Gaudi.
