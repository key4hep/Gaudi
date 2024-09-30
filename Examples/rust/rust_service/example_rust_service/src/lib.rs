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
use std::ops::Deref;

/// Dummy KeyValueStore implementation
struct DummyKvs<'a> {
    service: &'a ffi::Service,
    logger: cxx::UniquePtr<ffi::Logger>,
}

/// Implement the methods required for IStateful
impl DummyKvs<'_> {
    fn info(&self, msg: &str) {
        self.logger.deref().info(msg);
    }

    fn initialize(&mut self) -> Result<(), String> {
        self.logger = ffi::make_svc_logger(self.service);
        self.info(&format!("Initialize {} (Rust)", self.service.name()));
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
impl DummyKvs<'_> {
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

impl<'a> From<&'a ffi::Service> for DummyKvs<'a> {
    fn from(service: &'a ffi::Service) -> Self {
        Self {
            service,
            logger: cxx::UniquePtr::null(),
        }
    }
}

fn make_dummy_kvs(service: &ffi::Service) -> Box<DummyKvs> {
    Box::new(service.into())
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

    #[namespace = "Gaudi::Rust"]
    unsafe extern "C++" {
        include!("Gaudi/Rust/Logger.h");
        type Logger;

        fn make_svc_logger(service: &Service) -> UniquePtr<Logger>;
        fn info(&self, msg: &str);
    }

    extern "Rust" {
        // factory function to instantiate a DummyKvs
        fn make_dummy_kvs(service: &Service) -> Box<DummyKvs>;

        // opaque type for the KeyValueStore
        type DummyKvs<'a>;
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
