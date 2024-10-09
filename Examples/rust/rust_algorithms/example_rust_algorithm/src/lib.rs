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
            .set_execute_action(|data, host, ctx| {
                host.info(&format!(
                    "executing {}, storing {} into {} (Rust)",
                    host.instance_name(),
                    data.value,
                    data.output_location
                ));
                host.put(ctx, "OutputLocation", data.value);
                Ok(())
            })
            .build(),
    )))
}

#[derive(Default)]
struct I2FAlg {
    input_location: String,
    output_location: String,
}
#[no_mangle]
extern "C" fn i2f_factory() -> *mut WrappedAlg {
    Box::into_raw(Box::new(Box::new(
        gaudi::AlgorithmBuilder::<I2FAlg>::new()
            .add_input("int", "InputLocation", "\"/Event/Int\"", None)
            .add_output("float", "OutputLocation", "\"/Event/Float\"", None)
            .add_initialize_action(|data, host| {
                data.input_location = host
                    .get_property("InputLocation")
                    .unwrap()
                    .trim_matches('"')
                    .to_string();
                data.output_location = host
                    .get_property("OutputLocation")
                    .unwrap()
                    .trim_matches('"')
                    .to_string();
                Ok(())
            })
            .set_execute_action(|data, host, ctx| {
                let value: i32 = *host.get(ctx, "InputLocation");
                host.info(&format!(
                    "Converting: {} from {} and storing it into {} (Rust)",
                    value, data.input_location, data.output_location,
                ));
                host.put(ctx, "OutputLocation", value as f32);
                Ok(())
            })
            .build(),
    )))
}

#[derive(Default)]
struct FloatConsumer;
#[no_mangle]
extern "C" fn float_consumer_factory() -> *mut WrappedAlg {
    Box::into_raw(Box::new(Box::new(
        gaudi::AlgorithmBuilder::<FloatConsumer>::new()
            .add_input("float", "InputLocation", "\"/Event/Float\"", None)
            .set_execute_action(|_data, host, ctx| {
                let value: f32 = *host.get(ctx, "InputLocation");
                host.info(&format!(
                    "executing {}: {} (Rust)",
                    host.instance_name(),
                    value
                ));
                Ok(())
            })
            .build(),
    )))
}

#[derive(Default, Debug, Clone)]
struct Point {
    x: f32,
    y: f32,
    z: f32,
}

impl From<Point> for gaudi::BoxedDataObject {
    fn from(point: Point) -> Self {
        Self(ffi::wrap_point(Box::new(point)))
    }
}
impl<'a> From<gaudi::DataObjectRef<'a>> for &'a Point {
    fn from(value: gaudi::DataObjectRef<'a>) -> Self {
        ffi::unwrap_point(value.0).as_ref()
    }
}

impl std::ops::Sub for &Point {
    type Output = Point;
    fn sub(self, rhs: Self) -> Point {
        Point {
            x: self.x - rhs.x,
            y: self.y - rhs.y,
            z: self.z - rhs.z,
        }
    }
}

#[derive(Default)]
struct PointProducer {
    point: Point,
    output_location: String,
}

#[no_mangle]
extern "C" fn point_producer_factory() -> *mut WrappedAlg {
    Box::into_raw(Box::new(Box::new(
        gaudi::AlgorithmBuilder::<PointProducer>::new()
            .add_property("float", "X", "1.0", None)
            .add_property("float", "Y", "2.0", None)
            .add_property("float", "Z", "3.0", None)
            .add_output(
                "rust::Box<Gaudi::Examples::Rust::Point>",
                "OutputLocation",
                "\"/Event/Point\"",
                None,
            )
            .add_initialize_action(|data, host| {
                let get_property = |name: &str| -> Result<f32, String> {
                    host.get_property(name).unwrap().parse().map_err(|e| {
                        format!("{}.{} is not a float: {}", host.instance_name(), name, e)
                    })
                };
                let x = get_property("X")?;
                let y = get_property("Y")?;
                let z = get_property("Z")?;
                data.point = Point { x, y, z };
                data.output_location = host
                    .get_property("OutputLocation")
                    .unwrap()
                    .trim_matches('"')
                    .to_string();
                Ok(())
            })
            .set_execute_action(|data, host, ctx| {
                host.info(&format!(
                    "executing {}, storing {:?} into {} (Rust)",
                    host.instance_name(),
                    data.point,
                    data.output_location
                ));
                host.put(ctx, "OutputLocation", data.point.clone());
                Ok(())
            })
            .build(),
    )))
}

#[derive(Default)]
struct PointDiff;

#[no_mangle]
extern "C" fn points_diff_factory() -> *mut WrappedAlg {
    Box::into_raw(Box::new(Box::new(
        gaudi::AlgorithmBuilder::<PointDiff>::new()
            .add_input(
                "rust::Box<Gaudi::Examples::Rust::Point>",
                "InputLocation1",
                "\"/Event/Point1\"",
                None,
            )
            .add_input(
                "rust::Box<Gaudi::Examples::Rust::Point>",
                "InputLocation2",
                "\"/Event/Point2\"",
                None,
            )
            .set_execute_action(|_data, host, ctx| {
                let point1: &Point = host.get(ctx, "InputLocation1");
                let point2: &Point = host.get(ctx, "InputLocation2");
                host.info(&format!(
                    "{:?} - {:?} = {:?}",
                    point1,
                    point2,
                    point1 - point2
                ));
                Ok(())
            })
            .build(),
    )))
}

#[derive(Default)]
struct TestVectorAlg {
    input_location: String,
}

#[no_mangle]
extern "C" fn test_vector_consumer_factory() -> *mut WrappedAlg {
    Box::into_raw(Box::new(Box::new(
        gaudi::AlgorithmBuilder::<TestVectorAlg>::new()
            .add_input(
                "Gaudi::Examples::TestVector",
                "InputLocation",
                "\"/Event/TestVector\"",
                None,
            )
            .add_initialize_action(|data, host| {
                data.input_location = host
                    .get_property("InputLocation")
                    .unwrap()
                    .trim_matches('"')
                    .to_string();
                Ok(())
            })
            .set_execute_action(|data, host, ctx| {
                let vector: &ffi::TestVector = host.get(ctx, "InputLocation");
                host.info(&format!(
                    "got vector from {}: {:?}",
                    data.input_location,
                    (vector.x(), vector.y(), vector.z())
                ));
                Ok(())
            })
            .build(),
    )))
}

impl<'a> From<gaudi::DataObjectRef<'a>> for &'a ffi::TestVector {
    fn from(value: gaudi::DataObjectRef<'a>) -> Self {
        ffi::cast_to_testvector(value.0)
    }
}

#[cxx::bridge]
mod ffi {
    extern "Rust" {
        #[namespace = "Gaudi::Examples::Rust"]
        type Point;
    }
    unsafe extern "C++" {
        include!("helpers.h");
        type DataObject = gaudi_rust_bindings::ffi::DataObject;
        // C++ function to create a DataObject from a Point (Rust opaque type)
        fn wrap_point(point: Box<Point>) -> UniquePtr<DataObject>;
        fn unwrap_point<'a>(value: &'a DataObject) -> &'a Box<Point>;
    }

    unsafe extern "C++" {
        include!("Gaudi/Examples/TestVector.h");
        #[namespace = "Gaudi::Examples"]
        type TestVector;
        fn x(&self) -> f64;
        fn y(&self) -> f64;
        fn z(&self) -> f64;
        // fn SetX(&mut self, value: f64);
        // fn SetY(&mut self, value: f64);
        // fn SetZ(&mut self, value: f64);

        fn cast_to_testvector<'a>(value: &'a DataObject) -> &'a TestVector;
    }
}
