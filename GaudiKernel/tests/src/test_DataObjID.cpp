/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_DataObjID
#include <boost/test/unit_test.hpp>

#include <Gaudi/Application.h>
#include <GaudiKernel/ClassID.h>
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/IClassIDSvc.h>
#include <GaudiKernel/Service.h>

#include <map>

/// IClassIDSvc mockup
class MyClassIDSvc : public extends<Service, IClassIDSvc> {
public:
  using base_class::base_class;

  virtual CLID nextAvailableID() const override { return m_types.size() + 1; }

  virtual bool isIDInUse( const CLID& id ) const override { return m_types.contains( id ); }

  virtual bool isNameInUse( const std::string& name ) const override {
    CLID id;
    return getIDOfTypeName( name, id ).isSuccess();
  }

  virtual StatusCode getIDOfTypeName( const std::string& typeName, CLID& id ) const override {
    for ( const auto& [i, n] : m_types ) {
      if ( n == typeName ) {
        id = i;
        return StatusCode::SUCCESS;
      }
    }
    return StatusCode::FAILURE;
  }

  virtual StatusCode getTypeNameOfID( const CLID& id, std::string& typeName ) const override {
    const auto t = m_types.find( id );
    if ( t != m_types.end() ) {
      typeName = t->second;
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }

  // Not implemented
  virtual StatusCode getTypeInfoNameOfID( const CLID&, std::string& ) const override { return StatusCode::FAILURE; }
  virtual StatusCode getIDOfTypeInfoName( const std::string&, CLID& ) const override { return StatusCode::FAILURE; }
  virtual StatusCode setTypeForID( const CLID&, const std::string&, const std::string& ) override {
    return StatusCode::FAILURE;
  }

private:
  inline static const std::map<CLID, std::string> m_types = { { 1, "TFoo" } };
};
DECLARE_COMPONENT( MyClassIDSvc )

// Test with bare key and without ClassIDSvc (e.g. LHCb use-case)
BOOST_AUTO_TEST_CASE( noClassIDSvc ) {

  auto check = []( const DataObjID& dobj ) {
    BOOST_CHECK_EQUAL( dobj.key(), "X" );
    BOOST_CHECK_EQUAL( dobj.fullKey(), "X" );
    BOOST_CHECK_EQUAL( dobj.clid(), 0 );
    BOOST_CHECK( dobj.className().empty() );
  };

  const DataObjID dobj( "X" );
  check( dobj );

  // Copy constructor
  const DataObjID dobj2( dobj );
  BOOST_CHECK( dobj == dobj2 );
  check( dobj2 );

  // Assignment
  DataObjID dobj3 = dobj;
  BOOST_CHECK( dobj == dobj3 );
  check( dobj3 );

  // Change key
  dobj3.updateKey( "Y" );
  BOOST_CHECK_EQUAL( dobj3.key(), "Y" );
  BOOST_CHECK( dobj != dobj3 );
}

// Test construction with ClassIDSvc (e.g. ATLAS use-case)
BOOST_AUTO_TEST_CASE( withClassIDSvc ) {

  auto app = Gaudi::Application( { { "ApplicationMgr.JobOptionsType", "\"NONE\"" },
                                   { "ApplicationMgr.CreateSvc", "[\"MyClassIDSvc/ClassIDSvc\"]" } } );
  app.run( []( SmartIF<IStateful>& /*app*/ ) {
    // Lookup CLID via ClassIDSvc
    {
      const DataObjID dobj( "TFoo", "Key" );
      BOOST_CHECK_EQUAL( dobj.key(), "Key" );
      BOOST_CHECK_EQUAL( dobj.fullKey(), "TFoo/Key" );
      BOOST_CHECK_EQUAL( dobj.clid(), 1 );
      BOOST_CHECK_EQUAL( dobj.className(), "TFoo" );
    }
    // Lookup class name via ClassIDSvc
    {
      const DataObjID dobj( CLID( 1 ), "Key" );
      BOOST_CHECK_EQUAL( dobj.key(), "Key" );
      BOOST_CHECK_EQUAL( dobj.fullKey(), "TFoo/Key" );
      BOOST_CHECK_EQUAL( dobj.clid(), 1 );
      BOOST_CHECK_EQUAL( dobj.className(), "TFoo" );
    }

    return 0;
  } );
}
