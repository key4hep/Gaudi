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

   template <typename... Strings>
   std::string concat_alternatives(const Strings& ...s ) {
      return concat_alternatives(std::initializer_list<std::string>{ s... });
   }

   inline void updateReadHandleLocation(Algorithm& parent, const std::string& prop, const std::string& newLoc) {
        // "parse" the current text representation of the datahandle,
        // and then update the first token (the default location).
        // Ugly, but I don't see any better way of doing this...
        // underlying problem is the "syntax" of the text representation
        // of a datahandle property...
        std::string s;
        if (!parent.getProperty(prop,s))
            throw GaudiException( parent.name() + " does not have the requested property \"" + prop + "\"",
                                  parent.name(), StatusCode::FAILURE );
        s.replace(0,s.find("|"),newLoc); // tokens are seperated by |
        parent.setProperty(prop,s).ignore(); // first token is the default location
    }

namespace Traits {

   // traits classes used to customize Transformer and FilterPredicate
   // Define the types to to be used as baseclass, and as in- resp. output hanldes.
   // In case a type is not specified in the traits struct, a default is used.
   //
   // The defaults are:
   //
   //      using BaseClass = GaudiAlgorithm
   //      template <typename T> using InputHandle = DataObjectHandle<T>;
   //      template <typename T> using OutputHandle = DataObjectHandle<T>;
   //

   // the best way to 'compose' traits is by inheriting them one-by-one...
   template <typename ... Base> struct use_ : Base... {};

    // helper classes one can inherit from to specify a specific trait
   template <typename Base>
   struct BaseClass_t { using BaseClass = Base; };

   template <template<typename> class Handle>
   struct InputHandle_t { template <typename T> using InputHandle = Handle<T>; };

   template <template<typename> class Handle>
   struct OutputHandle_t { template <typename T> using OutputHandle = Handle<T>; };

   // this uses the defaults -- and it itself is the default ;-)
   using useDefaults = use_<>;

   // example: use AnyDataHandle as input and output, and the default BaseClass
   using useAnyDataHandle = use_< InputHandle_t<AnyDataHandle>
                                , OutputHandle_t<AnyDataHandle> > ;

   // this example uses GaudiHistoAlg as baseclass, and the default handle types for
   // input and output
   using useGaudiHistoAlg = use_< BaseClass_t<GaudiHistoAlg> >;

}

}}

#endif
