# Implement a Service in Rust
This is an extension of the example
"[Invoke a Rust function from C++ code](../invoke_rust_function/README.md)",
so we take as granted what is already explained there.

## Use case
Although it's relatively easy to implement a Gaudi service that uses functions
from Rust (see the dedicated example), it might be more practical to implement
the whole service logic in Rust.

## Implementation
In this example we are invoking Rust code from C++ (factory function to create
the service object, the service methods), but also C++ code from Rust (Service
base class methods, message reporting functions), so we cover bidirectional
bindings.

### The C++ side
To implement a Gaudi component, such as a Service, we need to register a
factory function to the PluginService. This is done via the preprocessor
macro `DECLARE_COMPONENT` in a translation unit that is compiled and linked
via the CMake helper function `gaudi_add_module`.

Gaudi components rely on virtual methods to access the various functionalities
of the component. In particular, for services, we ned to create an instance of a
class that inherits from `Service` (base class for all services) and one or more
abstract interfaces (APIs for the specific service). Clients of a service will
get a pointer to the abstract interface that they can use to access the desired
functionalities.

For this example we defined the interface `Gaudi::Examples::IKeyValueStore` that
provides the method `std::optional<std::string> get(std::string_view key) const`
to extract a value from the store given a key.

To simplify the integration with the plugin service we added a few of bridge
classes in C++.

`Gaudi::Rust::ServiceBridge<T>` is a `Service` specialization that uses the
PIMPL pattern to expose to C++ the few basic Service methods implemented in a
Rust type. The class is generic on the underlying Rust type that implements a
service. The instance of the Rust service is create by a partial specialization
of `ServiceBuilder<T>` and stored in a `rust::Box<T>` data member. The specific
partial specialization of `ServiceBuilder<T>` is defined in a translation unit
that is linked against the Rust crate containing the service definition and code
and calls a factory function exposed by the Rust crate. The Rust factory
function takes a reference to the C++ `Service` as a mean to provide hooks to
connect the Rust service implementation to Gaudi C++ facilities.

`Gaudi::Rust::IKeyValueStoreBridge<T>` extends `ServiceBridge<T>` adding the
method needed to comply with the abstract interface `IKeyValueStore`. A specific
template instantiation if `IKeyValueStoreBridge<T>` is declared to the plugin
service and its instance is what clients will get and talk to. Since the cxx
crate does not support yet `std::optional<T>` we have to use we have to add one
extra level of indirection, so `optional<string> IKeyValueStoreBridge<T>::get(string_view) const `
delegates to `T::get(&str) -> OptString`, where `OptString` is a type
shared between C++ and Rust that can be constructed in Rust from an
`Option<String>` and converted in C++ to a `std::optional<std::string>`.

`Gaudi::Rust::Logger` is a C++ wrapper that exposes the message reporting
features of a Gaudi component to Rust code. It presents itself as an opaque C++
object (wrapped in a `std::unique_ptr`) with a few methods that take a Rust
`&str` and print it through Gaudi `MessageSvc` using the matching methods of the
C++ component used to construct the `Logger` (in this case the `Service`
instance the wraps the Rust service implementation).

### The Rust library (AKA crate)
The crate used in this example defines a struct `DummyKvs` that holds a
reference to the C++ `Service` instance that wraps the `DummyKvs` instance (this
is a bit unsafe, but the borrow checker cannot really trace life time boundaries
through the C++ interface) and a `UniquePtr<Logger>` (Rust wrapper around
`std::unique_ptr`).

We implement `DummyKvs` methods `initialize`, `start`, `stop` and `finalize`
that are invoked by `ServiceBridge<T>`, as well as `get` for
`IKeyValueStoreBridge<T>`. We also implement `info` to streamline invocation of
`Logger::info` and `get_internal` that returns `Option<String>` and is wrapped
by `get` to convert the returned value to `OptString`.

In the `cxx::bridge` we declare `OptString` as a shared type and the functions
that will work as bridge between C++ and Rust, such as `Logger::info` or
`DummyKvs::initialize`.

It would be nice to use traits and blanket implementations (Rust way to provide
a functionality equivalent to C++ base classes) to help developing custom
services, otherwise we have to explicitly write a lot of boilerplate.
Unfortunately the cxx crate does not allow it (see
[cxx#1232](https://github.com/dtolnay/cxx/issues/1232)). It's possible to to
work around the limitation and we will revisit the implementation in the future.

### CMake
The main change with respect to what is done in the other examples is that
building a Rust static library that has to invoke C++ code requires that we
explicitly tell CMake to enable "Position Independent Code" (`-fPIC`).

### Testing
To verify that the service we developed in Rust work as expected, we added a C++
implementation of `IKeyValueStore` and an algorithm that queries an instance of
the service at every event, printing the value obtained for a predefined key.
Then we added a test that runs a Gaudi job with two instances of such algorithm,
one configured to use the C++ service and the other to use the Rust service.
