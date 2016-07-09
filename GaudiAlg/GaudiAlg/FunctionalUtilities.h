#ifndef FUNCTIONAL_UTILITIES_H
#define FUNCTIONAL_UTILITIES_H

#include <initializer_list>
#include <string>
#include "boost/algorithm/string/join.hpp"

// TODO: fwd declare instead?
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/AnyDataHandle.h"

namespace Gaudi { namespace Functional {

   // This utility is needed when the inputs of a functional algorithm may be stored in several locations
   inline std::string concat_alternatives(std::initializer_list<std::string> c) {
      return boost::algorithm::join(c,"&");
   }


   // traits classes used to customize Transformer and FilterPredicate

   struct useDataObjectHandle {
       template <typename T> using InputHandle = DataObjectHandle<T>;
       template <typename T> using OutputHandle = DataObjectHandle<T>;
   };
   struct useAnyDataHandle {
       template <typename T> using InputHandle = AnyDataHandle<T>;
       template <typename T> using OutputHandle = AnyDataHandle<T>;
   };
}}

#endif
