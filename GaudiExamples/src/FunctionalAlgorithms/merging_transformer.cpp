#include "GaudiAlg/MergingTransformer.h"
#include <string>

using ints              = Gaudi::Functional::vector_of_const_<int>;
using out_t             = std::tuple<float, float>;
using is2ff_merger_base = Gaudi::Functional::MergingMultiTransformer<out_t( ints const& )>;

struct is2ff_merger : public is2ff_merger_base {
  is2ff_merger( std::string const& name, ISvcLocator* pSvcLocator )
      : is2ff_merger_base( name, pSvcLocator, {"InputInts", {"firstInt", "secondInt"}},
                           {KeyValue{"O1", "firstFloat"}, KeyValue{"O2", "secondFloat"}} ) {}

  out_t operator()( ints const& is ) const override {
    float f1 = 1, f2 = 1;

    for ( auto i : is ) {
      info() << "i: " << i;
      f1 *= i;
      f2 *= 1.f / i;
    }
    info() << endmsg;
    return out_t{f1, f2};
  }
};

DECLARE_COMPONENT( is2ff_merger )
