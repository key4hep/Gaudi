//  ====================================================================
//  StoreSnifferAlg.cpp
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/SmartIF.h"

using namespace std;

/**@class StoreSnifferAlg
  *
  * Small algorithm, which traverses the data store and
  * prints a summary of the leafs accessed during the run.
  *
  * @author:  M.Frank
  * @version: 1.0
  */
class StoreSnifferAlg : public Algorithm
{
public:
  /// Standard algorithm constructor
  using Algorithm::Algorithm;

  SmartIF<IDataManagerSvc> m_mgr;

  struct LeafInfo final {
    int count;
    int id;
    CLID clid;
  };
  typedef map<string, LeafInfo> SniffInfo;
  typedef map<string, map<int, int>> Correlations;

  SniffInfo m_info, m_curr;
  Correlations m_corr, m_links;

  size_t explore( IRegistry* pObj )
  {
    if ( pObj ) {
      auto mgr = eventSvc().as<IDataManagerSvc>();
      if ( mgr ) {
        vector<IRegistry*> leaves;
        StatusCode sc = m_mgr->objectLeaves( pObj, leaves );
        if ( sc.isSuccess() ) {
          for ( auto& pReg : leaves ) {
            /// We are only interested in leaves with an object
            if ( !pReg->address() || !pReg->object() ) continue;
            const string& id = pReg->identifier();
            auto j           = m_info.find( id );
            if ( j == m_info.end() ) {
              m_info[id]      = LeafInfo();
              j               = m_info.find( id );
              j->second.count = 0;
              j->second.id    = m_info.size();
              j->second.clid  = pReg->object()->clID();
            }
            m_curr[id].id    = m_info[id].id;
            m_curr[id].count = explore( pReg );
          }
          return leaves.size();
        }
      }
    }
    return 0;
  }

  /// Initialize
  StatusCode initialize() override
  {
    m_info.clear();
    m_mgr = eventSvc();
    return StatusCode::SUCCESS;
  }

  /// Finalize
  StatusCode finalize() override
  {
    auto& log = always();
    log << "== BEGIN ============= Access list content:" << m_info.size() << endmsg;
    for ( const auto& i : m_info ) {
      const LeafInfo& info = i.second;
      log << "== ITEM == " << right << setw( 4 ) << dec << info.id << " clid:" << right << setw( 8 ) << hex << info.clid
          << " Count:" << right << setw( 6 ) << dec << info.count << " " << i.first + ":" << endmsg;
      auto c = m_corr.find( i.first );
      if ( c != m_corr.end() ) {
        int cnt = 0;
        log << "== CORRELATIONS:" << ( *c ).second.size() << endmsg;
        for ( const auto& k : c->second ) {
          if ( k.second > 0 ) {
            log << dec << k.first << ":" << k.second << "  ";
            if ( ++cnt == 10 ) {
              cnt = 0;
              log << endmsg;
            }
          }
        }
        if ( cnt > 0 ) log << endmsg;
      }
      auto l = m_links.find( i.first );
      if ( l != m_links.end() ) {
        int cnt = 0;
        log << "== LINKS:" << l->second.size() << endmsg;
        for ( const auto& k : l->second ) {
          if ( k.second > 0 ) {
            log << dec << k.first << ":" << k.second << "  ";
            if ( ++cnt == 10 ) {
              cnt = 0;
              log << endmsg;
            }
          }
        }
        if ( cnt > 0 ) log << endmsg;
      }
    }
    always() << "== END =============== Access list content:" << m_info.size() << endmsg;
    m_info.clear();
    m_mgr = nullptr;
    return StatusCode::SUCCESS;
  }

  /// Execute procedure
  StatusCode execute() override
  {
    SmartDataPtr<DataObject> root( eventSvc(), "/Event" );
    if ( root ) {
      m_curr.clear();
      auto& evnt = m_curr["/Event"];
      evnt.count = explore( root->registry() );
      evnt.clid  = root->clID();
      evnt.id    = m_curr.size();
      for ( const auto& i : m_curr ) m_info[i.first].count++;
      for ( const auto& i : m_info ) {
        const string& nam = i.first;
        // const LeafInfo& leaf = (*i).second;
        auto c = m_corr.find( nam );
        if ( c == m_corr.end() ) {
          m_corr[nam] = {};
          c           = m_corr.find( nam );
        }
        for ( const auto& l : m_curr ) {
          const auto& id                = l.second.id;
          auto k                        = c->second.find( id );
          if ( k == c->second.end() ) k = c->second.emplace( id, 0 ).first;
          ++( k->second );
        }

        c                           = m_links.find( nam );
        if ( c == m_links.end() ) c = m_links.emplace( nam, std::map<int, int>{} ).first;
        if ( m_curr.find( nam ) == m_curr.end() ) continue;

        SmartDataPtr<DataObject> obj( eventSvc(), nam );
        if ( !obj ) continue;

        LinkManager* m = obj->linkMgr();
        for ( long l = 0; l < m->size(); ++l ) {
          auto* lnk = m->link( l );
          auto il   = m_curr.find( lnk->path() );
          // cout << "Link:" << lnk->path() << " " << (char*)(il != m_curr.end() ? "Found" : "Not there") << endl;
          if ( il == m_curr.end() ) continue;
          if ( !lnk->object() ) continue;
          const auto& id                = il->second.id;
          auto k                        = c->second.find( id );
          if ( k == c->second.end() ) k = c->second.emplace( id, 0 ).first;
          ++( k->second );
        }
      }
    }
    return StatusCode::SUCCESS;
  }
};

DECLARE_COMPONENT( StoreSnifferAlg )
