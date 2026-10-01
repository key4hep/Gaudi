.. include:: ../old/global.rst

|newpage|

.. _dev-tools:

Tools and ToolSvc
=================

A *tool* (interface ``IAlgTool``) is a component that packages reusable
functionality for algorithms: a vertex fitter, a tool producing random
numbers, etc. Tools are managed, instantiated and retrieved by the
framework (through the ``ToolSvc`` service); user code accesses them
via handles or, in the functional framework, as plain function
arguments.

What has changed with respect to the legacy guide:

* public tools are now accessed through a ``ToolHandle<T>`` (or
  consumed directly as an argument of a functional algorithm);
* **private tools are discouraged** — they make configuration and state
  ownership hard to track; they still exist for legacy code;
* the ``AlgTool`` helper base class is built on legacy idioms: whether a
  class can be used as a tool only depends on it implementing the
  requested ``IAlgTool`` interface, and any regular
  ``Gaudi::Algorithm`` can host tools.

Using a tool in an algorithm: ToolHandle
----------------------------------------

In a class-based algorithm, declare a member ``ToolHandle<T>`` where
``T`` is the tool interface. The handle auto-declares itself as a
property:

.. code-block:: cpp

    #include <GaudiKernel/ToolHandle.h>

    class MyAlg : public Gaudi::Algorithm {
      ...
      // "MyTool" is the option name, "MyExampleTool" the default type/name
      ToolHandle<IMyTool> m_tool{ this, "MyTool", "MyExampleTool" };
    };

    ... = m_tool->doSomething();   // dereference to use the tool

The ``type/name`` of the bound tool can then be changed from the job
options (``C.MyAlg(MyTool="MyExampleTool/Special")``), and the
framework retrieves the tool instance during initialization. A list of
tools is declared with ``ToolHandleArray<T>``.

Public vs private tools
^^^^^^^^^^^^^^^^^^^^^^^

A *public* tool is a shared, named instance retrieved through
``ToolSvc`` and configured from the job options by every component that
uses it. A *private* tool (``parent`` set to the owning algorithm)
belongs to a single algorithm instance. As private tools duplicate
configuration surface and hide state, prefer public tools; private ones
exist for backward compatibility only.

Tools as inputs of functional algorithms
----------------------------------------

In the functional framework, a tool the algorithm needs is declared in
the algorithm's *signature* and appears as a const reference argument —
no handle, no retrieval, and the framework knows the dependency:

.. code-block:: cpp

    // adapted from GaudiTestSuite/src/FunctionalAlgorithms/MakeAndConsume.cpp
    struct ToolConsumer final
        : Gaudi::Functional::Consumer<void( IMyTool const& ), BaseClass_t> {
      ToolConsumer( const std::string& name, ISvcLocator* svcLoc )
          : Consumer( name, svcLoc, { "MyTool", "MyExampleTool" } ) {}

      void operator()( IMyTool const& tool ) const override { tool(); }
    };
    DECLARE_COMPONENT( ToolConsumer )

``"MyTool"`` is the name of the property the algorithm constructor
creates (settable from job options as any tool location), and
``"MyExampleTool"`` the default tool type/name.


Writing a tool
--------------

A tool is a component implementing an interface deriving from
``IAlgTool``:

.. code-block:: cpp

    struct IMyTool : extend_interfaces<IAlgTool> {
      DeclareInterfaceID( IMyTool, 1, 0 );
      virtual void operator()() const = 0;
    };

    struct MyExampleTool : extends<AlgTool, IMyTool> {
      using extends::extends;
      void operator()() const override { always() << m_message.value() << endmsg; }
      Gaudi::Property<std::string> m_message{ this, "Message", "Boring Default Message" };
    };
    DECLARE_COMPONENT( MyExampleTool )

(The ``extends<AlgTool, ...>`` base is kept for convenience of legacy
tools; a plain class implementing the interface and registered with
``DECLARE_COMPONENT``/the plugin machinery is equally a tool.)

Tools may consume data themselves, through the
``Gaudi::Functional::ToolBinder<>`` template: the binder inherits from
``AlgTool``, declares the tool's data inputs, and at call time builds a
*bound instance* implementing the interface, capturing the event data.
See ``MakeAndConsume.cpp`` in ``GaudiTestSuite/src/FunctionalAlgorithms``
for a complete example.

Tools of tools / tools of services
----------------------------------

Handles work identically in all component types: a service may declare
a ``ToolHandle``, a tool may use another tool, and an algorithm may
host sub-tools — the older rule that tools are "created and owned by
``ToolSvc`` with algorithms as parents" is reduced to the mechanics
described above.
