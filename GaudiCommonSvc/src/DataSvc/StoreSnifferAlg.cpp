//  ====================================================================
//  StoreSnifferAlg.cpp
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
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
class StoreSnifferAlg : public Algorithm {
public:

  SmartIF<IDataManagerSvc> m_mgr;

  struct LeafInfo {
    int  count;
    int  id;
    CLID clid;
  };
  typedef map<string, LeafInfo> SniffInfo;
  typedef map<string,map<int,int> > Correlations;

  SniffInfo    m_info, m_curr;
  Correlations m_corr, m_links;

  /// Standard algorithm constructor
  StoreSnifferAlg(const string& name, ISvcLocator* pSvc) : Algorithm(name, pSvc)  {
  }
  /// Standard Destructor
  virtual ~StoreSnifferAlg()     {
  }

  size_t explore(IRegistry* pObj)    {
    if ( 0 != pObj )    {
      SmartIF<IDataManagerSvc> mgr(eventSvc());
      if ( mgr )    {
        typedef vector<IRegistry*> Leaves;
        Leaves leaves;
        StatusCode sc = m_mgr->objectLeaves(pObj, leaves);
        if ( sc.isSuccess() )  {
          for (Leaves::const_iterator i=leaves.begin(); i != leaves.end(); i++ )   {
            IRegistry* pReg = *i;
            const string& id = pReg->identifier();
            /// We are only interested in leaves with an object
            if ( pReg->address() && pReg->object() )  {
              SniffInfo::iterator j=m_info.find(id);
              if ( j == m_info.end() )   {
                m_info[id] = LeafInfo();
                j = m_info.find(id);
                (*j).second.count = 0;
                (*j).second.id    = m_info.size();
                (*j).second.clid  = pReg->object()->clID();
              }
              m_curr[id].id    = m_info[id].id;
              m_curr[id].count = explore(pReg);
            }
          }
          return leaves.size();
        }
      }
    }
    return 0;
  }

  /// Initialize
  virtual StatusCode initialize()   {
    m_info.clear();
    m_mgr = eventSvc();
    return StatusCode::SUCCESS;
  }

  /// Finalize
  virtual StatusCode finalize() {
    MsgStream log(msgSvc(), name());
    log << MSG::ALWAYS << "== BEGIN ============= Access list content:" << m_info.size() << endmsg;
    for(SniffInfo::const_iterator i=m_info.begin(); i!=m_info.end();++i) {
      const LeafInfo& info = (*i).second;
      log << "== ITEM == " << right << setw(4) << dec << info.id << " clid:"
          << right << setw(8) << hex << info.clid << " Count:"
          << right << setw(6) << dec << info.count << " "
          << (*i).first+":"
          << endmsg;
      Correlations::const_iterator c=m_corr.find((*i).first);
      if ( c != m_corr.end() ) {
        int cnt = 0;
        log << "== CORRELATIONS:" << (*c).second.size() << endmsg;
        for(map<int,int>::const_iterator k=(*c).second.begin(); k!=(*c).second.end();++k) {
          if ( (*k).second > 0 ) {
            log << dec << (*k).first << ":" << (*k).second << "  ";
            if ( ++cnt == 10 ) {
              cnt = 0;
              log << endmsg;
            }
          }
        }
        if ( cnt > 0 ) log << endmsg;
      }
      Correlations::const_iterator l=m_links.find((*i).first);
      if ( l != m_links.end() ) {
        int cnt = 0;
        log << "== LINKS:" << (*l).second.size() << endmsg;
        for(map<int,int>::const_iterator k=(*l).second.begin(); k!=(*l).second.end();++k) {
          if ( (*k).second > 0 ) {
            log << dec << (*k).first << ":" << (*k).second << "  ";
            if ( ++cnt == 10 ) {
              cnt = 0;
              log << endmsg;
            }
          }
        }
        if ( cnt > 0 ) log << endmsg;
      }
    }
    log << MSG::ALWAYS << "== END =============== Access list content:" << m_info.size() << endmsg;
    m_info.clear();
    m_mgr = 0;
    return StatusCode::SUCCESS;
  }

  /// Execute procedure
  virtual StatusCode execute()    {
    SmartDataPtr<DataObject> root(eventSvc(),"/Event");
    if ( root )    {
      m_curr.clear();
      m_curr["/Event"].count = explore(root->registry());
      m_curr["/Event"].clid  = root->clID();
      m_curr["/Event"].id    = m_curr.size();
      for(SniffInfo::const_iterator i=m_curr.begin(); i!=m_curr.end();++i) {
        LeafInfo& li = m_info[(*i).first];
        li.count++;
      }
      for(SniffInfo::const_iterator i=m_info.begin(); i!=m_info.end();++i) {
        const string& nam = (*i).first;
        // const LeafInfo& leaf = (*i).second;
        Correlations::iterator c=m_corr.find(nam);
        if ( c == m_corr.end() )  {
          m_corr[nam] = map<int,int>();
          c = m_corr.find(nam);
        }
        for(SniffInfo::const_iterator l=m_curr.begin(); l!=m_curr.end();++l) {
          const LeafInfo& li = (*l).second;
          map<int,int>::iterator k = (*c).second.find(li.id);
          if ( k==(*c).second.end() ) (*c).second[li.id] = 0;
          ++((*c).second[li.id]);
        }

        c=m_links.find(nam);
        if ( c == m_links.end() )  {
          m_links[nam] = map<int,int>();
          c = m_links.find(nam);
        }
        if ( m_curr.find(nam) != m_curr.end() ) {
          SmartDataPtr<DataObject> obj(eventSvc(),nam);
          if ( obj ) {
            LinkManager* m = obj->linkMgr();
            for(long l=0; l<m->size(); ++l) {
              LinkManager::Link* lnk=m->link(l);
              SniffInfo::const_iterator il=m_curr.find(lnk->path());
              // cout << "Link:" << lnk->path() << " " << (char*)(il != m_curr.end() ? "Found" : "Not there") << endl;
              if ( il != m_curr.end() ) {
                if ( lnk->object() ) {
                  const LeafInfo& li = (*il).second;
                  map<int,int>::iterator k = (*c).second.find(li.id);
                  if ( k==(*c).second.end() ) (*c).second[li.id] = 0;
                  ++((*c).second[li.id]);
                }
              }
            }
          }
        }
      }
      return StatusCode::SUCCESS;
    }
    return StatusCode::SUCCESS;
  }
};

DECLARE_COMPONENT(StoreSnifferAlg)
