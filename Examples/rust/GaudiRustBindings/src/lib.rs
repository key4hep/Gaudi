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
    use cxx::let_cxx_string;

    // bridge to the C++ object wrapping the Rust algorithm
    #[derive(Clone, Copy)]
    pub struct Host<'a> {
        pub alg: &'a super::ffi::AlgWrapper,
    }

    macro_rules! common_msg_forward {
        ($level:ident) => {
            pub fn $level(&self, msg: &str) {
                cxx::let_cxx_string!(msg = msg);
                self.alg.$level(&msg);
            }
        };
    }
    impl Host<'_> {
        pub fn instance_name(&self) -> &str {
            self.alg.name().to_str().expect("Invalid UTF-8 string")
        }
        common_msg_forward! { verbose }
        common_msg_forward! { debug }
        common_msg_forward! { info }
        common_msg_forward! { warning }
        common_msg_forward! { error }
        common_msg_forward! { fatal }
        pub fn trace(&self, msg: &str) {
            self.verbose(msg);
        }

        pub fn add_property(&self, semantics: &str, name: &str, default: &str, doc: Option<&str>) {
            let_cxx_string!(semantics = semantics);
            let_cxx_string!(name = name);
            let_cxx_string!(default = default);
            let_cxx_string!(doc = doc.unwrap_or(""));
            self.alg.addProperty(&semantics, &name, &default, &doc);
        }

        pub fn get_property(&self, name: &str) -> Option<String> {
            let_cxx_string!(name = name);
            crate::ffi::getPropertyValueHelper(self.alg, &name).ok()
        }

        pub fn add_input(&self, type_name: &str, name: &str, location: &str, doc: Option<&str>) {
            let_cxx_string!(name = name);
            let_cxx_string!(type_name = type_name);
            let_cxx_string!(location = location);
            let_cxx_string!(doc = doc.unwrap_or(""));
            self.alg.addInputHandle(&type_name, &name, &location, &doc);
        }
        pub fn add_output(&self, type_name: &str, name: &str, location: &str, doc: Option<&str>) {
            let_cxx_string!(name = name);
            let_cxx_string!(type_name = type_name);
            let_cxx_string!(location = location);
            let_cxx_string!(doc = doc.unwrap_or(""));
            self.alg.addOutputHandle(&type_name, &name, &location, &doc);
        }

        pub fn put<T: Into<BoxedDataObject>>(&self, ctx: &EventContext, key: &str, value: T) {
            let_cxx_string!(key = key);
            self.alg.put(ctx.inner, &key, value.into().0);
        }

        pub fn get<'a, T>(&self, ctx: &EventContext, key: &str) -> &'a T
        where
            &'a T: From<DataObjectRef<'a>>,
        {
            let_cxx_string!(key = key);
            let obj = DataObjectRef(self.alg.get(ctx.inner, &key));
            obj.into()
        }
    }

    /// Helper wrapping a C++ `std::unique_ptr<DataObject>` to implement conversion to DataObject.
    pub struct BoxedDataObject(pub cxx::UniquePtr<crate::ffi::DataObject>);
    /// Helper wrapping a C++ `DataObject const&` to implement conversion from DataObject.
    pub struct DataObjectRef<'a>(pub &'a crate::ffi::DataObject);

    macro_rules! dobj_conversion {
        ($type:ty, $dobj_from:ident, $from_dobj:ident) => {
            impl From<$type> for BoxedDataObject {
                fn from(value: $type) -> Self {
                    Self(crate::ffi::$dobj_from(value))
                }
            }
            impl<'a> From<DataObjectRef<'a>> for &'a $type {
                fn from(value: DataObjectRef<'a>) -> Self {
                    crate::ffi::$from_dobj(value.0)
                }
            }
        };
    }
    dobj_conversion! {i32, dobj_from_i32, i32_from_dobj}
    dobj_conversion! {f32, dobj_from_f32, f32_from_dobj}

    pub trait AlgorithmTrait {
        fn bind_host(&mut self, host: Host) -> Result<(), String>;
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
        bind_host_actions: Actions<State>,
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
                bind_host_actions: vec![],
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
        fn bind_host(&mut self, host: Host) -> Result<(), String> {
            for action in self.bind_host_actions.iter_mut() {
                action(&mut self.state, host)?;
            }
            Ok(())
        }
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

    #[derive(Default)]
    pub struct AlgorithmBuilder<State> {
        algorithm: Algorithm<State>,
    }

    impl<State: Default> AlgorithmBuilder<State> {
        pub fn new() -> AlgorithmBuilder<State> {
            Default::default()
        }
    }

    impl<State> AlgorithmBuilder<State> {
        pub fn add_bind_host_action(
            mut self,
            action: impl FnMut(&mut State, Host) -> Result<(), String> + 'static,
        ) -> Self {
            self.algorithm.bind_host_actions.push(Box::new(action));
            self
        }
        pub fn add_property(
            self,
            semantics: &str,
            name: &str,
            default: &str,
            doc: Option<&str>,
        ) -> Self {
            let semantics = semantics.to_string();
            let name = name.to_string();
            let default = default.to_string();
            let doc = doc.map(|s| s.to_string());
            self.add_bind_host_action(move |_state, host| {
                host.add_property(&semantics, &name, &default, doc.as_deref());
                Ok(())
            })
        }
        pub fn add_input(
            self,
            type_name: &str,
            name: &str,
            location: &str,
            doc: Option<&str>,
        ) -> Self {
            let type_name = type_name.to_string();
            let name = name.to_string();
            let location = location.to_string();
            let doc = doc.map(|s| s.to_string());
            self.add_bind_host_action(move |_state, host| {
                host.add_input(&type_name, &name, &location, doc.as_deref());
                Ok(())
            })
        }
        pub fn add_output(
            self,
            type_name: &str,
            name: &str,
            location: &str,
            doc: Option<&str>,
        ) -> Self {
            let type_name = type_name.to_string();
            let name = name.to_string();
            let location = location.to_string();
            let doc = doc.map(|s| s.to_string());
            self.add_bind_host_action(move |_state, host| {
                host.add_output(&type_name, &name, &location, doc.as_deref());
                Ok(())
            })
        }
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
    impl EventContext<'_> {
        pub fn evt(&self) -> usize {
            self.inner.evt()
        }
        pub fn slot(&self) -> usize {
            self.inner.slot()
        }
        pub fn sub_slot(&self) -> Option<usize> {
            if self.inner.usesSubSlot() {
                Some(self.inner.subSlot())
            } else {
                None
            }
        }
        pub fn is_valid(&self) -> bool {
            self.inner.valid()
        }
    }
}

pub type WrappedAlg = Box<dyn gaudi::AlgorithmTrait>;

unsafe fn alg_bind_host(alg: *mut WrappedAlg, host: &ffi::AlgWrapper) -> Result<(), String> {
    (*alg).bind_host(gaudi::Host { alg: host })
}

unsafe fn alg_initialize(alg: *mut WrappedAlg, host: &ffi::AlgWrapper) -> Result<(), String> {
    (*alg).initialize(gaudi::Host { alg: host })
}
unsafe fn alg_start(alg: *mut WrappedAlg, host: &ffi::AlgWrapper) -> Result<(), String> {
    (*alg).start(gaudi::Host { alg: host })
}
unsafe fn alg_stop(alg: *mut WrappedAlg, host: &ffi::AlgWrapper) -> Result<(), String> {
    (*alg).stop(gaudi::Host { alg: host })
}
unsafe fn alg_finalize(alg: *mut WrappedAlg, host: &ffi::AlgWrapper) -> Result<(), String> {
    (*alg).finalize(gaudi::Host { alg: host })
}
unsafe fn alg_execute(
    alg: *const WrappedAlg,
    host: &ffi::AlgWrapper,
    ctx: &crate::ffi::EventContext,
) -> Result<(), String> {
    (*alg).execute(gaudi::Host { alg: host }, &gaudi::EventContext::from(ctx))
}
unsafe fn alg_drop(alg: *mut WrappedAlg) {
    let _ = Box::from_raw(alg);
}

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("GaudiKernel/EventContext.h");
        type EventContext;
        fn evt(&self) -> usize;
        fn slot(&self) -> usize;
        fn subSlot(&self) -> usize;
        fn usesSubSlot(&self) -> bool;
        fn valid(&self) -> bool;
    }

    extern "C++" {
        include!("GaudiKernel/DataObject.h");
        pub type DataObject;
    }

    unsafe extern "C++" {
        include!("Gaudi/Rust/AlgWrapper.h");
        include!("rust_helpers.h");
        #[namespace = "Gaudi::Rust"]
        type AlgWrapper;
        fn name(&self) -> &CxxString;
        fn verbose(&self, msg: &CxxString);
        fn debug(&self, msg: &CxxString);
        fn info(&self, msg: &CxxString);
        fn warning(&self, msg: &CxxString);
        fn error(&self, msg: &CxxString);
        fn fatal(&self, msg: &CxxString);
        fn addProperty(
            &self,
            semantics: &CxxString,
            name: &CxxString,
            default_value: &CxxString,
            doc: &CxxString,
        );
        fn addInputHandle(
            &self,
            name: &CxxString,
            type_name: &CxxString,
            location: &CxxString,
            doc: &CxxString,
        );
        fn addOutputHandle(
            &self,
            name: &CxxString,
            type_name: &CxxString,
            location: &CxxString,
            doc: &CxxString,
        );

        fn put(&self, ctx: &EventContext, name: &CxxString, value: UniquePtr<DataObject>);
        fn get<'a>(&self, ctx: &EventContext, name: &CxxString) -> &'a DataObject;

        #[namespace = "Gaudi::Rust::helpers"]
        fn getPropertyValueHelper(alg: &AlgWrapper, name: &CxxString) -> Result<String>;
    }

    #[namespace = "Gaudi::Rust::helpers"]
    unsafe extern "C++" {
        fn dobj_from_i32(value: i32) -> UniquePtr<DataObject>;
        fn i32_from_dobj<'a>(value: &'a DataObject) -> &'a i32;

        fn dobj_from_f32(value: f32) -> UniquePtr<DataObject>;
        fn f32_from_dobj<'a>(value: &'a DataObject) -> &'a f32;
    }

    #[namespace = "Gaudi::Rust::details"]
    extern "Rust" {
        type WrappedAlg;

        unsafe fn alg_bind_host(alg: *mut WrappedAlg, host: &AlgWrapper) -> Result<()>;
        unsafe fn alg_initialize(alg: *mut WrappedAlg, host: &AlgWrapper) -> Result<()>;
        unsafe fn alg_start(alg: *mut WrappedAlg, host: &AlgWrapper) -> Result<()>;
        unsafe fn alg_stop(alg: *mut WrappedAlg, host: &AlgWrapper) -> Result<()>;
        unsafe fn alg_finalize(alg: *mut WrappedAlg, host: &AlgWrapper) -> Result<()>;
        unsafe fn alg_execute(
            alg: *const WrappedAlg,
            host: &AlgWrapper,
            ctx: &EventContext,
        ) -> Result<()>;
        unsafe fn alg_drop(alg: *mut WrappedAlg);
    }
}
