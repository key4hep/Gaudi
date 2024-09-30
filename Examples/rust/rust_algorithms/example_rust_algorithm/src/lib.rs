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

mod gaudi {
    pub trait Stateful {
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
    pub trait Algorithm
    where
        Self: Stateful,
    {
        fn execute(&self, ctx: &crate::ffi::EventContext) -> Result<(), String>;
    }
}

type DynAlg<'a> = Box<dyn gaudi::Algorithm + 'a>;

fn dynalg_initialize(alg: &mut Box<DynAlg>) -> Result<(), String> {
    alg.initialize()
}
fn dynalg_start(alg: &mut Box<DynAlg>) -> Result<(), String> {
    alg.start()
}
fn dynalg_stop(alg: &mut Box<DynAlg>) -> Result<(), String> {
    alg.stop()
}
fn dynalg_finalize(alg: &mut Box<DynAlg>) -> Result<(), String> {
    alg.finalize()
}
fn dynalg_execute(alg: &Box<DynAlg>, ctx: &crate::ffi::EventContext) -> Result<(), String> {
    alg.execute(ctx)
}

use std::ops::Deref;
use std::sync::atomic::{AtomicUsize, Ordering::Relaxed};

// This should be done with a derive macro
// ```rs
// #[derive(gaudi::Algorithm)]
// struct MyAlg {
//     counter: AtomicUsize,
// }
// ```
// possibly with parameters that state which methods to leave out and which not
struct MyAlg<'a> {
    wrapper: &'a ffi::Algorithm,
    counter: AtomicUsize,
    logger: cxx::UniquePtr<ffi::Logger>,
}
impl<'a> From<&'a ffi::Algorithm> for MyAlg<'a> {
    fn from(wrapper: &'a ffi::Algorithm) -> Self {
        Self {
            wrapper,
            counter: Default::default(),
            logger: cxx::UniquePtr::null(),
        }
    }
}

impl gaudi::Stateful for MyAlg<'_> {
    fn initialize(&mut self) -> Result<(), String> {
        self.logger = ffi::make_alg_logger(self.wrapper);
        self.logger
            .deref()
            .info(&format!("Initialize {} (Rust)", self.wrapper.name()));
        Ok(())
    }
}

impl gaudi::Algorithm for MyAlg<'_> {
    fn execute(&self, _ctx: &crate::ffi::EventContext) -> Result<(), String> {
        self.counter.fetch_add(1, Relaxed);
        self.logger
            .deref()
            .info(&format!("counted {} events", self.counter.load(Relaxed)));
        Ok(())
    }
}

fn my_rust_counting_alg_factory<'a>(wrapper: &'a ffi::Algorithm) -> Box<DynAlg<'a>> {
    Box::new(Box::new(MyAlg::from(wrapper)))
}

#[cxx::bridge]
mod ffi {
    extern "C++" {
        include!("GaudiKernel/EventContext.h");
        type EventContext;
    }

    unsafe extern "C++" {
        include!("Gaudi/Algorithm.h");
        #[namespace = "Gaudi"]
        type Algorithm;
        fn name(&self) -> &CxxString;
    }

    #[namespace = "Gaudi::Rust"]
    unsafe extern "C++" {
        include!("Gaudi/Rust/Logger.h");
        type Logger;

        fn make_alg_logger(wrapper: &Algorithm) -> UniquePtr<Logger>;
        fn info(&self, msg: &str);
    }

    #[namespace = "Gaudi::Rust"]
    extern "Rust" {
        type DynAlg<'a>;

        fn dynalg_initialize(alg: &mut Box<DynAlg>) -> Result<()>;
        fn dynalg_start(alg: &mut Box<DynAlg>) -> Result<()>;
        fn dynalg_stop(alg: &mut Box<DynAlg>) -> Result<()>;
        fn dynalg_finalize(alg: &mut Box<DynAlg>) -> Result<()>;

        fn dynalg_execute(alg: &Box<DynAlg>, ctx: &EventContext) -> Result<()>;

        fn my_rust_counting_alg_factory(wrapper: &Algorithm) -> Box<DynAlg>;
    }
}
