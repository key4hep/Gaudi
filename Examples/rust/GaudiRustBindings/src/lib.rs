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
pub mod gaudi {
    // bridge to the C++ object wrapping the Rust algorithm
    #[derive(Clone, Copy)]
    pub struct Host<'a> {
        pub alg: &'a super::ffi::Algorithm,
    }
    impl Host<'_> {
        pub fn instance_name(&self) -> &str {
            self.alg.name().to_str().expect("Invalid UTF-8 string")
        }
    }

    pub trait AlgorithmTrait {
        fn initialize(&mut self, host: Host) -> Result<(), String>;
        fn start(&mut self, host: Host) -> Result<(), String>;
        fn stop(&mut self, host: Host) -> Result<(), String>;
        fn finalize(&mut self, host: Host) -> Result<(), String>;
        fn execute(&self, host: Host, ctx: &EventContext) -> Result<(), String>;
    }

    pub type Action<State> = Box<dyn FnMut(&mut State, Host) -> Result<(), String>>;
    pub type ExecuteAction<State> = Box<dyn Fn(&State, Host, &EventContext) -> Result<(), String>>;
    pub type Actions<State> = Vec<Action<State>>;

    pub struct Algorithm<State> {
        initialize_actions: Actions<State>,
        start_actions: Actions<State>,
        stop_actions: Actions<State>,
        finalize_actions: Actions<State>,
        state: State,
        execute_action: ExecuteAction<State>,
    }

    impl<State: Default> Default for Algorithm<State> {
        fn default() -> Self {
            Self {
                initialize_actions: vec![],
                start_actions: vec![],
                stop_actions: vec![],
                finalize_actions: vec![],
                state: Default::default(),
                execute_action: Box::new(|_, _, _| {
                    unimplemented!();
                }),
            }
        }
    }

    impl<State> AlgorithmTrait for Algorithm<State> {
        fn initialize(&mut self, host: Host) -> Result<(), String> {
            for action in self.initialize_actions.iter_mut() {
                action(&mut self.state, host)?;
            }
            Ok(())
        }

        fn start(&mut self, host: Host) -> Result<(), String> {
            for action in self.start_actions.iter_mut() {
                action(&mut self.state, host)?;
            }
            Ok(())
        }

        fn stop(&mut self, host: Host) -> Result<(), String> {
            for action in self.stop_actions.iter_mut() {
                action(&mut self.state, host)?;
            }
            Ok(())
        }

        fn finalize(&mut self, host: Host) -> Result<(), String> {
            for action in self.finalize_actions.iter_mut() {
                action(&mut self.state, host)?;
            }
            Ok(())
        }

        fn execute(&self, host: Host, ctx: &EventContext) -> Result<(), String> {
            (self.execute_action)(&self.state, host, ctx)
        }
    }

    pub struct AlgorithmBuilder<State> {
        algorithm: Algorithm<State>,
    }

    impl<State: Default> AlgorithmBuilder<State> {
        pub fn new() -> AlgorithmBuilder<State> {
            AlgorithmBuilder {
                algorithm: Algorithm::default(),
            }
        }
    }

    impl<State> AlgorithmBuilder<State> {
        pub fn add_initialize_action(
            mut self,
            action: impl FnMut(&mut State, Host) -> Result<(), String> + 'static,
        ) -> Self {
            self.algorithm.initialize_actions.push(Box::new(action));
            self
        }
        pub fn add_start_action(
            mut self,
            action: impl FnMut(&mut State, Host) -> Result<(), String> + 'static,
        ) -> Self {
            self.algorithm.start_actions.push(Box::new(action));
            self
        }
        pub fn add_stop_action(
            mut self,
            action: impl FnMut(&mut State, Host) -> Result<(), String> + 'static,
        ) -> Self {
            self.algorithm.stop_actions.push(Box::new(action));
            self
        }
        pub fn add_finalize_action(
            mut self,
            action: impl FnMut(&mut State, Host) -> Result<(), String> + 'static,
        ) -> Self {
            self.algorithm.finalize_actions.push(Box::new(action));
            self
        }
        pub fn set_execute_action(
            mut self,
            action: impl Fn(&State, Host, &EventContext) -> Result<(), String> + 'static,
        ) -> Self {
            self.algorithm.execute_action = Box::new(action);
            self
        }
        pub fn build(self) -> Algorithm<State> {
            self.algorithm
        }
    }

    pub struct EventContext<'a> {
        inner: &'a crate::ffi::EventContext,
    }
    impl<'a> From<&'a crate::ffi::EventContext> for EventContext<'a> {
        fn from(ctx: &'a crate::ffi::EventContext) -> Self {
            Self { inner: ctx }
        }
    }
}

pub type WrappedAlg = Box<dyn gaudi::AlgorithmTrait>;

fn alg_initialize(alg: &mut Box<WrappedAlg>, host: &ffi::Algorithm) -> Result<(), String> {
    alg.initialize(gaudi::Host { alg: host })
}
// fn alg_start(alg: &mut Box<WrappedAlg>) -> Result<(), String> {
//     alg.start()
// }
// fn alg_stop(alg: &mut Box<WrappedAlg>) -> Result<(), String> {
//     alg.stop()
// }
// fn alg_finalize(alg: &mut Box<WrappedAlg>) -> Result<(), String> {
//     alg.finalize()
// }
// fn alg_execute(alg: &Box<WrappedAlg>, ctx: &crate::ffi::EventContext) -> Result<(), String> {
//     alg.execute(ctx)
// }

#[cxx::bridge]
pub mod ffi {
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

    //     #[namespace = "Gaudi::Rust"]
    //     unsafe extern "C++" {
    //         include!("Gaudi/Rust/Logger.h");
    //         type Logger;

    //         fn make_alg_logger(wrapper: &Algorithm) -> UniquePtr<Logger>;
    //         fn info(&self, msg: &str);
    //     }

    #[namespace = "Gaudi::Rust::details"]
    extern "Rust" {
        type WrappedAlg;

        fn alg_initialize(alg: &mut Box<WrappedAlg>, host: &Algorithm) -> Result<()>;
        // fn alg_start(alg: &mut Box<WrappedAlg>) -> Result<()>;
        //         fn alg_stop(alg: &mut Box<WrappedAlg>) -> Result<()>;
        //         fn alg_finalize(alg: &mut Box<WrappedAlg>) -> Result<()>;

        //         fn alg_execute(alg: &Box<WrappedAlg>, ctx: &EventContext) -> Result<()>;
    }
}
