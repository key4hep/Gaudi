#ifndef FUNCTIONAL_UTILITIES_H
#define FUNCTIONAL_UTILITIES_H

#include <initializer_list>
#include <string>
#include "boost/algorithm/string/join.hpp"

// TODO: fwd declare instead?
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/AnyDataHandle.h"
#include "GaudiKernel/SerializeSTL.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiHistoAlg.h"

namespace Gaudi { namespace Functional {

   // This utility is needed when the inputs of a functional algorithm may be stored in several locations
   inline std::string concat_alternatives(std::initializer_list<std::string> c) {
      return boost::algorithm::join(c,":");
   }

   template <typename... Strings>
   std::string concat_alternatives(const Strings& ...s ) {
      return concat_alternatives(std::initializer_list<std::string>{ s... });
   }

   inline void updateHandleLocation(IProperty& parent, const std::string& prop, const std::string& newLoc) {
        auto sc = parent.setProperty(prop,newLoc);
        if (sc.isFailure()) throw GaudiException("Could not set Property",prop + " -> " + newLoc ,sc);
    }

   inline void updateHandleLocations(IProperty& parent, const std::string& prop, const std::vector<std::string>& newLocs) {
        std::ostringstream ss;
        GaudiUtils::details::ostream_joiner( ss << '[', newLocs, ", " ,
                                             [](std::ostream& os, const auto& i)
                                             -> auto& { return os << "'" << i << "'"; } ) << ']';
        auto sc = parent.setProperty(prop,ss.str());
        if (sc.isFailure()) throw GaudiException("Could not set Property",prop + " -> " + ss.str(),sc);
    }

   [[deprecated("please use updateHandleLocation instead")]]
   inline void updateReadHandleLocation(IProperty& parent, const std::string& prop, const std::string& newLoc)
   { return updateHandleLocation(parent,prop,newLoc); }

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
