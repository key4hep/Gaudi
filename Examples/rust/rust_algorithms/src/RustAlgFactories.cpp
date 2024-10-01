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
#include <example_rust_algorithm_bridge/lib.h>

DECLARE_FACTORY_WITH_ID( Gaudi::Rust::AlgWrapper, "Gaudi::Examples::MyRustCountingAlg",
                         ( []( std::string const& name, ISvcLocator* svcLoc ) -> auto {
                           return std::make_unique<Gaudi::Rust::AlgWrapper>( name, svcLoc,
                                                                             my_rust_counting_alg_factory() );
                         } ) )
