#ifndef PYCONF_VALIDATOR_MODULE
#error This header is meant only to be used in the implementation of validators for configurables
#endif
// Checker functions
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
