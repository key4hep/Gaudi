// Framework include files
#include "GaudiKernel/Debugger.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/strcasecmp.h"
#include "GaudiUtils/IFileCatalog.h"
#include "IODataManager.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IIncidentSvc.h"

#include <set>

using namespace Gaudi;

DECLARE_COMPONENT(IODataManager)

enum { S_OK = StatusCode::SUCCESS, S_ERROR=StatusCode::FAILURE };

static std::set<std::string>    s_badFiles;

IODataManager::IODataManager(CSTR nam, ISvcLocator* svcloc)
  : base_class(nam, svcloc), m_ageLimit(2)
{
  declareProperty("CatalogType",     m_catalogSvcName="Gaudi::MultiFileCatalog/FileCatalog");
  declareProperty("UseGFAL",         m_useGFAL = true);
  declareProperty("QuarantineFiles", m_quarantine = true);
  declareProperty("AgeLimit",        m_ageLimit = 2);
  declareProperty("DisablePFNWarning", m_disablePFNWarning = false,
                  "if set to True, we will not report when a file "
                  "is opened by it's physical name");
}

/// IService implementation: Db event selector override
StatusCode IODataManager::initialize()  {
  // Initialize base class
  StatusCode status = Service::initialize();
  MsgStream log(msgSvc(), name());
  if ( !status.isSuccess() )    {
    log << MSG::ERROR << "Error initializing base class Service!" << endmsg;
    return status;
  }
  // Retrieve conversion service handling event iteration
  m_catalog = serviceLocator()->service(m_catalogSvcName);
  if( !m_catalog.isValid() ) {
    log << MSG::ERROR
        << "Unable to localize interface IFileCatalog from service:"
        << m_catalogSvcName << endmsg;
    return StatusCode::FAILURE;
  }
  m_incSvc = serviceLocator()->service("IncidentSvc");
  if( !m_incSvc.isValid() ) {
    log << MSG::ERROR << "Error initializing IncidentSvc Service!" << endmsg;
    return status;
  }

  return status;
}

/// IService implementation: finalize the service
StatusCode IODataManager::finalize()  {
  m_catalog = 0; // release
  return Service::finalize();
}

// Small routine to issue exceptions
StatusCode IODataManager::error(CSTR msg, bool rethrow)  {
  MsgStream log(msgSvc(),name());
  log << MSG::ERROR << "Error: " << msg << endmsg;
  if ( rethrow )  {
    System::breakExecution();
  }
  return S_ERROR;
}
/// Get connection by owner instance (0=ALL)
IODataManager::Connections IODataManager::connections(const IInterface* owner) const  {
  Connections conns;
  for(ConnectionMap::const_iterator i=m_connectionMap.begin(); i!=m_connectionMap.end();++i) {
    IDataConnection* c = (*i).second->connection;
    if ( 0 == owner || c->owner() == owner )
      conns.push_back(c);
  }
  return conns;
}

/// Connect data file for writing
StatusCode IODataManager::connectRead(bool keep_open, Connection* con)  {
  if ( !establishConnection(con) )  {
    return connectDataIO(UNKNOWN,Connection::READ,con->name(),"UNKNOWN",keep_open,con);
  }
  std::string dsn = con ? con->name() : std::string("Unknown");
  return error("Failed to connect to data:"+dsn,false);
}

/// Connect data file for reading
StatusCode IODataManager::connectWrite(Connection* con,IoType mode,CSTR doctype)  {
  if ( !establishConnection(con) )  {
    return connectDataIO(UNKNOWN,mode,con->name(),doctype,true,con);
  }
  std::string dsn = con ? con->name() : std::string("Unknown");
  return error("Failed to connect to data:"+dsn,false);
}

/// Read raw byte buffer from input stream
StatusCode IODataManager::read(Connection* con, void* const data, size_t len)  {
  return establishConnection(con).isSuccess() ? con->read(data,len) : S_ERROR;
}

/// Write raw byte buffer to output stream
StatusCode IODataManager::write(Connection* con, const void* data, int len)  {
  return establishConnection(con).isSuccess() ? con->write(data,len) : S_ERROR;
}

/// Seek on the file described by ioDesc. Arguments as in ::seek()
long long int IODataManager::seek(Connection* con, long long int where, int origin)  {
  return establishConnection(con).isSuccess() ? con->seek(where,origin) : -1;
}

StatusCode IODataManager::disconnect(Connection* con)    {
  if ( con )  {
    std::string dataset = con->name();
    std::string dsn = dataset;
    StatusCode sc = con->disconnect();
    if ( ::strncasecmp(dsn.c_str(),"FID:",4)==0 )
      dsn = dataset.substr(4);
    else if ( ::strncasecmp(dsn.c_str(),"LFN:",4)==0 )
      dsn = dataset.substr(4);
    else if ( ::strncasecmp(dsn.c_str(),"PFN:",4)==0 )
      dsn = dataset.substr(4);

    FidMap::iterator j = m_fidMap.find(dataset);
    if ( j != m_fidMap.end() )  {
      std::string fid = (*j).second;
      std::string gfal_name = "gfal:guid:" + fid;
      ConnectionMap::iterator i=m_connectionMap.find(fid);
      m_fidMap.erase(j);
      if ( (j=m_fidMap.find(fid)) != m_fidMap.end() )
        m_fidMap.erase(j);
      if ( (j=m_fidMap.find(gfal_name)) != m_fidMap.end() )
        m_fidMap.erase(j);
      if ( i != m_connectionMap.end() ) {
        if ( (*i).second )  {
          IDataConnection* c = (*i).second->connection;
          std::string pfn = c->pfn();
          if ( (j=m_fidMap.find(pfn)) != m_fidMap.end() )
            m_fidMap.erase(j);
          if ( c->isConnected() )  {
            MsgStream log(msgSvc(),name());
            c->disconnect();
            log << MSG::INFO << "Disconnect from dataset " << dsn
                << " [" << fid << "]" << endmsg;
          }
          delete (*i).second;
          m_connectionMap.erase(i);
        }
      }
    }
    return sc;
  }
  return S_ERROR;
}

StatusCode IODataManager::reconnect(Entry* e)  {
  StatusCode sc = S_ERROR;
  if ( e && e->connection )  {
    switch(e->ioType)  {
    case Connection::READ:
      sc = e->connection->connectRead();
      break;
    case Connection::UPDATE:
    case Connection::CREATE:
    case Connection::RECREATE:
      sc = e->connection->connectWrite(e->ioType);
      break;
    default:
      return S_ERROR;
    }
    if ( sc.isSuccess() && e->ioType == Connection::READ )  {
      std::vector<Entry*> to_retire;
      e->connection->resetAge();
      for(ConnectionMap::iterator i=m_connectionMap.begin(); i!=m_connectionMap.end();++i) {
        IDataConnection* c = (*i).second->connection;
        if ( e->connection != c && c->isConnected() && !(*i).second->keepOpen )  {
          c->ageFile();
          if ( c->age() > m_ageLimit ) {
            to_retire.push_back((*i).second);
          }
        }
      }
      if ( !to_retire.empty() )  {
        MsgStream log(msgSvc(),name());
        for(std::vector<Entry*>::iterator j=to_retire.begin(); j!=to_retire.end();++j)  {
          IDataConnection* c = (*j)->connection;
          c->disconnect();
          log << MSG::INFO << "Disconnect from dataset " << c->pfn()
              << " [" << c->fid() << "]" << endmsg;
        }
      }
    }
  }
  return sc;
}

/// Retrieve known connection
IIODataManager::Connection* IODataManager::connection(CSTR dataset) const  {
  FidMap::const_iterator j = m_fidMap.find(dataset);
  if ( j != m_fidMap.end() )  {
    ConnectionMap::const_iterator i=m_connectionMap.find((*j).second);
    return (i != m_connectionMap.end()) ? (*i).second->connection : 0;
  }
  return 0;
}

StatusCode IODataManager::establishConnection(Connection* con)  {
  if ( con )  {
    if ( !con->isConnected() )  {
      ConnectionMap::const_iterator i=m_connectionMap.find(con->name());
      if ( i != m_connectionMap.end() )  {
        Connection* c = (*i).second->connection;
        if ( c != con )  {
          m_incSvc->fireIncident(Incident(con->name(),IncidentType::FailInputFile));
          return error("Severe logic bug: Twice identical connection object for DSN:"+con->name(),true);
        }
        if ( reconnect((*i).second).isSuccess() ) {
          return S_OK;
        }
      }
      return S_ERROR;
    }
    con->resetAge();
    return S_OK;
  }
  return error("Severe logic bug: No connection object avalible.",true);
}

StatusCode
IODataManager::connectDataIO(int typ, IoType rw, CSTR dataset, CSTR technology,bool keep_open,Connection* connection)  {
  MsgStream log(msgSvc(),name());
  std::string dsn = dataset;
  try  {
    StatusCode sc(StatusCode::SUCCESS,true);
    if ( ::strncasecmp(dsn.c_str(),"FID:",4)==0 )
      dsn = dataset.substr(4), typ = FID;
    else if ( ::strncasecmp(dsn.c_str(),"LFN:",4)==0 )
      dsn = dataset.substr(4), typ = LFN;
    else if ( ::strncasecmp(dsn.c_str(),"PFN:",4)==0 )
      dsn = dataset.substr(4), typ = PFN;
    else if ( typ == UNKNOWN )
      return connectDataIO(PFN, rw, dsn, technology, keep_open, connection);

    if(std::find(s_badFiles.begin(),s_badFiles.end(),dsn) != s_badFiles.end())  {
      m_incSvc->fireIncident(Incident(dsn,IncidentType::FailInputFile));
      return IDataConnection::BAD_DATA_CONNECTION;
    }
    if ( typ == FID )  {
      ConnectionMap::iterator fi = m_connectionMap.find(dsn);
      if ( fi == m_connectionMap.end() )  {
        IFileCatalog::Files files;
        m_catalog->getPFN(dsn,files);
        if ( files.size() == 0 ) {
          if ( !m_useGFAL )   {
            if ( m_quarantine ) s_badFiles.insert(dsn);
            m_incSvc->fireIncident(Incident(dsn,IncidentType::FailInputFile));
            error("connectDataIO> failed to resolve FID:"+dsn,false).ignore();
	    return IDataConnection::BAD_DATA_CONNECTION;
          }
          else if ( dsn.length() == 36 && dsn[8]=='-' && dsn[13]=='-' )  {
            std::string gfal_name = "gfal:guid:" + dsn;
            m_fidMap[dsn] = m_fidMap[dataset] = m_fidMap[gfal_name] = dsn;
            sc = connectDataIO(PFN, rw, gfal_name, technology, keep_open, connection);
            if ( sc.isSuccess() ) return sc;
            if ( m_quarantine ) s_badFiles.insert(dsn);
          }
          if ( m_quarantine ) s_badFiles.insert(dsn);
          m_incSvc->fireIncident(Incident(dsn,IncidentType::FailInputFile));
          error("connectDataIO> Failed to resolve FID:"+dsn,false).ignore();
          return IDataConnection::BAD_DATA_CONNECTION;
        }
        std::string pfn = files[0].first;
        m_fidMap[dsn] = m_fidMap[dataset] = m_fidMap[pfn] = dsn;
        sc = connectDataIO(PFN, rw, pfn, technology, keep_open, connection);
        if ( !sc.isSuccess() )  {
          if ( m_quarantine ) s_badFiles.insert(pfn);
          m_incSvc->fireIncident(Incident(pfn,IncidentType::FailInputFile));
          return IDataConnection::BAD_DATA_CONNECTION;
        }

        return sc;
      }
      return S_ERROR;
      //Connection* c = (*fi).second->connection;
      //sc = connectDataIO(PFN, rw, c->pfn(), technology, keep_open, connection);
      //if ( !sc.isSuccess() && m_quarantine ) s_badFiles.insert(c->pfn());
      //return sc;
    }
    std::string fid;
    FidMap::iterator j = m_fidMap.find(dsn);
    if ( j == m_fidMap.end() )  {
      IFileCatalog::Files files;
      switch(typ)  {
      case LFN:
        fid = m_catalog->lookupLFN(dsn);
        if ( fid.empty() )  {
          m_incSvc->fireIncident(Incident(dsn,IncidentType::FailInputFile));
          log << MSG::ERROR << "Failed to resolve LFN:" << dsn
              << " Cannot access this dataset." << endmsg;
          return IDataConnection::BAD_DATA_CONNECTION;
        }
        break;
      case PFN:
        fid = m_catalog->lookupPFN(dsn);
        if ( !fid.empty() ) m_catalog->getPFN(fid, files);
        if ( files.empty() )   {
          if ( rw == Connection::CREATE || rw == Connection::RECREATE )  {
            if ( fid.empty() ) fid = m_catalog->createFID();
            m_catalog->registerPFN(fid,dsn,technology);
            log << MSG::INFO << "Referring to dataset " << dsn
                << " by its file ID:" << fid << endmsg;
          }
          else  {
            fid = dsn;
          }
        }
        break;
      }
    }
    else {
      fid = (*j).second;
    }
    if ( typ == PFN )  {
      // Open PFN
      ConnectionMap::iterator fi = m_connectionMap.find(fid);
      if ( fi == m_connectionMap.end() )  {
        connection->setFID(fid);
        connection->setPFN(dsn);
        Entry* e = new Entry(technology, keep_open, rw, connection);
        // Here we open the file!
        if ( !reconnect(e).isSuccess() )   {
          delete e;
          if ( m_quarantine ) s_badFiles.insert(dsn);
          m_incSvc->fireIncident(Incident(dsn,IncidentType::FailInputFile));
          error("connectDataIO> Cannot connect to database: PFN="+dsn+" FID="+fid,false).ignore();
          return IDataConnection::BAD_DATA_CONNECTION;
        }
        fid = connection->fid();
        m_fidMap[dataset] = m_fidMap[dsn] = m_fidMap[fid] = fid;
        if (  !(rw==Connection::CREATE || rw==Connection::RECREATE) )  {
          if ( ! m_disablePFNWarning && strcasecmp(dsn.c_str(),fid.c_str()) == 0 )  {
            log << MSG::ERROR << "Referring to existing dataset " << dsn
                << " by its physical name." << endmsg;
            log << "You may not be able to navigate back to the input file"
                << " -- processing continues" << endmsg;
          }
        }
        m_connectionMap.insert(std::make_pair(fid,e));
        return S_OK;
      }
      // Here we open the file!
      if ( !reconnect((*fi).second).isSuccess() )   {
        if ( m_quarantine ) s_badFiles.insert(dsn);
        m_incSvc->fireIncident(Incident(dsn,IncidentType::FailInputFile));
        error("connectDataIO> Cannot connect to database: PFN="+dsn+" FID="+fid,false).ignore();
        return IDataConnection::BAD_DATA_CONNECTION;
      }
      return S_OK;
    }
    sc = connectDataIO(FID, rw, fid, technology, keep_open, connection);
    if ( !sc.isSuccess() && m_quarantine ) {
      s_badFiles.insert(fid);
    }
    else if ( typ == LFN ) {
      m_fidMap[dataset] = fid;
    }
    return sc;
  }
  catch (std::exception& e)  {
    error(std::string("connectDataIO> Caught exception:")+e.what(), false).ignore();
  }
  catch(...) {
    error(std::string("connectDataIO> Caught unknown exception"), false).ignore();
  }
  m_incSvc->fireIncident(Incident(dsn,IncidentType::FailInputFile));
  error("connectDataIO> The dataset "+dsn+" cannot be opened.",false).ignore();
  s_badFiles.insert(dsn);
  return IDataConnection::BAD_DATA_CONNECTION;
}
