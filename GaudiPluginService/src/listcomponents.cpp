/*****************************************************************************\
* (c) Copyright 2013 CERN                                                     *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "LICENCE".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

/// @author Marco Clemencic <marco.clemencic@cern.ch>

#include <iostream>
#include <string>
#include <set>

#include <getopt.h>
#include <dlfcn.h>

#include <Gaudi/PluginService.h>

int main(int argc, char* argv[]) {
  Gaudi::PluginService::Details::Registry &reg =
      Gaudi::PluginService::Details::Registry::instance();
  typedef Gaudi::PluginService::Details::Registry::KeyType key_type;

  // cache to keep track of the loaded factories
  std::map<key_type, std::string> loaded;
  {
    // initialize the local cache
    std::set<key_type> base = reg.loadedFactories();
    for (std::set<key_type>::const_iterator f = base.begin(); f != base.end(); ++f)
    {
      loaded.insert(std::make_pair(*f, std::string("<preloaded>")));
    }
  }

  // loop over the list of libraries passed on the command line
  for (int i = 1; i < argc; ++i) {

    if (dlopen(argv[i], RTLD_LAZY | RTLD_LOCAL)) {

      std::set<key_type> factories = reg.loadedFactories();
      std::set<key_type>::const_iterator f;
      for (f = factories.begin(); f != factories.end(); ++f) {
        if (loaded.find(*f) == loaded.end())
        {
          std::cout << argv[i] << ":" << *f << std::endl;
          loaded[*f] = argv[i];
        }
        else
          std::cerr << "WARNING: factory '" << *f
                    << "' already found in " << loaded[*f]
                    << std::endl;
      }

    } else {
      std::cerr << "ERROR: failed to load " << argv[i] << std::endl;
    }
  }

  return 0;
}
