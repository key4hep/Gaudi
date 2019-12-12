# GaudiConfig2 documentation  {#GaudiConfig2-readme}

## Rationale
After many years of experience with Gaudi Python based configuration, and with
the time window of the CERN LHC Second Long Shutdown (LS2), it's time to
review the implementation of the Python and C++ code we used as backbone for
Python based configuration.

What is required from a new implementation is:

- precise and punctual validation of configuration parameters (properties)
- extensibility (allow user defined semantics for user defined property types
  without the need of changes in core C++ code)
- intuitive behaviour, in particular no implicit _named_ singletons
  (AKA *global configurables registry*)

## Design principles
### Information flow
Properties exist in two domains (C++ and Python) which should interoperate
exchanging informations.

The following points describe the main steps of a property lifetime:

- a property is declared in C++ as data members of component classes using
  `Gaudi::Property<T>`
- a C++ tool (`genconf`) scans the component class looking from properties and
  generates files with details about each property of each component class found
- Python facade classes to component classes are generated from the details
  collected by the C++ tool
- users define the configuration of a Gaudi application creating instances of
  such Python facade classes and assigning values to the properties
- the Gaudi application bootstrap executes user Python configuration code to
  collect all user set properties
- all user set properties are passed as strings to a dedicated service inside
  the Gaudi application
- the required C++ components are instatiated by the framework and initialized
  setting the values of the properties from their string representation

For the exchange of information to work, these operations must be correctly
defined for a property:

- conversion for C++ default value to its Python version
- validation of user input
- conversion from a Python value object to a string that can be parsed to the
  corresponding C++ value

For improved usability, there is no need that the internal storage of the Python
value matches the string representation to be passed to C++ for parsing.  Take
for example the case of a string property meant to hold _type/name_ of a Gaudi
`Service` (or `AlgTool`) instance; we can easily envisage the internal storage
to be the instance of the Configurable class matching the requests `Service`
type, but the Python property to be assigned from a _type/name_ string or from
the configurable instance (we can extend the type checking to ensure that the
interface implemented by the service matches the requirements from C++), and
only when converting to string during the export to C++ it is mapped to the
correct _type/name_ string.

Implementing the semantics in terms of

- what the user uses to set a value
- what the user sees when getting the value
- what C++ gets as string representation of the value
- what to do with values when merging configurations

allow for creation of very powerful and easy to use interfaces to user options.

### Configurables database
Another aspect to consider is how users access and instantiate the Python
facade classes.

C++ components come from different libraries in different subdirectories of all
projects, and it's often difficult to a component name to the library containing
it.

To simplify user life we can use a database of components that records the
component fully qualified name, the type of component and its properties (with
their types and semantics).

For a more _Pythonic_ access to the facade classes, the C++ namespaces can be
mapped to Python modules, via _fake_ modules that hide the lookup in the
database and create the class objects on demand.  Of course, helper functions
to get a facade class via the fully qualified C++ name can be provided too.

The database can be extended to record all kind of information we want to access
from Python.  For example, we record the list of interfaces implemented by the
components, so that they can be used in the type checking of property values.

### Stacking configuration files
A typical use case for the configuration of a Gaudi application is to start from
a predefined configuration and change some parameters (for examples the
verbosity level, or the number of events to process).

Instances of facade classes are, by default, not participating in the
configuration of the application, and to enamble them they must be given a name,
which is required by Gaudi framework to instantiate the corresponding C++
classes. Once a name is used by an instance, it cannot be used by another, but
there must be a way to get the instance with a given name (it is also useful
to allow automatic creation of a new instance if the name is not used yet,
similar to the behaviour of Python's `dict.get` method).

Retrieving either a named instance is a way of overriding its settings.  Another
way would be to keep unnamed instances in shared Python modules, to be then
given a name to enter the actual final configuration.

Means to merge unnamed or named instances can be also provided.

To be noted that some property semantics may required an instance to have a
name, for example to add an algorithm to the list of executed algorithms.
