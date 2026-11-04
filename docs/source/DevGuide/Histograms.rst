.. include:: ../old/global.rst

|newpage|

.. _dev-histograms:

Histograms and counters
=======================

Modern Gaudi implements histograms and counters as *monitoring
entities* declared as members of algorithms. They are accumulated
locally — concurrently if the algorithm runs on several threads —,
registered with a central monitoring hub, and at the end of the job
handed over to **sinks** for persistency (see :ref:`dev-logging`).
The old "book into HistogramService, fill, and inspect in finalize"
pattern is therefore replaced by "declare as member, fill, done".

Counters
--------

Counters live in ``Gaudi::Accumulators`` (header ``Gaudi/Accumulators.h``).
The most common flavours are:

``Counter``
    An integral counter (number of events/candidates seen, ...).
    Has a ++ operator and `nbEntries` method

``AveragingCounter``
    Keeps count and mean of the values added to it.
    Only has a += operator, `nbEntries`, `sum` and `mean` methods

``SigmaCounter``
    Keep count, mean and second moment of the values added to it.
    Only has a += operator, the methods of ``AveragingCounter`` plus
    `biased_sample_variance`, `unbiased_sample_variance`, `standard_deviation`
    and `meanErr` methods

``StatCounter``
    Keep count, mean, second moment, min and max of the values added to it.
    Only has a += operator, the methods of ``SigmaCounter`` plus
    `min` and `max` methods

``BinomialCounter``
    An efficiency counter, tracking the number of passes and of trials.
    Only has a += operator taking a boolean and methods `nEntries`,
    `nTrueEntries`, `nFalseEntries`, `efficiency` and `efficiencyErr`
    methods

Each counter class is templated with :

.. code-block:: cpp

    template <typename Arithmetic = double, atomicity Atomicity = atomicity::full>

So by default counters use double and are thread safe. Any arithmetic type
can be used and atomicity can be set to ``atomicity::none`` for faster, but
non thread safe counters.

Counters are highly extensible and can be easily composed based on `Accumulators`,
see the documentation in Accumulator.h file.

An example of counter extension enabling easier logging is :

``MsgCounter``
    A counter also printing a given message to the message service on
    each call to ++ operator. Also has a mas count after which it will
    stop logging and just report once "Suppressing message ...".
    To be used for all errors and warnings.


Usage (adapted from
``GaudiTestSuite/src/FunctionalAlgorithms/EventCounter.cpp``):

.. code-block:: cpp

    struct EventCounter final : Gaudi::Functional::Consumer<void()> {
      using Consumer::Consumer;
      void operator()() const override { ++m_count; }
      mutable Gaudi::Accumulators::Counter<> m_count{ this, "count" };
    };

* Construction needs the owner component (``this``) and a name; this
  is enough for the entity to be registered with the
  ``Gaudi::Monitoring::Hub`` and printed/persisted at the end of the
  job.
* Counters are ``mutable`` by design: they are the one piece of member
  state algorithms are allowed to modify while running concurrently.
  Accumulation is thread-safe (atomic operations per entry
  per thread, merged when reported).
* Counter *sets* (maps of counters indexed by a string key, e.g. one
  counter per cut) are available as well.


Optimizing counter usage via buffers
------------------------------------

When counters are thread safe, updating them involves more than a simple
operation. Typically atomics are involved. In cases where this is repeated
in a tigh loop, it can become costful.

A simple solution is provided by the framework under the name of `buffer`s.
Every counter has a ``buffer`` method allowing to get a local, thread unsafe
and fast version of the counter, which is automatically merged into the main
counter when exiting the scope.
Typical code looks like this :

.. code-block:: cpp

    AveragingCounter<float, atomicity::full> avg;
    {
      auto buf = avg.buffer();
      for ( int i = 0; i < 1000; i++ ) buf += i;
      // single update of original counter when buf is destroyed
    }

The buffer behave exactly like the orginal counter, except it holds only a local
count if you would call `nbEntries` for example.


Histograms
----------

Histograms are only a special typeof counters. They live in namespace
``Gaudi::Accumulators`` (headers in ``Gaudi/Accumulators/`` directory).

There is a variety of different Histograms available to the end user,
depending on their content, behavior and need for runtime configuration
or not. Let's detail all of them

Histograms are templated
........................

Template parameters :

.. code-block:: cpp

    template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
              typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>

``ND``
    this is the dimension of the Histogram. Any dimension is supported in principle
    although only dimension up to 3 included can make it to disk or can be
    converted to ROOT histograms

``Atomicity``
    like any counters, thread safety is the default, but atomicity::none
    provides a faster, non thread safe version

``Arithmetic``
    again like any counter, the internal Arithmetic type, double as a default

``AxisTupleType``
    the set of Axis types for the histogram. Usually the default is all fine


Static or regular
.................

Every histogram type exists in 2 main versions, namely the Static and regular version.
E.g. ``StaticHistogram<1>`` vs ``Histogram<1>`` for the most basic 1D histogram.

The Static version (header ``Gaudi/Accumulators/StaticHistogram.h``)
is the prefered one but supposes knowledge of all parameters
at compile time. Here is a typical example :

.. code-block:: cpp

    #include <Gaudi/Accumulators/StaticHistogram.h>

    mutable Gaudi::Accumulators::StaticHistogram<1> m_hOccupancy{
        this, "HistoName", "The nice title",
        Gaudi::Accumulators::Axis<double>{ 100, 0., 1. } };

    void operator()() const override {
        ++m_hOccupancy[value];   // count occurrences of 'value'
    }

The constructor requires an owner (algorithm, service or tool), a name,
a title and a list of axis matching the dimension. The default Axis class is
constructed with number of bins, lower bound and upper bound plus optional
title and set of labels.
Once a StaticHistogram is declared, title and axis cannot be changed anymore

The non static version of Histograms (header ``Gaudi/Accumulators/Histogram.h``)
works similarly but allows to defer the creation of the histogram, and
thus the setting of title and axis to the initialization phase.
Typical usage looks like this :

.. code-block:: cpp

    #include <Gaudi/Accumulators/Histogram.h>

    mutable Gaudi::Accumulators::Histogram<1> m_hOccupancy{
        this, "HistoName" };
    Gaudi::Property<unsigned int> m_nbins{ this, "NBins", 100 };

    StatusCode initialize() {
      m_hOccupancy.setTitle( std::format( "My Histo with {} bins", m_nbins.value() ) );
      m_hOccupancy.setAxis<0>( { m_nbins.value(), 0., 1. } );
    }

    void operator()() const override {
        ++m_hOccupancy[value];   // count occurrences of 'value'
    }


Histogram creation time
.......................

Static Histograms are created when the constructor of the Algorithm /
Service / Tool is executed while non static ones will only be constructed
at the end of initialization by default and can thus use properties for
example, as shown above.

However, we can even delay further the construction of non static histograms
and make it on demand by passing extra "false" argument to their constructor
asking to no initialize by default and then calling ``createHistogram`` by hand.
Typical code looks like :

.. code-block:: cpp

    #include <Gaudi/Accumulators/Histogram.h>

    mutable Gaudi::Accumulators::Histogram<1> m_hOccupancy{
        this, "HistoName", "", {}, false }; // title and axis will be overwritten
    Gaudi::Property<unsigned int> m_nbins{ this, "NBins", 100 };
    Gaudi::Property<bool> m_buildHistos{ this, "BuildHistos", true };

    StatusCode initialize() {
      if ( m_buildHistos ) {
        m_hOccupancy.setTitle( std::format( "My Histo with {} bins", m_nbins.value() ) );
        m_hOccupancy.setAxis<0>( { m_nbins.value(), 0., 1. } );
        m_hOccupancy.createHistogram( this );
      }
    }

    void operator()() const override {
      if ( m_buildHistos ) {
        ++m_hOccupancy[value];   // count occurrences of 'value'
      }
    }

Once ``createHistogram`` has been called, no further changes of title of axis
can be done obviously. ``createHistogram`` can also be called in operator().
Take care though of the thread safety, as you can call it only once


Histogram Properties
....................

Non static histograms can be configured from python easily via Properties
automatically declared for each of them in their owner :

``<histo_name>_Title``
    where `<histo_name>` is the name of the histogram given to the C++ constructor
    (with all non alphanumeric characters replaced by `_` if necessary) allows to
    set the histogram title

``<histo_name>_Axis<n>``
    same thing for `<histo_name>` and `<n>` if the axis number starting from 0.
    It allows to define an Axis

Example code in C++ and the python configuration :

.. code-block:: cpp

    #include <Gaudi/Accumulators/Histogram.h>
    #include <Gaudi/Functional/Consumer.h>

    struct EventHisto final : Gaudi::Functional::Consumer<void(int n)> {
      using Consumer::Consumer;

      void operator()(int n) const override { ++m_hOccupancy[n]; }

      mutable Gaudi::Accumulators::Histogram<1> m_hOccupancy{
        this, "HistoName" };
    };
    DECLARE_COMPONENT( EventHisto )

.. code-block:: python

    from GaudiConfig2 import Configurables as C

    C.EventHisto( "MyhistoAlgo",
                  HistoName_Title = "title from python",
                  HistoName_Axis0 = (100, -5.0, 5.0) )


Histogram, WeightedHistogram, ProfileHistogram, RootHistogram
.............................................................

Histograms exist in 4 flavors, independently of their template parameters and
the fact that they are static or not :

``(Static)Histogram<...>``
    regular histogram, with a count of entries per bin.
    The code to add a new entry is ``++histo[n]`` in 1D or
    ``++histo[{i, j}]`` in 2D


``(Static)WeightedHistogram<...>``
    A weighted variant, where each entry in the bins has a weight
    The code to add a new entry is thus ``histo[n] += w`` in 1D
    and ``histo[{i, j}] += w`` in 2D

``(Static)ProfileHistogram<...>``
    A profile variant, keeping mean and spread of a quantity
    within each bin. The code to add in new entry is similar
    to the weighted case : ``histo[n] += value`` in 1D
    and ``histo[{i, j}] += value`` in 2D

``(Static)WeightedProfileHistogram<...>``
    A weighted profile variant, keeping mean and spread of a
    weighted quantity within each bin.
    The code to add in new entry is thus : ``histo[n] += { value, weight }`` in 1D
    and ``histo[{i, j}] += { value, weigth }`` in 2D


All previously described histograms only keep values per bin, with no
overall statistical data like overall mean of inserted values.
This can be problematic when converting these to ROOT histograms.
The missing values (sum, mean, sum of squares in 1D) can be approximated
but the original value is lost. Hence the extra

``(Static)RootHistogram<...>``
    A regular histogram keeping extra global statistics, namely
    - sum and sum of squares (sum2) in 1D
    - sumx, sumy, sumx2, sumy2, sumxy in 2D
    - sumx, sumy, sumz, sumx2, sumy2, sumz2, sumxy, sumxz, sumyz in 3D
    This allows to compute the different means, standard deviations or covariances
    Header ``Gaudi/Accumulators/(Static)RootHistogram.h`` is required.


HistogramArray, HistogramMap
............................

The Gaudi framework has direct support for arrays and maps of histograms
through the HistogramArray (header ``Gaudi/Accumulators/HistogramArray.h``) and
HistogramMap (header ``Gaudi/Accumulators/HistogramMap.h``) classes.

These are used as wrappers around std::array and std::map of histograms
sharing the same axis but with different names and titles potentially.
They are templated by the type of Histogram used :

.. code-block:: cpp

    template <typename Histo, std::size_t N>
    struct HistogramArray

    template <typename Key, typename Histo>
    struct HistogramMap


They have 2 constructors each allowing to adapt names and titles for each entry.
Here is the `HistogramArray` versions, the `HistogramMap` works the same with the
key type instead of integers :

.. code-block:: cpp

    template <typename OWNER>
    HistogramArray( OWNER* owner, std::string_view name, std::string_view title, AxisType... allAxis )

    template <typename OWNER, std::invocable<int> FormatName, std::invocable<int> FormatTitle>
    HistogramArray( OWNER* owner, FormatName&& fname, FormatTitle&& ftitle, AxisType... allAxis )

The first version requires an owner, name, title and axis. The name and title
will be used with std::format passing the index of the given histogram as first
and only parameter. Thus a string like ``"Title of Histo {}"`` will translate into
``"Title of Histo 0"``, ``"Title of Histo 1"``, ...

In the second version, fname and ftitle are callables taking an integer as input
and returning the title and name for the given histogram respectivly.

Here are some examples :

.. code-block:: cpp

   // Array of 5 1D histograms with simple names and titles
   // Names will be GaudiH1D-0, GaudiH1D-1, ... and similarly for titles
   Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::Histogram<1>, 5> histo1d{
     &algo, "GaudiH1D-{}", "A Gaudi 1D histogram - number {}", { 21, -10.5, 10.5, "X" } };
   ++histo1d[3][-10.0];

   // Array of 5 2D weighted histograms with simple names and titles
   // Names will be Name0, Name1, ... and similarly for titles
   Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::WeightedHistogram<2>, 7> histo2dw{
     &algo, "Name{}", "Title {}", { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } };
   histo2dw[1][{ -10.0, -10.0 }] += 0.25;

   // Array of histograms with custom name and titles
   // Names will be GaudiH1D-0-0, GaudiH1D-1-1, GaudiH1D-2-4, ...
   // Titles will be "Title 1 of 5", "Title 2 of 5", "Title 3 of 5", ...
   Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::Histogram<1>, 5> histoCustom{
     &algo,
     []( int n ) { return fmt::format( "GaudiH1D-{}-{}", n, n*n ); },
     [nb = 5]( int n ) {
       return fmt::format( "Title {} of {}", n+1, nb );
     },
     { 21, -10.5, 10.5, "X" } };
   ++histoCustom[2][-10.0];

   // Map of 1D histograms with simple names and titles, key being an int
   // Names will be GaudiH1D-N and similarly for titles where N is the key associated
   Gaudi::Accumulators::HistogramMap<int, Gaudi::Accumulators::Histogram<1>> histo1d{
     &algo, "GaudiH1D-{}", "A Gaudi 1D histogram - number {}", { 21, -10.5, 10.5, "X" } };
   ++histo1d[3][-10.0];

   // Map of 2D weighted histograms with simple names and titles, key being int
   // Names will be Name0, Name1, ... and similarly for titles
   Gaudi::Accumulators::HistogramMap<int, Gaudi::Accumulators::WeightedHistogram<2>> histo2dw{
     &algo, "Name{}", "Title {}", { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } };
   histo2dw[1][{ -10.0, -10.0 }] += 0.25;

   // Map of histograms with custom name and titles, key is a pair<int, string>
   // Names will be GaudiH1D-<N>-<S>, where the key was the pair (<n>, <S>)
   // Titles will be "Title <S> (<N>)"
   Gaudi::Accumulators::HistogramMap<std::pair<int, std::string>, Gaudi::Accumulators::Histogram<1>> histoCustom{
     &algo,
     []( std::pair<int, std::string> const& p ) { return fmt::format( "GaudiH1D-{}-{}", p.first, p.second ); },
     []( std::pair<int, std::string> const& p ) {
       return fmt::format( "Title {} ({})", p.first, p.second );
     },
     { 21, -10.5, 10.5, "X" } };
   ++histoCustom[{3, "three"}][-10.0];


Persistency
----------------------------

All counters and thus histograms are automatically registered to the
``Gaudi::Monitoring::Hub`` by their owner component. At regular points
of the finite state machine — and at finalization — the hub collects
all entities and forwards them to the registered *sinks* (a JSON dump
for the counters, a json summary plus a ROOT file for histograms, ...).
Which sinks are active is a job configuration option, described in
:ref:`dev-logging`. User code does not write histograms to files by
itself.


Legacy interfaces
-----------------

* The ``IHistogramSvc``-based booking API of the old guide still works
  for historic code but should not be used for new code: the
  corresponding histograms are not covered by the monitoring machinery
  and the interface ties user code to ROOT.
* ``GaudiAlg``-style histogram helpers (``book``, ``plot``...) are
  likewise legacy.
