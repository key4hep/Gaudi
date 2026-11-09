.. include:: ../old/global.rst

|newpage|

.. _dev-overview:

Introduction
============

Purpose of this document
------------------------

This document is the developer's guide to the Gaudi application
framework. It describes how to write, configure and run applications
built on top of Gaudi: how to write algorithms, how to exchange data
between them, how to use records such as histograms, counters and
ntuples, and how to exploit the services and tools provided by the
framework.

It is complemented by the auto-generated :doc:`API documentation
<../doxygen>`.

A previous version of this guide, covering the legacy interfaces that
are kept for backward compatibility, is still available in
:doc:`../old/GDG`. Example of old-style code can be found there.


The goal of Gaudi
-----------------

Gaudi is a framework for building data processing applications for
High-Energy Physics experiments. Its goal is simple to state:

  *run a set of user-provided algorithms on a set of events, and
  provide all the tools needed to do it efficiently and correctly.*

Around this core goal, the framework provides:

* the machinery to load, configure and run user algorithms, including
  multi-threaded, out-of-order scheduling of those algorithms
  (see :ref:`dev-architecture`);
* the transient data stores in which algorithms publish and retrieve
  per-event, per-run or per-file data (see :ref:`dev-dataaccess`);
* a set of core services: message logging with pluggable sinks,
  random number generation, incident handling, data persistency, etc.
  (see :ref:`dev-services` and :ref:`dev-logging`);
* tools for the needs of physics code: counters, statistical
  accumulators and histograms (see :ref:`dev-histograms`), ntuples
  (see :ref:`dev-ntuples`), and a mechanism to share utility code
  through tools (see :ref:`dev-tools`).

Gaudi scales from a quick interactive test job processing a handful of
events up to the production reconstruction workflows of the LHC
experiments.


The spirit of the framework
---------------------------

The central design goal of Gaudi is to make *user code independent of
the internal implementation of the framework*. User code should
describe **what** it needs, not **how** the framework must provide it.

This is achieved with a *declarative* programming style. Instead of
calling into framework internals, an algorithm declares:

* its inputs and outputs (data dependencies), which the scheduler
  uses to decide when it can run;
* its configuration properties, which the framework sets from the job
  configuration before the algorithm is initialized;
* the counters, histograms and ntuples it wants to fill, which the
  framework instantiates, registers and persists;
* the services and tools it wants to use, which the framework locates
  and instantiates on the algorithm's behalf.

All of these declarations are bound to concrete implementations
internally through *handles* and *accumulators*. Because user code only
talks to handles and never to the underlying services directly, the
framework is free to evolve: for example the same algorithm code runs
sequentially on one thread or concurrently on many threads without any
modification.

The functional framework described in :ref:`dev-algorithms` pushes this
style to its logical conclusion: in it, an algorithm is essentially a
function, and everything it needs (inputs, tools, counters, histograms)
is declared up-front in its class definition or members.


Structure of this guide
-----------------------

:ref:`dev-architecture` gives an overview of the framework architecture,
with the emphasis on algorithm management and multi-threaded scheduling.

:ref:`dev-starting` shows how to configure and run a first job using the
modern (``GaudiConfig2``) configuration and a functional algorithm.

The remaining chapters cover the main areas of the framework in depth:
writing algorithms, accessing data, histograms and counters, ntuples,
services and properties, tools, logging and sinks, scripting and
visualization.
