// Dear emacs, this is -*- c++ -*-
#ifndef GAUDIUTILS_CREATEGUIDASSTRING_H
#define GAUDIUTILS_CREATEGUIDASSTRING_H

// System include(s):
#include <string>

namespace Gaudi
{

  /// Helper function creating file identifier using the UUID mechanism
  std::string createGuidAsString();

} // namespace Gaudi

#endif // GAUDIUTILS_CREATEGUIDASSTRING_H
