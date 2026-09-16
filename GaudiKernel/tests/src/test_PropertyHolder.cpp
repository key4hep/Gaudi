/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_PropertyHolder
#include <boost/test/unit_test.hpp>

#include <GaudiKernel/DataHandleHolderBase.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/PropertyHolder.h>

#include <set>

namespace {
  const std::string emptyName{};
  /// Helper to allow instantiation of PropertyHolder.
  struct AnonymousPropertyHolder : public PropertyHolder<implements<IProperty, INamedInterface>> {
    const std::string& name() const override { return emptyName; }
  };

  /// Minimal component exercising registration of vector data handles.
  struct DataHandleHolder : DataHandleHolderBase<PropertyHolder<implements<IProperty, INamedInterface>>> {
    const std::string& name() const override { return emptyName; }
    void               acceptDHVisitor( IDataHandleVisitor* ) const override {}

    Gaudi::DataHandleVector<DataObjectReadHandle, int> inputs{
        this, "Inputs", { "/Event/A", "/Event/B" }, "Input TES keys" };
    Gaudi::DataHandleVector<DataObjectWriteHandle, int> outputs{ this, "Outputs", { "/Event/C" } };
  };

  std::set<std::string> keys( const std::vector<Gaudi::DataHandle*>& handles ) {
    std::set<std::string> result;
    for ( const auto* handle : handles ) result.insert( handle->objKey() );
    return result;
  }
} // namespace

BOOST_AUTO_TEST_CASE( declareProperty ) {
  Gaudi::Property<std::string> p1{ "p1", "v1" };
  Gaudi::Property<std::string> p2{ "p2", "v2" };
  Gaudi::Property<std::string> p3{ "p3", "v3" };
  {
    AnonymousPropertyHolder mgr;
    mgr.declareProperty( "p1", p1 );
    mgr.declareProperty( "p2", p2 );

    BOOST_CHECK( mgr.hasProperty( "p1" ) );
    BOOST_CHECK( mgr.hasProperty( "p2" ) );
    BOOST_CHECK( !mgr.hasProperty( "p0" ) );

    // case insensitive check
    BOOST_CHECK( mgr.hasProperty( "P1" ) );

    // FIXME: to be enabled if we decide to throw an exception, otherwise
    //        we need to improve it to check that the warning is printed
    // auto redeclare_property = [&mgr, &p3] () {
    //  mgr.declareProperty("p1", p3);
    //};
    // BOOST_CHECK_THROW(redeclare_property(), GaudiException);
  }
}

BOOST_AUTO_TEST_CASE( backward_compatibility ) {
  {
    AnonymousPropertyHolder                   mgr;
    Gaudi::Property<std::vector<std::string>> vp{ &mgr, "name", {} };

    auto sc = Gaudi::Utils::setProperty( &mgr, "name", std::vector<std::string>{ { "All" } } );

    BOOST_CHECK( sc.isSuccess() );
    BOOST_CHECK( vp == std::vector<std::string>{ { "All" } } );
  }
  {
    AnonymousPropertyHolder mgr;
    Gaudi::Property<bool>   p{ &mgr, "flag", false };

    BOOST_CHECK( mgr.setProperty( "flag", "true" ) );
    BOOST_CHECK_EQUAL( p, true );
  }
  {
    AnonymousPropertyHolder mgr;
    Gaudi::Property<int>    p{ &mgr, "int_prop", false };

    auto orig_policy =
        Gaudi::Details::Property::setParsingErrorPolicy( Gaudi::Details::Property::ParsingErrorPolicy::Exception );
    BOOST_CHECK_EXCEPTION(
        mgr.setProperty( "int_prop", "abc" ).ignore(), GaudiException, []( const GaudiException& err ) -> bool {
          return err.message() ==
                 "Cannot convert 'abc' for property 'int_prop' in class '(anonymous "
                 "namespace)::AnonymousPropertyHolder': std::invalid_argument, cannot parse 'abc' to int";
        } );

    Gaudi::Details::Property::setParsingErrorPolicy( Gaudi::Details::Property::ParsingErrorPolicy::Ignore );
    BOOST_TEST( mgr.setProperty( "int_prop", "abc" ) == StatusCode::FAILURE );

    Gaudi::Details::Property::setParsingErrorPolicy( orig_policy );
  }
}

BOOST_AUTO_TEST_CASE( vector_data_handles_are_registered ) {
  DataHandleHolder holder;

  const std::set<std::string> expectedInputs{ "/Event/A", "/Event/B" };
  const std::set<std::string> expectedOutputs{ "/Event/C" };

  BOOST_CHECK_EQUAL( holder.getProperty( "Inputs" ).documentation(), "Input TES keys" );
  BOOST_CHECK_EQUAL( holder.getProperty( "Outputs" ).documentation(), "" );
  BOOST_CHECK( keys( holder.inputHandles() ) == expectedInputs );
  BOOST_CHECK( keys( holder.outputHandles() ) == expectedOutputs );
  BOOST_REQUIRE_EQUAL( holder.inputs.keys().size(), 2 );
  BOOST_CHECK_EQUAL( holder.inputs.keys()[0].key(), "/Event/A" );
  BOOST_CHECK_EQUAL( holder.inputs.keys()[1].key(), "/Event/B" );

  BOOST_REQUIRE( holder.setProperty( "Inputs", "['/Event/D']" ).isSuccess() );
  BOOST_CHECK( keys( holder.inputHandles() ) == std::set<std::string>( { "/Event/D" } ) );
  BOOST_REQUIRE_EQUAL( holder.inputs.keys().size(), 1 );
  BOOST_CHECK_EQUAL( holder.inputs.keys()[0].key(), "/Event/D" );

  holder.inputs.clear();
  BOOST_CHECK( holder.inputs.keys().empty() );
  BOOST_CHECK( holder.inputs.handles().empty() );
  BOOST_CHECK( holder.inputHandles().empty() );
}
