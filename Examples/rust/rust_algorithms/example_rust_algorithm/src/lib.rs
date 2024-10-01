// #####################################################################################
// # (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
// #                                                                                   #
// # This software is distributed under the terms of the Apache version 2 licence,     #
// # copied verbatim in the file "LICENSE".                                            #
// #                                                                                   #
// # In applying this licence, CERN does not waive the privileges and immunities       #
// # granted to it by virtue of its status as an Intergovernmental Organization        #
// # or submit itself to any jurisdiction.                                             #
// #####################################################################################
extern crate gaudi_rust_bindings;
use gaudi_rust_bindings::*;
use std::ops::Deref;
use std::sync::atomic::{AtomicUsize, Ordering::Relaxed};

struct CounterAlg {
    counter: AtomicUsize,
}

// type WrappedAlg = gaudi_rust_bindings::

fn my_rust_counting_alg_factory() -> Box<WrappedAlg> {
    Box::new(Box::new(
        gaudi::AlgorithmBuilder::new()
            .add_initialize_action(|data, host| {
                println!("Initialize {}", host.instance_name());
                data.counter.store(0, Relaxed);
            })
            .set_execute_action(|data, _host, _ctx| {
                data.counter.fetch_add(1, Relaxed);
            })
            .add_finalize_action(|data, host| {
                println!(
                    "Finalize {}: count = {}",
                    host.instance_name(),
                    data.counter.load(Relaxed)
                );
            })
            .build(),
    ))
}

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        type Algorithm = gaudi_rust_bindings::ffi::Algorithm;
        fn name(&self) -> &CxxString;
    }

    #[namespace = "Gaudi::Rust::details"]
    extern "Rust" {
        type WrappedAlg<'a>;
        fn my_rust_counting_alg_factory() -> Box<WrappedAlg>;
    }
}
