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

struct DummyKvs;

impl DummyKvs {
    fn get_wrapper(&self, key: &str) -> ffi::KvsReturnValue {
        ffi::KvsReturnValue {
            found: true,
            value: format!("{0}-{0}", key),
        }
    }
    fn initialize(&mut self) -> Result<(), String> {
        Ok(())
    }
    fn start(&mut self) -> Result<(), String> {
        Ok(())
    }
    fn stop(&mut self) -> Result<(), String> {
        Ok(())
    }
    fn finalize(&mut self) -> Result<(), String> {
        Ok(())
    }
}

#[cxx::bridge]
mod ffi {
    struct KvsReturnValue {
        found: bool,
        value: String,
    }

    unsafe extern "C++" {
        include!("GaudiKernel/Service.h");
        type Service;
        fn name(&self) -> &CxxString;
    }

    extern "Rust" {
        type DummyKvs;
        fn make_kvs(wrapper: &Service) -> Box<DummyKvs>;
        fn get_wrapper(&self, key: &str) -> KvsReturnValue;

        fn initialize(&mut self) -> Result<()>;
        fn start(&mut self) -> Result<()>;
        fn stop(&mut self) -> Result<()>;
        fn finalize(&mut self) -> Result<()>;
    }
}
// declare_component!(RustKeyValueStore);

fn make_kvs(_wrapper: &ffi::Service) -> Box<DummyKvs> {
    // println!("{}", wrapper.name());
    Box::new(DummyKvs)
}
