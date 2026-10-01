.. include:: ../old/global.rst

|newpage|

.. _dev-ntuples:

NTuple and event collection facilities
======================================

This chapter describes the modern way of writing ntuples (ROOT TTrees)
from an algorithm. The old NTuple classes (``NtuplePtr``, the
``NTupleSvc``-based booking of the legacy guide) still exist for
backward compatibility but should not be used in new code.

The new interface lives in ``Gaudi/NTuple/Writer.h`` (in the GaudiUtils
library) and provides three building blocks, all layered on top of the
functional framework (see :ref:`dev-algorithms`).

``Gaudi::NTuple::Writer``
-------------------------

For the common case of **one ntuple entry per event**: the class
inherits from a *signature* describing the input data and the ntuple
columns, and only implements a ``transform`` function converting the
inputs into a tuple of column values:

.. code-block:: cpp

    #include <Gaudi/NTuple/Writer.h>

    struct MyWriter final
        : Gaudi::NTuple::Writer<std::tuple<int, std::size_t, float>( const std::vector<int>& )> {
      MyWriter( const std::string& name, ISvcLocator* svcLoc )
          : Writer( name, svcLoc, { "InputLocation", "MyVector" } ) {}

      std::tuple<int, std::size_t, float> transform( const std::vector<int>& v ) const override {
        return { std::accumulate( v.begin(), v.end(), 0 ), v.size(), m_rand() };
      }
    };
    DECLARE_COMPONENT( MyWriter )

``Gaudi::NTuple::SimpleWriter``
-------------------------------

When the number of entries per event is not fixed, inherit from
``Gaudi::NTuple::SimpleWriter`` and call ``fillTree`` as many times as
entries are needed::

    struct MySimpleWriter final
        : Gaudi::NTuple::SimpleWriter<std::tuple<int, std::size_t, float>( const std::vector<int>& )> {
      using SimpleWriter::SimpleWriter;
      void operator()( const std::vector<int>& v ) const override {
        for ( std::size_t i = 0; auto const& x : v ) { fillTree( { x, i++, 0.5f } ); }
      }
    };

``fillTree`` is internally synchronized, hence thread-safe.

The properties of both writer classes are:

``OutputFile``
    the identifier of the TFile in the ``FileSvc`` to write to;

``NTupleName``
    the name of the written TTree (a ``/`` in the name creates
    directories in the file);

``BranchNames``
    the names of the ntuple columns, one per column type of the
    signature tuple. Array properties are set from the job options
    with lists.

``Gaudi::NTuple::Wrapper``
--------------------------

The lower-level building block, wrapping a TTree: ``initTree`` creates
the branches from a list of names, ``fillTree`` appends an entry
(thread-safe), ``writeTree`` persists the tree. Together with
``Gaudi::NTuple::FileHolder`` (a mixin that adds the ``OutputFile``
property and FileSvc handling to an algorithm), it allows several
NTuples per algorithm.

.. code-block:: cpp

    Gaudi::NTuple::Wrapper<int, float> m_ntuple;
    ...
    m_ntuple.initTree( file, name(), "MyNtuple", { "a", "b" } );
    ...
    m_ntuple.fillTree( { 1, 2.5f } );
    ...
    m_ntuple.writeTree( file, name() );


Opening the output file
-----------------------

The output file is opened through the ``FileSvc`` service, so that it
is shared with other components (histogram sinks may write to the same
file — see :ref:`dev-logging`):

.. code-block:: python

    C.FileSvc(Config={"NTuple": "myfile.root?mode=recreate"})

with ``OutputFile="NTuple"`` (the default) on the writer algorithm,
and the service and the writer in the job's ``ExtSvc``/``TopAlg``.

Event collections and persistency of event data
-----------------------------------------------

As discussed in :ref:`dev-dataaccess`, Gaudi no longer ships a generic
"persistent event store": writing event data collections is the job of
dedicated, experiment-specific algorithms.
