/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/Producer.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IDataStoreLeaves.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/SmartIF.h"

namespace Gaudi {
  namespace Hive {
    class FetchLeavesFromFile final : public Gaudi::Functional::Producer<IDataStoreLeaves::LeavesList()> {
    public:
      FetchLeavesFromFile( const std::string& name, ISvcLocator* pSvcLocator )
          : Producer( name, pSvcLocator, KeyValue( "InputFileLeavesLocation", "/Event/InputFileLeaves" ) ) {}

      StatusCode initialize() override;

      IDataStoreLeaves::LeavesList operator()() const override { return i_collectLeaves(); }

      StatusCode finalize() override {
        m_dataMgrSvc.reset();
        return Algorithm::finalize();
      }

      // Scan the data service starting from the node specified as \b Root.
      IDataStoreLeaves::LeavesList i_collectLeaves() const;

      // Scan the data service starting from the specified node.
      template <typename OutputIterator>
      void i_collectLeaves( const IRegistry& reg, OutputIterator iter ) const;

      // Return the pointer to the IRegistry object associated to the node
      // specified as \b Root.
      const IRegistry& i_getRootNode() const;

    private:
      Gaudi::Property<std::string> m_dataSvcName{this, "DataService", "EventDataSvc",
                                                 "Name of the data service to use"};

      Gaudi::Property<std::string> m_rootNode{this, "Root", "", "Path to the element from which to start the scan"};

      // Pointer to the IDataManagerSvc interface of the data service.
      SmartIF<IDataManagerSvc> m_dataMgrSvc;
    };

    // implementation

    StatusCode FetchLeavesFromFile::initialize() {
      StatusCode sc = Algorithm::initialize();
      if ( sc ) {
        m_dataMgrSvc = serviceLocator()->service( m_dataSvcName );
        if ( !m_dataMgrSvc ) {
          error() << "Cannot get IDataManagerSvc " << m_dataSvcName << endmsg;
          return StatusCode::FAILURE;
        }
      }
      return sc;
    }

    const IRegistry& FetchLeavesFromFile::i_getRootNode() const {
      DataObject* obj = nullptr;
      StatusCode  sc  = Gaudi::Algorithm::evtSvc()->retrieveObject( m_rootNode.value(), obj );
      if ( sc.isFailure() ) {
        throw GaudiException( "Cannot get " + m_rootNode + " from " + m_dataSvcName, name(), StatusCode::FAILURE );
      }
      return *obj->registry();
    }

    IDataStoreLeaves::LeavesList FetchLeavesFromFile::i_collectLeaves() const {
      IDataStoreLeaves::LeavesList all_leaves;
      i_collectLeaves( i_getRootNode(), std::back_inserter( all_leaves ) );
      return all_leaves;
    }

    template <typename OutputIterator>
    void FetchLeavesFromFile::i_collectLeaves( const IRegistry& reg, OutputIterator iter ) const {
      // create a LeavesList to save all the leaves
      IOpaqueAddress* addr = reg.address();
      if ( addr ) { // we consider only objects that are in a file
        if ( msgLevel( MSG::VERBOSE ) ) verbose() << "::i_collectLeaves added " << reg.identifier() << endmsg;
        *iter = reg.object(); // add this object
        // Origin of the current object
        const std::string& base = addr->par()[0];

        std::vector<IRegistry*> lfs; // leaves of the current object
        StatusCode              sc = m_dataMgrSvc->objectLeaves( &reg, lfs );
        if ( sc.isSuccess() ) {
          for ( const auto& i : lfs ) {
            // Continue if the leaf has the same database as the parent
            if ( i->address() && i->address()->par()[0] == base ) {
              DataObject* obj = nullptr;
              // append leaves to all leaves
              Gaudi::Algorithm::evtSvc()
                  ->retrieveObject( const_cast<IRegistry*>( &reg ), i->name(), obj )
                  .andThen( [&] { i_collectLeaves( *i, iter ); } )
                  .orElse( [&] {
                    throw GaudiException( "Cannot get " + i->identifier() + " from " + m_dataSvcName, name(),
                                          StatusCode::FAILURE );
                  } )
                  .ignore();
            }
          }
        }
      }
    }

  } // namespace Hive
} // namespace Gaudi

DECLARE_COMPONENT( Gaudi::Hive::FetchLeavesFromFile )
