/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Functional/MergingTransformer.h>
#include <string>

using ints                     = Gaudi::Functional::vector_of_const_<int>;
using is2f_merger_base         = Gaudi::Functional::MergingTransformer<float( ints const&, int const& )>;
using is_merger_consumer_base  = Gaudi::Functional::MergingConsumer<void( ints const&, int const& )>;
using out_t                    = std::tuple<float, float>;
using is2ff_merger_base        = Gaudi::Functional::MergingMultiTransformer<out_t( ints const& )>;
using is2ff_merger_scalar_base = Gaudi::Functional::MergingMultiTransformer<out_t( ints const&, int const& )>;
using is2ff_merger_filter_base = Gaudi::Functional::MergingMultiTransformerFilter<out_t( ints const& )>;
using is2ff_merger_filter_scalar_base =
    Gaudi::Functional::MergingMultiTransformerFilter<out_t( ints const&, int const& )>;
using filter_out_t = std::tuple<bool, float, float>;

struct is2f_merger_with_scalar : public is2f_merger_base {
  is2f_merger_with_scalar( std::string const& name, ISvcLocator* pSvcLocator )
      : is2f_merger_base( name, pSvcLocator, { { "InputInts", { "firstInt", "secondInt" } }, { "Scale", "scale" } },
                          { "Output", "scaledSum" } ) {}

  float operator()( ints const& is, int const& scale ) const override {
    auto sum = 0.f;
    info() << "input locations: " << inputLocation( 0 ) << ", " << inputLocation<1>() << endmsg;
    info() << "input location sizes: " << inputLocationSize( 0 ) << ", " << inputLocationSize( 1 ) << endmsg;
    for ( auto i : is ) {
      info() << "i: " << i << " ";
      sum += i;
    }
    info() << "scale: " << scale << endmsg;
    return sum * scale;
  }
};

DECLARE_COMPONENT( is2f_merger_with_scalar )

struct is_merger_consumer_with_scalar : public is_merger_consumer_base {
  is_merger_consumer_with_scalar( std::string const& name, ISvcLocator* pSvcLocator )
      : is_merger_consumer_base( name, pSvcLocator,
                                 { { "InputInts", { "firstInt", "secondInt" } }, { "Scale", "scale" } } ) {}

  void operator()( ints const& is, int const& scale ) const override {
    auto sum = 0;
    for ( auto i : is ) { sum += i; }
    info() << "consumed scaled sum: " << sum * scale << " from " << inputLocation( 0 ) << " and " << inputLocation<1>()
           << endmsg;
  }
};

DECLARE_COMPONENT( is_merger_consumer_with_scalar )

struct is2ff_merger : public is2ff_merger_base {
  is2ff_merger( std::string const& name, ISvcLocator* pSvcLocator )
      : is2ff_merger_base( name, pSvcLocator, { "InputInts", { "firstInt", "secondInt" } },
                           { { "O1", "firstFloat" }, { "O2", "secondFloat" } } ) {}

  out_t operator()( ints const& is ) const override {
    float f1 = 1, f2 = 1;

    for ( auto i : is ) {
      info() << "i: " << i << " ";
      f1 *= i;
      f2 *= 1.f / i;
    }
    info() << endmsg;
    return { f1, f2 };
  }
};

DECLARE_COMPONENT( is2ff_merger )

struct is2ff_merger_with_scalar : public is2ff_merger_scalar_base {
  is2ff_merger_with_scalar( std::string const& name, ISvcLocator* pSvcLocator )
      : is2ff_merger_scalar_base( name, pSvcLocator,
                                  { { "InputInts", { "firstInt", "secondInt" } }, { "Scale", "scale" } },
                                  { { "O1", "firstFloat" }, { "O2", "secondFloat" } } ) {}

  out_t operator()( ints const& is, int const& scale ) const override {
    float sum = 0, product = 1;

    for ( auto i : is ) {
      info() << "i: " << i << " ";
      sum += i;
      product *= i;
    }
    info() << "scale: " << scale << endmsg;
    return { sum * scale, product * scale };
  }
};

DECLARE_COMPONENT( is2ff_merger_with_scalar )

struct is2ff_merger_filter : public is2ff_merger_filter_base {
  is2ff_merger_filter( std::string const& name, ISvcLocator* pSvcLocator )
      : is2ff_merger_filter_base( name, pSvcLocator, { "InputInts", { "firstInt", "secondInt" } },
                                  { { "O1", "firstFloat" }, { "O2", "secondFloat" } } ) {}

  filter_out_t operator()( ints const& is ) const override {
    float f1 = 1, f2 = 1;

    for ( auto i : is ) {
      info() << "i: " << i << " ";
      f1 *= i;
      f2 *= 1.f / i;
    }
    info() << endmsg;
    auto filter_passed = f1 > 10;
    info() << "Filter " << ( filter_passed ? "passed" : "failed" ) << endmsg;
    return { filter_passed, f1, f2 };
  }
};

DECLARE_COMPONENT( is2ff_merger_filter )

struct is2ff_merger_filter_with_scalar : public is2ff_merger_filter_scalar_base {
  is2ff_merger_filter_with_scalar( std::string const& name, ISvcLocator* pSvcLocator )
      : is2ff_merger_filter_scalar_base( name, pSvcLocator,
                                         { { "InputInts", { "firstInt", "secondInt" } }, { "Scale", "scale" } },
                                         { { "O1", "firstFloat" }, { "O2", "secondFloat" } } ) {}

  filter_out_t operator()( ints const& is, int const& scale ) const override {
    float product = 1, inverse = 1;

    for ( auto i : is ) {
      info() << "i: " << i << " ";
      product *= i;
      inverse *= 1.f / i;
    }
    info() << "scale: " << scale << endmsg;
    product *= scale;
    inverse *= scale;
    auto filter_passed = product > 10;
    info() << "Filter " << ( filter_passed ? "passed" : "failed" ) << endmsg;
    return { filter_passed, product, inverse };
  }
};

DECLARE_COMPONENT( is2ff_merger_filter_with_scalar )
