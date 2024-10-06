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
extern "C" fn my_rust_counting_alg_factory() -> *mut WrappedAlg {
    Box::into_raw(Box::new(Box::new(
        gaudi::AlgorithmBuilder::<CounterAlg>::new()
            .add_property("std::string", "Text", "\"lorem ipsum\"", None)
            .add_initialize_action(|data, host| {
                host.info(&format!("Initialize {} (Rust)", host.instance_name()));
                host.debug(&format!("property Text -> {:?}", host.get_property("Text")));
                host.debug(&format!(
                    "property Unknown -> {:?}",
                    host.get_property("Unknown")
                ));
                data.counter.store(0, Relaxed);
                Ok(())
            })
            .set_execute_action(|data, host, _ctx| {
                data.counter.fetch_add(1, Relaxed);
                host.info(&format!("counted {} events", data.counter.load(Relaxed)));
                Ok(())
            })
            .add_finalize_action(|data, host| {
                host.info(&format!(
                    "Finalize {}: count = {}",
                    host.instance_name(),
                    data.counter.load(Relaxed)
                ));
                Ok(())
            })
            .build(),
    )))
}

#[derive(Default)]
struct IntProducer {
    value: i32,
    output_location: String,
}

#[no_mangle]
extern "C" fn int_producer_factory() -> *mut WrappedAlg {
    Box::into_raw(Box::new(Box::new(
        gaudi::AlgorithmBuilder::<IntProducer>::new()
            .add_property("int", "Value", "7", Some("The integer value to produce."))
            .add_output("int", "OutputLocation", "\"/Event/Int\"", None)
            .add_initialize_action(|data, host| {
                data.value = host.get_property("Value").unwrap().parse().map_err(|e| {
                    format!("{}.Value is not an integer: {}", host.instance_name(), e)
                })?;
                data.output_location = host
                    .get_property("OutputLocation")
                    .unwrap()
                    .trim_matches('"')
                    .to_string();
                Ok(())
            })
            .set_execute_action(|data, host, _ctx| {
                host.info(&format!(
                    "executing {}, storing {} into {}",
                    host.instance_name(),
                    data.value,
                    data.output_location
                ));
                Ok(())
            })
            .build(),
    )))
}
