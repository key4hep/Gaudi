/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <Gaudi/Rust/AlgWrapper.h>

DECLARE_RUST_ALG( my_rust_counting_alg_factory, "Gaudi::Examples::MyRustCountingAlg" )

DECLARE_RUST_ALG( int_producer_factory, "Gaudi::Examples::RustAlgorithms::IntDataProducer" )

DECLARE_RUST_ALG( i2f_factory, "Gaudi::Examples::RustAlgorithms::IntToFloatData" )

DECLARE_RUST_ALG( float_consumer_factory, "Gaudi::Examples::RustAlgorithms::FloatDataConsumer" )
