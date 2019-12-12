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
//	====================================================================
//  StoreExplorerAlg.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/KeyedContainer.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ObjectContainerBase.h"
#include "GaudiKernel/ObjectList.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/SmartIF.h"

#include <numeric>

/**@class StoreExplorerAlg
 *
 * Small algorith, which traverses the data store and
 * prints generic information about all leaves, which
 * can be loaded/accessed.
 *
 * @author:  M.Frank
 * @version: 1.0
 */
class StoreExplorerAlg : public Algorithm {

  Gaudi::Property<bool>   m_load{this, "Load", false, "load non existing items"};
  Gaudi::Property<long>   m_print{this, "PrintEvt", 1, "limit printout to first N events"};
  Gaudi::Property<long>   m_printMissing{this, "PrintMissing", 0, "indicate if missing entities should be printed"};
  Gaudi::Property<double> m_frequency{this, "PrintFreq", 0.0, "printout frequency"};
  Gaudi::Property<bool>   m_exploreRelations{this, "ExploreRelations", false, "if relations should be followed"};
  Gaudi::Property<std::string> m_dataSvcName{this, "DataSvc", "EventDataSvc", "name of the data provider service"};
  Gaudi::Property<bool>        m_testAccess{this, "TestAccess", false,
                                     "test access to objects (DataObject and ContainedObject)"};
  Gaudi::Property<bool> m_accessForeign{this, "AccessForeign", false, "indicate if foreign files should be opened"};

  /// Internal counter to trigger printouts
  long m_total = 0;
  /// Internal counter to adjust printout frequency
  long m_frqPrint = 0;
  /// Reference to data provider service
  SmartIF<IDataProviderSvc> m_dataSvc;
  /// Name of the root leaf (obtained at initialize)
  std::string m_rootName;

public:
  /// Inherited constructor
  using Algorithm::Algorithm;

  template <class T>
  std::string access( T* p ) {
    if ( !p ) return "Access FAILED.";
    std::string result = std::accumulate(
        std::begin( *p ), std::end( *p ), std::string{}, [&]( std::string s, typename T::const_reference i ) {
          return s + std::to_string( p->index( i ) ) + ":" + std::to_string( i->clID() ) + ",";
        } );
    return result.substr( 0, result.length() - 2 );
  }

  /// Print datastore leaf
  void printObj( IRegistry* pReg, std::vector<bool>& flg ) {
    auto& log = info();
    for ( size_t j = 1; j < flg.size(); j++ ) {
      if ( !flg[j - 1] && flg[j] )
        log << "| ";
      else if ( flg[j] )
        log << "  ";
      else
        log << "| ";
    }
    log << "+--> " << pReg->name();
    if ( pReg->address() ) {
      log << " [Address: CLID=" << std::showbase << std::hex << pReg->address()->clID()
          << " Type=" << (void*)pReg->address()->svcType() << "]";
    } else {
      log << " [No Address]";
    }
    DataObject* p = pReg->object();
    if ( p ) {
      try {
        std::string typ = System::typeinfoName( typeid( *p ) );
        if ( m_testAccess ) p->clID();
        log << "  " << typ.substr( 0, 32 );
      } catch ( ... ) { log << "Access test FAILED"; }
    } else {
      log << "  (Unloaded) ";
    }
    ObjectContainerBase* base = dynamic_cast<ObjectContainerBase*>( p );
    if ( base ) {
      try {
        int        numObj = base->numberOfObjects();
        const CLID id     = p->clID();
        log << " [" << numObj << "]";
        if ( m_testAccess ) {
          CLID idd = id >> 16;
          switch ( idd ) {
          case CLID_ObjectList >> 16: /* ObjectList    */
            access( (ObjectList<ContainedObject>*)base );
            break;
          case CLID_ObjectVector >> 16: /* ObjectVector  */
            access( (ObjectVector<ContainedObject>*)base );
            break;
          case ( CLID_ObjectVector + 0x00030000 ) >> 16: /* Keyed Map     */
            access( (KeyedContainer<KeyedObject<int>, Containers::Map>*)base );
            break;
          case ( CLID_ObjectVector + 0x00040000 ) >> 16: /* Keyed Hashmap */
            access( (KeyedContainer<KeyedObject<int>, Containers::HashMap>*)base );
            break;
          case ( CLID_ObjectVector + 0x00050000 ) >> 16: /* Keyed array   */
            access( (KeyedContainer<KeyedObject<int>, Containers::Array>*)base );
            break;
          }
        }
      } catch ( ... ) { log << "Access test FAILED"; }
    }
    log << endmsg;
  }

  void explore( IRegistry* pObj, std::vector<bool>& flg ) {
    printObj( pObj, flg );
    if ( pObj ) {
      auto mgr = eventSvc().as<IDataManagerSvc>();
      if ( mgr ) {
        std::vector<IRegistry*> leaves;
        StatusCode              sc   = mgr->objectLeaves( pObj, leaves );
        const std::string*      par0 = nullptr;
        if ( pObj->address() ) par0 = pObj->address()->par();
        if ( sc.isSuccess() ) {
          for ( auto i = leaves.begin(); i != leaves.end(); i++ ) {
            const std::string& id = ( *i )->identifier();
            DataObject*        p  = nullptr;
            if ( !m_accessForeign && ( *i )->address() ) {
              if ( par0 ) {
                const std::string* par1 = ( *i )->address()->par();
                if ( par1 && par0[0] != par1[0] ) continue;
              }
            }
            if ( m_load ) {
              sc = eventSvc()->retrieveObject( id, p );
            } else {
              sc = eventSvc()->findObject( id, p );
            }
            if ( sc.isSuccess() ) {
              if ( id != "/Event/Rec/Relations" || m_exploreRelations ) {
                flg.push_back( i + 1 == leaves.end() );
                explore( *i, flg );
                flg.pop_back();
              }
            } else {
              flg.push_back( i + 1 == leaves.end() );
              printObj( *i, flg );
              flg.pop_back();
            }
          }
        }
      }
    }
  }

  /// Initialize
  StatusCode initialize() override {
    m_rootName.clear();
    m_dataSvc = service( m_dataSvcName, true );
    if ( !m_dataSvc ) {
      error() << "Failed to access service \"" << m_dataSvcName << "\"." << endmsg;
      return StatusCode::FAILURE;
    }
    auto mgr = m_dataSvc.as<IDataManagerSvc>();
    if ( !mgr ) {
      error() << "Failed to retrieve IDataManagerSvc interface." << endmsg;
      return StatusCode::FAILURE;
    }
    m_rootName = mgr->rootName();
    return StatusCode::SUCCESS;
  }

  /// Finalize
  StatusCode finalize() override {
    m_dataSvc.reset();
    return StatusCode::SUCCESS;
  }

  /// Execute procedure
  StatusCode execute() override {
    SmartDataPtr<DataObject> root( m_dataSvc.get(), m_rootName );
    if ( ( ( m_print > m_total++ ) || ( m_frequency * m_total > m_frqPrint ) ) && root ) {
      if ( m_frequency * m_total > m_frqPrint ) m_frqPrint++;
      std::string store_name = "Unknown";
      IRegistry*  pReg       = root->registry();
      if ( pReg ) {
        auto isvc = SmartIF<IService>{pReg->dataSvc()};
        if ( isvc ) store_name = isvc->name();
      }
      info() << "========= " << m_rootName << "[" << std::showbase << std::hex << (unsigned long)root.ptr() << std::dec
             << "@" << store_name << "]:" << endmsg;
      std::vector<bool> flg( 1, true );
      explore( root->registry(), flg );
      return StatusCode::SUCCESS;
    } else if ( root ) {
      return StatusCode::SUCCESS;
    }
    error() << "Cannot retrieve \"/Event\"!" << endmsg;
    return StatusCode::FAILURE;
  }
};

DECLARE_COMPONENT( StoreExplorerAlg )
