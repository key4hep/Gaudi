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

/// Dummy KeyValueStore implementation
struct DummyKvs;

/// Implement the methods required for IStateful
impl DummyKvs {
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

/// Implement the methods required for Gaudi::Examples::IKeyValueStore
impl DummyKvs {
    // Since cxx::bridge does not support std::optional<T> or Option<T>,
    // we use a helper shared struct (OptString)
    fn get(&self, key: &str) -> ffi::OptString {
        self.get_internal(key).into()
    }
    // This is the actual implementation we would like to have
    fn get_internal(&self, key: &str) -> Option<String> {
        Some(format!("{0}-{0}", key))
    }
}

impl From<&ffi::Service> for DummyKvs {
    fn from(_service: &ffi::Service) -> Self {
        Self
    }
}

#[cxx::bridge]
mod ffi {
    // Helper to convert a Option<String> to an std::optional<std::string>
    // (implemented in the C++ bridge)
    struct OptString {
        value: String,
        is_set: bool,
    }

    // from C++ we get the opaque Service type and the name method
    unsafe extern "C++" {
        include!("GaudiKernel/Service.h");
        type Service;
        fn name(&self) -> &CxxString;
    }

    extern "Rust" {
        // factory function to instantiate a DummyKvs
        fn make_dummy_kvs(service: &Service) -> Box<DummyKvs>;

        // opaque type for the KeyValueStore
        type DummyKvs;
        // IKeyValueStore methods
        fn get(&self, key: &str) -> OptString;
        // IStateful methods
        fn initialize(&mut self) -> Result<()>;
        fn start(&mut self) -> Result<()>;
        fn stop(&mut self) -> Result<()>;
        fn finalize(&mut self) -> Result<()>;
    }
}

impl From<Option<String>> for ffi::OptString {
    fn from(value: Option<String>) -> Self {
        let is_set = value.is_some();
        ffi::OptString {
            value: value.unwrap_or_default(),
            is_set,
        }
    }
}

fn make_dummy_kvs(service: &ffi::Service) -> Box<DummyKvs> {
    Box::new(service.into())
}
