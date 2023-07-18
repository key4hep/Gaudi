/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Functional/Producer.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IDataStoreLeaves.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/SmartIF.h>
#include <fmt/format.h>

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
        return Producer::finalize();
      }

      // Scan the data service starting from the node specified as \b Root.
      IDataStoreLeaves::LeavesList i_collectLeaves() const;

    private:
      Gaudi::Property<std::string> m_dataSvcName{ this, "DataService", "EventDataSvc",
                                                  "Name of the data service to use" };

      Gaudi::Property<std::string> m_rootNode{ this, "Root", "", "Path to the element from which to start the scan" };

      // Pointer to the IDataManagerSvc interface of the data service.
      SmartIF<IDataManagerSvc> m_dataMgrSvc;
    };

    // implementation

    StatusCode FetchLeavesFromFile::initialize() {
      return Producer::initialize().andThen( [&]() {
        m_dataMgrSvc = serviceLocator()->service( m_dataSvcName );
        if ( !m_dataMgrSvc ) {
          error() << "Cannot get IDataManagerSvc " << m_dataSvcName << endmsg;
          return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
      } );
    }

    IDataStoreLeaves::LeavesList FetchLeavesFromFile::i_collectLeaves() const {
      // we have to make sure that the node we start the traversal from is loaded
      {
        DataObject* obj = nullptr;
        evtSvc()
            ->retrieveObject( m_rootNode, obj )
            .orThrow( fmt::format( "failed to retrieve {} from {}", m_rootNode.value(), m_dataSvcName.value() ),
                      name() );
      }
      // result
      IDataStoreLeaves::LeavesList all_leaves;
      // used to test for nodes with same origin
      std::string origin;
      // we do not get info from exceptions in the data store agent, so we record the exception message, if any
      std::string failure_msg;
      m_dataMgrSvc
          ->traverseSubTree( m_rootNode,
                             [&]( IRegistry* reg, int ) {
                               if ( reg->address() && reg->dataSvc() ) { // we consider only objects that come from a
                                                                         // file
                                 if ( origin.empty() ) {
                                   // this is the first node we encounter, so we record where it comes from
                                   origin = reg->address()->par()[0];
                                 }
                                 // if the current object comes from the same file as the first entry...
                                 if ( origin == reg->address()->par()[0] ) {
                                   // ... make sure the object has been loaded...
                                   DataObject* obj = reg->object();
                                   if ( !obj )
                                     reg->dataSvc()
                                         ->retrieveObject( reg->identifier(), obj )
                                         .orElse( [&]() {
                                           failure_msg = fmt::format( "failed to retrieve {} from {}",
                                                                      reg->identifier(), m_dataSvcName.value() );
                                           // we do not really care about the exception we throw because traverseSubTree
                                           // will just use it to abort the traversal
                                           throw GaudiException( failure_msg, name(), StatusCode::FAILURE );
                                         } )
                                         .ignore();
                                   // ... and add it to the list
                                   all_leaves.push_back( obj );
                                   if ( msgLevel( MSG::VERBOSE ) )
                                     verbose() << "::i_collectLeaves added " << reg->identifier() << endmsg;
                                   return true; // we can continue the recursion
                                 }
                               }
                               // if we reach this point the object was not interesting, so no need to recurse further
                               return false;
                             } )
          .orThrow( failure_msg, name() );
      return all_leaves;
    }

  } // namespace Hive
} // namespace Gaudi

DECLARE_COMPONENT( Gaudi::Hive::FetchLeavesFromFile )
