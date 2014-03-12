//	====================================================================
//  StoreExplorerAlg.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#include "GaudiKernel/ObjectContainerBase.h"
#include "GaudiKernel/ObjectList.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/KeyedContainer.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"

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
  /// Job option to limit printout to first nnn events
  long              m_print;
  /// Job option to set the printout frequency
  double            m_frequency;
  /// Flag to check if relations should be followed
  bool              m_exploreRelations;
  /// Flag to indicate if missing entities should be printed
  long              m_printMissing;
  /// Internal counter to trigger printouts
  long              m_total;
  /// Internal counter to adjust printout frequency
  long              m_frqPrint;
  /// Flag to load non existing items
  bool              m_load;
  /// Flag to test access to objects (DataObject and ContainedObject)
  bool              m_testAccess;
  /// Flag to indicate if foreign files should be opened
  bool              m_accessForeign;
  /// Reference to data provider service
  IDataProviderSvc* m_dataSvc;
  /// Name of the data provider service
  std::string       m_dataSvcName;
  /// Name of the root leaf (obtained at initialize)
  std::string       m_rootName;
public:

  /// Standard algorithm constructor
  StoreExplorerAlg(const std::string& name, ISvcLocator* pSvcLocator)
  :	Algorithm(name, pSvcLocator), m_dataSvc(0)
  {
    m_total = m_frqPrint = 0;
    declareProperty("Load",             m_load = false);
    declareProperty("PrintEvt",         m_print = 1);
    declareProperty("PrintMissing",     m_printMissing = 0);
    declareProperty("PrintFreq",        m_frequency = 0.0);
    declareProperty("ExploreRelations", m_exploreRelations = false);
    declareProperty("DataSvc",          m_dataSvcName="EventDataSvc");
    declareProperty("TestAccess",       m_testAccess = false);
    declareProperty("AccessForeign",    m_accessForeign = false);
  }
  /// Standard Destructor
  virtual ~StoreExplorerAlg()     {
  }

  template <class T>
  std::string access(T* p)  {
    if ( p )  {
      std::stringstream s;
      for (typename T::const_iterator i = p->begin(); i != p->end(); ++i )  {
        int idx = p->index(*i);
        s << idx << ":" << (*i)->clID() << ",";
      }
      std::string result = s.str();
      return result.substr(0, result.length()-2);
    }
    return "Access FAILED.";
  }


  /// Print datastore leaf
  void printObj(IRegistry* pReg, std::vector<bool>& flg)    {
    MsgStream log(msgSvc(), name());
    log << MSG::INFO;
    for (size_t j = 1; j < flg.size(); j++ )     {
      if ( !flg[j-1] && flg[j] ) log << "| ";
      else if ( flg[j] ) log << "  ";
      else          log << "| ";
    }
    log << "+--> " << pReg->name();
    if ( pReg->address() )  {
      log << " [Address: CLID="
	  << std::showbase << std::hex << pReg->address()->clID();
      log << " Type=" << (void*)pReg->address()->svcType() << "]";
    }
    else  {
      log << " [No Address]";
    }
    DataObject* p = pReg->object();
    if ( p )   {
      try  {
        std::string typ = System::typeinfoName(typeid(*p));
        if ( m_testAccess )  {
          p->clID();
        }
        log << "  " << typ.substr(0,32);
      }
      catch (...)  {
        log << "Access test FAILED";
      }
    }
    else    {
      log << "  (Unloaded) ";
    }
    ObjectContainerBase* base = dynamic_cast<ObjectContainerBase*>(p);
    if ( base )  {
      try   {
        int numObj = base->numberOfObjects();
        const CLID id = p->clID();
        log << " [" << numObj << "]";
        if ( m_testAccess )  {
          CLID idd = id>>16;
          switch(idd) {
            case CLID_ObjectList>>16:                /* ObjectList    */
              access((ObjectList<ContainedObject>*)base);
              break;
            case CLID_ObjectVector>>16:              /* ObjectVector  */
              access((ObjectVector<ContainedObject>*)base);
              break;
            case (CLID_ObjectVector+0x00030000)>>16: /* Keyed Map     */
              access((KeyedContainer<KeyedObject<int>,Containers::Map>*)base);
              break;
            case (CLID_ObjectVector+0x00040000)>>16: /* Keyed Hashmap */
              access((KeyedContainer<KeyedObject<int>,Containers::HashMap>*)base);
              break;
            case (CLID_ObjectVector+0x00050000)>>16: /* Keyed array   */
              access((KeyedContainer<KeyedObject<int>,Containers::Array>*)base);
              break;
          }
        }
      }
      catch (...)  {
        log << "Access test FAILED";
      }
    }
    log << endmsg;
  }

  void explore(IRegistry* pObj, std::vector<bool>& flg)    {
    printObj(pObj, flg);
    if ( 0 != pObj )    {
      SmartIF<IDataManagerSvc> mgr(eventSvc());
      if ( mgr )    {
        typedef std::vector<IRegistry*> Leaves;
        Leaves leaves;
        StatusCode sc = mgr->objectLeaves(pObj, leaves);
        const std::string* par0 = 0;
        if ( pObj->address() )  {
          par0 = pObj->address()->par();
        }
        if ( sc.isSuccess() )  {
          for ( Leaves::const_iterator i=leaves.begin(); i != leaves.end(); i++ )   {
            const std::string& id = (*i)->identifier();
            DataObject* p = 0;
            if ( !m_accessForeign && (*i)->address() )  {
              if ( par0 )  {
                const std::string* par1 = (*i)->address()->par();
                if ( par1 )  {
                  if ( par0[0] != par1[0] )  {
                    continue;
                  }
                }
              }
            }
            if ( m_load )  {
              sc = eventSvc()->retrieveObject(id, p);
            }
            else {
              sc = eventSvc()->findObject(id, p);
            }
            if ( sc.isSuccess() )  {
              if ( id != "/Event/Rec/Relations" || m_exploreRelations )   {
                flg.push_back(i+1 == leaves.end());
                explore(*i, flg);
                flg.pop_back();
              }
            }
            else {
              flg.push_back(i+1 == leaves.end());
              printObj(*i, flg);
              flg.pop_back();
            }
          }
        }
      }
    }
  }

  /// Initialize
  virtual StatusCode initialize()   {
    MsgStream log(msgSvc(), name());
    m_rootName = "";
    StatusCode sc = service(m_dataSvcName, m_dataSvc, true);
    if ( sc.isSuccess() )  {
      SmartIF<IDataManagerSvc> mgr(m_dataSvc);
      if ( mgr )  {
        m_rootName = mgr->rootName();
        return sc;
      }
      log << MSG::ERROR << "Failed to retrieve IDataManagerSvc interface." << endmsg;
      return StatusCode::FAILURE;
    }
    log << MSG::ERROR << "Failed to access service \""
        << m_dataSvcName << "\"." << endmsg;
    return StatusCode::FAILURE;
  }

  /// Finalize
  virtual StatusCode finalize() {
    if ( m_dataSvc ) m_dataSvc->release();
    m_dataSvc = 0;
    return StatusCode::SUCCESS;
  }

  /// Execute procedure
  virtual StatusCode execute()    {
    MsgStream log(msgSvc(), name());
    SmartDataPtr<DataObject>   root(m_dataSvc,m_rootName);
    if ( ((m_print > m_total++) || (m_frequency*m_total > m_frqPrint)) && root )    {
      if ( m_frequency*m_total > m_frqPrint )  {
        m_frqPrint++;
      }
      std::string store_name = "Unknown";
      IRegistry* pReg = root->registry();
      if ( pReg )  {
        SmartIF<IService> isvc(pReg->dataSvc());
        if ( isvc )  {
          store_name = isvc->name();
        }
      }
      log << MSG::INFO << "========= " << m_rootName << "["
          << "0x" << std::hex << (unsigned long) root.ptr() << std::dec
          << "@" << store_name << "]:" << endmsg;
      std::vector<bool> flg;
      flg.push_back(true);
      explore(root->registry(), flg);
      return StatusCode::SUCCESS;
    }
    else if ( root )   {
      return StatusCode::SUCCESS;
    }
    log << MSG::ERROR << "Cannot retrieve \"/Event\"!" << endmsg;
    return StatusCode::FAILURE;
  }
};

DECLARE_COMPONENT(StoreExplorerAlg)
