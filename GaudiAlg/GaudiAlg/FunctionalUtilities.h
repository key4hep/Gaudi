#ifndef FUNCTIONAL_UTILITIES_H
#define FUNCTIONAL_UTILITIES_H

#include <initializer_list>
#include <string>
#include "boost/algorithm/string/join.hpp"

// TODO: fwd declare instead?
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/AnyDataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiHistoAlg.h"

namespace Gaudi { namespace Functional {

   // This utility is needed when the inputs of a functional algorithm may be stored in several locations
   inline std::string concat_alternatives(std::initializer_list<std::string> c) {
      return boost::algorithm::join(c,"&");
   }

namespace Traits {

   // traits classes used to customize Transformer and FilterPredicate
   // Define the classes to to be used as baseclass, in- or output hanldes.
   // In case a type is not specified in the traits struct, a default is used.
   //
   // The defaults are:
   //
   //      using BaseClass = GaudiAlgorithm
   //      template <typename T> using InputHandle = DataObjectHandle<T>;
   //      template <typename T> using OutputHandle = DataObjectHandle<T>;
   //

   // this uses the defaults -- and it itself is the default ;-)
   struct useDefaults {
   };

   // this example uses AnyDataHandle as input and output, and the default BaseClass
   struct useAnyDataHandle {
       template <typename T> using InputHandle = AnyDataHandle<T>;
       template <typename T> using OutputHandle = AnyDataHandle<T>;
   };

   // this example uses GaudiHistoAlg as baseclass, and the default handle types for
   // input and output
   struct useGaudiHistoAlg {
       using BaseClass = GaudiHistoAlg;
   };

}

}}

#endif
