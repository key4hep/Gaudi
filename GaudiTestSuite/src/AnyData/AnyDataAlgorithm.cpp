/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/AnyDataWrapper.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <string>
#include <vector>
//-----------------------------------------------------------------------------
// Implementation file for class : AnyData{Get,Put}Algorithm
//
// 2016-05-26 : Roel Aaij
//-----------------------------------------------------------------------------
/** @class AnyDataPutAlgorithm AnyDataPutAlgorithm.h AnyData/AnyDataPutAlgorithm.h
 *
 *
 *  @author Roel Aaij
 *  @date   2016-05-26
 */
class AnyDataPutAlgorithm : public Algorithm {
  Gaudi::Property<std::string>            m_loc{ this, "Location", "Test" };
  DataObjectWriteHandle<std::vector<int>> m_ids{ this, "Output", "/Event/Test/Ids" };
  std::vector<DataObjectWriteHandle<int>> m_id_vec;

public:
  AnyDataPutAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator ) {
    for ( int i = 0; i < 100; i++ ) { m_id_vec.emplace_back( "/Event/Test/Ids" + std::to_string( i ), this ); }
  }

  StatusCode execute() override {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

    auto i = std::make_unique<AnyDataWrapper<int>>( 0 );
    auto j = std::make_unique<AnyDataWrapper<std::vector<int>>>( std::vector<int>{ 0, 1, 2, 3 } );

    eventSvc()
        ->registerObject( m_loc.value() + "/One", i.release() )
        .orThrow( "failed to register " + m_loc.value() + "/One" );
    eventSvc()
        ->registerObject( m_loc.value() + "/Two", j.release() )
        .orThrow( "failed to register " + m_loc.value() + "/Two" );

    m_ids.put( std::vector<int>( { 42, 84 } ) );

    for ( int i = 0; i < 100; ++i ) m_id_vec[i].put( std::move( i ) );

    return StatusCode::SUCCESS;
  }
};

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( AnyDataPutAlgorithm )
/** @class AnyDataGetAlgorithm AnyDataGetAlgorithm.h AnyData/AnyDataGetAlgorithm.h
 *
 *
 *  @author Roel Aaij
 *  @date   2016-05-26
 */
template <class T>
class AnyDataGetAlgorithm : public Algorithm {
  Gaudi::Property<std::string> m_location{ this, "Location" };

  DataObjectReadHandle<std::vector<int>> m_ids{ this, "Input", "/Event/Test/Ids" };

public:
  using Algorithm::Algorithm;

  StatusCode execute() override {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

    DataObject* tmp = nullptr;
    eventSvc()->retrieveObject( m_location, tmp ).ignore();
    auto base = dynamic_cast<AnyDataWrapperBase*>( tmp );
    if ( base ) {
      info() << "Got base from " << m_location.value() << endmsg;
    } else {
      error() << "failed to get base from " << m_location.value() << endmsg;
      return StatusCode::FAILURE;
    }
    const auto i = dynamic_cast<const AnyDataWrapper<T>*>( base );
    if ( i ) {
      info() << "Got " << System::typeinfoName( typeid( T ) ) << " from " << m_location.value() << ": " << i->getData()
             << endmsg;
    }

    const auto& ids = m_ids.get();
    info() << "AnyDataHandler holds:";
    for ( const auto& i : *ids ) info() << i << " ";
    info() << endmsg;
    return StatusCode::SUCCESS;
  }
};

// Declaration of the Algorithm Factory
DECLARE_COMPONENT_WITH_ID( AnyDataGetAlgorithm<int>, "AnyDataGetAlgorithm_Int" )
DECLARE_COMPONENT_WITH_ID( AnyDataGetAlgorithm<std::vector<int>>, "AnyDataGetAlgorithm_VectorInt" )
