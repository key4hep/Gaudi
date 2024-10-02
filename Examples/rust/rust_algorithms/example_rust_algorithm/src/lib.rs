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
use gaudi_rust_bindings::*;
use std::sync::atomic::{AtomicUsize, Ordering::Relaxed};

#[derive(Default)]
struct CounterAlg {
    counter: AtomicUsize,
}

#[no_mangle]
extern "C" fn my_rust_counting_alg_factory() -> *mut Box<dyn gaudi::AlgorithmTrait> {
    Box::into_raw(Box::new(Box::new(
        gaudi::AlgorithmBuilder::<CounterAlg>::new()
            .add_initialize_action(|data, host| {
                println!("Initialize {}", host.instance_name());
                data.counter.store(0, Relaxed);
                Ok(())
            })
            .set_execute_action(|data, _host, _ctx| {
                data.counter.fetch_add(1, Relaxed);
                Ok(())
            })
            .add_finalize_action(|data, host| {
                println!(
                    "Finalize {}: count = {}",
                    host.instance_name(),
                    data.counter.load(Relaxed)
                );
                Ok(())
            })
            .build(),
    )))
}

#[cxx::bridge]
mod ffi {

}
