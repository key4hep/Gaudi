## Invoke a rust function from C++ code

### Use case
Some functionality is available in a Rust crate that does not provide a C API or
we want to implement some new functionality using Rust, may be to leverage on
some existing Rust crate.

### Implementation
For Rust (or any other language) we can use C bindings or C++ bindings if the
language supports them. The easiest option is to use
[Corrosion](https://corrosion-rs.github.io/corrosion) and
[cxx](https://cxx.rs/).

In this example we use an algorithm to invoke some Rust code, but the
procedure is applicable to services or tools.
