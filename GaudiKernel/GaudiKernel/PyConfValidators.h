#ifndef PYCONF_VALIDATOR_MODULE
#error This header is meant only to be used in the implementation of validators for configurables
#endif

// This must be included before GCC standard headers to avoid warnings about
// redefined macros in Python.
#include <boost/python.hpp>

// Property classes/headers
#include "GaudiKernel/Property.h"

// Checker functions
bool check(const std::string &) {
  return true;
}
template <class TYPE>
bool check(const std::string &s) {
  TYPE tmp ;
  return Gaudi::Parsers::parse(tmp, s).isSuccess();
}
template <class TYPE, class VERIFIER>
bool check(const std::string &s) {
  TYPE tmp ;
  return Gaudi::Parsers::parse(tmp, s).isSuccess()
         && VERIFIER().isValid(&tmp);
}
