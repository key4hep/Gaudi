// $Id: HistogramSvc.cpp,v 1.28 2007/09/26 16:14:47 marcocle Exp $
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#pragma warning(disable:2259)
#endif
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#pragma warning(disable:4996)
#endif
// ============================================================================
// Include files
// ============================================================================
// STD&STL
// ============================================================================
#include <cstdlib>
#include <stdexcept>
#include <sstream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/xtoa.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/GenericAddress.h"
// ============================================================================
// Local
// ============================================================================
#include "HistogramSvc.h"
// ============================================================================
// Instantiation of a factory class used by clients
DECLARE_COMPONENT(HistogramSvc)
// ============================================================================
using namespace AIDA;
// ============================================================================
namespace
{
  // ==========================================================================
  inline std::string histoAddr
  ( const std::string& name)
  {
    if ( 0 == name.find ( "/stat/" ) ){ return std::string( name , 6 ) ; }
    return name ;
  }
  // ==========================================================================
  inline std::string histoAddr
  ( const DataObject*  obj ,
    const std::string& rel )
  {
    if ( 0 == obj      ) { return rel  ; }
    IRegistry* reg = obj->registry() ;
    if ( 0 == reg      ) { return rel  ; }
    const std::string& name = reg->identifier() ;
    //
    if ( rel  .empty() ) { return histoAddr ( name ) ; }
    if ( '/' == name[name.size()-1] ||
         '/' == rel[0] ) { return histoAddr ( name + rel ) ; }
    return histoAddr ( name + "/" + rel ) ;
  }
  // ==========================================================================
}
//------------------------------------------------------------------------------
std::string HistogramSvc::_STR(int i)  {
  std::ostringstream txt; txt << i;
  return txt.str();
}
//------------------------------------------------------------------------------
StatusCode HistogramSvc::registerObject(CSTR full, IBaseHistogram* obj)  {
  std::pair<std::string,std::string> split=i_splitPath(full);
  return registerObject(split.first, split.second, obj);
}
//------------------------------------------------------------------------------
StatusCode HistogramSvc::registerObject
(DataObject* pPar,CSTR obj,IBaseHistogram* hObj) {
  // Set the histogram id
  if (obj[0] == SEPARATOR)    {
    // hObj->setTitle(obj.substr(1) + "|" + hObj->title());
    if (!hObj->annotation().addItem("id", obj.substr(1)))
      hObj->annotation().setValue("id", obj.substr(1));
  }
  else {
    // hObj->setTitle(obj + "|" + hObj->title());
    if (!hObj->annotation().addItem("id", obj))
      hObj->annotation().setValue("id", obj);
  }
  // Register the histogram in the histogram data store
  return DataSvc::registerObject(pPar,obj,__cast(hObj));
}

// Helper for 2D projections
AIDA::IHistogram2D*
HistogramSvc::i_project(CSTR nameAndTitle,const IHistogram3D& h, CSTR dir)  {
  TH3D *h3d = Gaudi::getRepresentation<IHistogram3D,TH3D>(h);
  if ( h3d )  {
    TH2D *h2d = dynamic_cast<TH2D*>(h3d->Project3D(dir.c_str()));
    if ( h2d )  {
      std::pair<DataObject*,H2D*> r=Gaudi::createH2D(h2d);
      if ( r.second && registerObject(nameAndTitle,r.second).isSuccess() )  {
        return r.second;
      }
    }
  }
  return 0;
}

//------------------------------------------------------------------------------
// ASCII output
//------------------------------------------------------------------------------
std::ostream& HistogramSvc::print(IBaseHistogram* h, std::ostream& s) const  {
  Gaudi::HistogramBase* b = dynamic_cast<Gaudi::HistogramBase*>(h);
  if(0 != b) return b->print(s);
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "Unknown histogram type: Cannot cast to Gaudi::HistogramBase."
      << endmsg;
  return s;
}
//------------------------------------------------------------------------------
std::ostream& HistogramSvc::write(IBaseHistogram* h, std::ostream& s) const  {
  Gaudi::HistogramBase* b = dynamic_cast<Gaudi::HistogramBase*>(h);
  if(0 != b) return b->write(s);
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "Unknown histogram type: Cannot cast to Gaudi::HistogramBase."
      << endmsg;
  return s;
}
//------------------------------------------------------------------------------
int HistogramSvc::write(IBaseHistogram* h, const char* file_name) const  {
  Gaudi::HistogramBase* b = dynamic_cast<Gaudi::HistogramBase*>(h);
  if(0 != b) return b->write(file_name);
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "Unknown histogram type: Cannot cast to Gaudi::HistogramBase."
      << endmsg;
  return 0;
}
//------------------------------------------------------------------------------
std::pair<std::string,std::string> HistogramSvc::i_splitPath(CSTR full)   {
  std::string tmp = full;
  if (tmp[0] != SEPARATOR)    {
    tmp.insert(tmp.begin(), SEPARATOR);
    tmp.insert(tmp.begin(), m_rootName.begin(), m_rootName.end());
  }
  // Remove trailing "/" from newPath if it exists
  if (tmp.rfind(SEPARATOR) == tmp.length()-1) {
    tmp.erase(tmp.rfind(SEPARATOR),1);
  }
  int sep = tmp.rfind(SEPARATOR);
  return std::pair<std::string,std::string>
    (tmp.substr(0,sep),tmp.substr(sep,tmp.length()-sep));
}
//------------------------------------------------------------------------------
DataObject* HistogramSvc::createPath(CSTR newPath)  {
  std::string tmpPath = newPath;
  if (tmpPath[0] != SEPARATOR)    {
    tmpPath.insert(tmpPath.begin(), SEPARATOR);
    tmpPath.insert(tmpPath.begin(), m_rootName.begin(), m_rootName.end());
  }
  // Remove trailing "/" from newPath if it exists
  if (tmpPath.rfind(SEPARATOR) == tmpPath.length()-1) {
    tmpPath.erase(tmpPath.rfind(SEPARATOR),1);
  }
  DataObject* pObject = 0;
  StatusCode sc = DataSvc::findObject(tmpPath, pObject);
  if(sc.isSuccess()) {
    return pObject;
  }
  int sep = tmpPath.rfind(SEPARATOR);
  std::string rest(tmpPath, sep+1, tmpPath.length()-sep);
  std::string subPath(tmpPath, 0, sep);
  if(0 != sep) {
    createPath(subPath);
  }
  else {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << "Unable to create the histogram path" << endmsg;
    return 0;
  }
  pObject = createDirectory(subPath, rest);
  return pObject;
}
//------------------------------------------------------------------------------
DataObject* HistogramSvc::createDirectory(CSTR parentDir,CSTR subDir) {
  DataObject*  directory = new DataObject();
  if (0 != directory)  {
    DataObject* pnode;
    StatusCode status = DataSvc::retrieveObject(parentDir, pnode);
    if(status.isSuccess()) {
      status = DataSvc::registerObject(pnode, subDir, directory);
      if (!status.isSuccess())   {
        MsgStream log(msgSvc(), name());
        log << MSG::ERROR << "Unable to create the histogram directory: "
                          << parentDir << "/" << subDir << endmsg;
        delete directory;
        return 0;
      }
    }
    else {
      MsgStream log(msgSvc(), name());
      log << MSG::ERROR << "Unable to create the histogram directory: "
                        << parentDir << "/" << subDir << endmsg;
      delete directory;
      return 0;
    }
  }
  return directory;
}
//------------------------------------------------------------------------------
HistogramSvc::~HistogramSvc()   {
  setDataLoader( 0 ).ignore();
  clearStore().ignore();
}
//------------------------------------------------------------------------------
StatusCode HistogramSvc::connectInput(CSTR ident) {
  MsgStream log (msgSvc(), name());
  DataObject* pO = 0;
  StatusCode status = this->findObject(m_rootName, pO);
  if (status.isSuccess())   {
    Tokenizer tok(true);
    std::string::size_type loc = ident.find(" ");
    std::string filename, auth, svc = "", typ = "";
    std::string logname = ident.substr(0,loc);
    tok.analyse(ident.substr(loc+1,ident.length()), " ", "", "", "=", "'", "'");
    for (Tokenizer::Items::iterator i = tok.items().begin();
         i != tok.items().end(); i++) {
      CSTR tag = (*i).tag();
      switch(::toupper(tag[0]))   {
      case 'F':   // FILE='<file name>'
      case 'D':   // DATAFILE='<file name>'
        filename = (*i).value();
        break;
      case 'T':   // TYP='<HBOOK,ROOT,OBJY,...>'
        typ = (*i).value();
        break;
      default:
        break;
      }
    }
    if (typ.length() > 0)    {
      // Now add the registry entry to the store
      std::string entryname = m_rootName;
      entryname += '/';
      entryname += logname;
      GenericAddress* pA = 0;
      switch(::toupper(typ[0])) {
      case 'H':
        pA=new GenericAddress(HBOOK_StorageType,CLID_StatisticsFile,
                              filename,entryname,0,'O');
        break;
      case 'R':
        pA=new GenericAddress(ROOT_StorageType,CLID_StatisticsFile,
                              filename,entryname,0,'O');
        break;
      }
      if (0 != pA)    {
        status = registerAddress(pO, logname, pA);
        if (status.isSuccess())    {
          log << MSG::INFO << "Added stream file:" << filename
              << " as " << logname << endmsg;
          return status;
        }
        pA->release();
      }
    }
  }
  log << MSG::ERROR << "Cannot add " << ident << " invalid filename!" << endmsg;
  return StatusCode::FAILURE;
}
//------------------------------------------------------------------------------
StatusCode HistogramSvc::initialize()   {
  MsgStream log(msgSvc(), name());
  StatusCode status = DataSvc::initialize();
  // Set root object
  if (status.isSuccess()) {
    DataObject* rootObj = new DataObject();
    status = setRoot("/stat", rootObj);
    if (!status.isSuccess()) {
      log << MSG::ERROR << "Unable to set hstogram data store root." << endmsg;
      delete rootObj;
      return status;
    }
    IConversionSvc* svc = 0;
    status = service("HistogramPersistencySvc",svc,true);
    if ( status.isSuccess() )   {
      setDataLoader( svc ).ignore();
      svc->release();
    }
    else  {
      log << MSG::ERROR << "Could not find HistogramPersistencySvc." << endmsg;
      return status;
    }
    // Connect all input streams (if any)
    for (DBaseEntries::iterator j = m_input.begin(); j != m_input.end(); j++)    {
      status = connectInput(*j);
      if (!status.isSuccess())  {
        return status;
      }
    }
  }
  if ( !m_defs1D.empty() )
  {
    log << MSG::INFO << " Predefined 1D-Histograms: " << endmsg ;
    for ( Histo1DMap::const_iterator ih = m_defs1D.begin() ;
          m_defs1D.end() != ih ; ++ih )
    {
      log << MSG::INFO
          << " Path='"       << ih->first  << "'"
          << " Description " << ih->second << endmsg ;
    }
  }
  return status;
}
//------------------------------------------------------------------------------
StatusCode HistogramSvc::reinitialize()   {
  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
IHistogram1D* HistogramSvc::sliceX
(CSTR name,const IHistogram2D& h,int idxY1,int idxY2) {
  std::pair<DataObject*,IHistogram1D*> o(0,0);
  try {
    int firstbin = Gaudi::Axis::toRootIndex(idxY1,h.yAxis().bins());
    int lastbin  = Gaudi::Axis::toRootIndex(idxY2,h.yAxis().bins());
    o = Gaudi::slice1DX(name, h, firstbin, lastbin);
  }
  catch ( ... ) {
    throw GaudiException("Cannot cast 2D histogram to H2D to create sliceX `"
			 + name + "'!", "HistogramSvc", StatusCode::FAILURE);
  }
  if ( o.first && registerObject(name,(IBaseHistogram*)o.second).isSuccess() )  {
    return o.second;
  }
  delete o.first;
  throw GaudiException("Cannot create sliceX `" + name + "' of 2D histogram!",
    "HistogramSvc", StatusCode::FAILURE);
}
//------------------------------------------------------------------------------
IHistogram1D*
HistogramSvc::sliceY(CSTR name,const IHistogram2D& h,int indexX1,int indexX2) {
  std::pair<DataObject*,IHistogram1D*> o(0,0);
  try {
    int firstbin = Gaudi::Axis::toRootIndex( indexX1, h.xAxis().bins() );
    int lastbin  = Gaudi::Axis::toRootIndex( indexX2, h.xAxis().bins() );
    o = Gaudi::slice1DY(name, h, firstbin, lastbin);
  }
  catch ( ... ) {
    throw GaudiException("Cannot create sliceY `"+name+"'!",
                         "HistogramSvc",StatusCode::FAILURE);
  }
  // name stands here for the fullPath of the histogram
  if ( o.first && registerObject(name,(IBaseHistogram*)o.second).isSuccess() )  {
    return o.second;
  }
  delete o.first;
  throw GaudiException("Cannot create sliceY `"+name+"' of 2D histogram!",
			"HistogramSvc", StatusCode::FAILURE);
}
//------------------------------------------------------------------------------
bool HistogramSvc::destroy( IBaseHistogram* hist ) {
  StatusCode sc = unregisterObject( dynamic_cast<IHistogram*>(hist) );
  if ( !sc.isSuccess() ) return false;
  if ( hist ) delete hist;
  return true;
}
// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book
(DataObject* pPar, CSTR rel, CSTR title, DBINS(x))
{
  if ( m_defs1D.empty () )
  { return i_book(pPar,rel,title,Gaudi::createH1D(title, BINS(x))); }
  std::string hn = histoAddr ( pPar , rel ) ;
  Histo1DMap::const_iterator ifound = m_defs1D.find( hn ) ;
  if ( m_defs1D.end() == ifound )
  { return i_book(pPar,rel,title,Gaudi::createH1D(title, BINS(x))); }
  if (msgLevel(MSG::DEBUG)) {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::DEBUG
        << " Redefine the parameters for the histogram '" + hn + "' to be "
        << ifound->second
        << endmsg;
  }
  m_mods1D.insert ( hn ) ;
  return i_book ( pPar , rel , ifound -> second.title   () ,
                  Gaudi::createH1D
                  ( ifound -> second.title   () ,
                    ifound -> second.bins    () ,
                    ifound -> second.lowEdge () ,
                    ifound -> second.lowEdge () ) ) ;
}
// ============================================================================
// constructor
// ============================================================================
HistogramSvc::HistogramSvc(CSTR nam, ISvcLocator* svc)
  : base_class(nam, svc)
  , m_defs1D ()
  , m_mods1D ()
{
  // Properties can be declared here
  m_rootName = "/stat";
  m_rootCLID = CLID_DataObject;
  declareProperty ( "Input",  m_input);
  declareProperty ( "Predefined1DHistos" , m_defs1D ,
                    "Histograms with predefined parameters" ) ;
  // update handler
  Property* p = Gaudi::Utils::getProperty ( this , "Predefined1DHistos" ) ;
  p->declareUpdateHandler ( &HistogramSvc::update1Ddefs , this ) ;

}
// ============================================================================
// handler to be invoked for updating property m_defs1D
// ============================================================================
namespace
{
  inline size_t removeLeading
  ( HistogramSvc::Histo1DMap& m , const std::string& lead = "/stat/")
  {
    for ( HistogramSvc::Histo1DMap::iterator it = m.begin() ;
          m.end() != it ; ++it )
    {
      if ( 0 == it->first.find ( lead ) )
      {
        std::string       addr = std::string( it->first , lead.size() ) ;
        Gaudi::Histo1DDef hdef = it->second ;
        m.erase ( it ) ;                            // remove
        m[addr] = hdef ;                            // insert
        return 1 + removeLeading ( m , lead ) ;     // return
      }
    }
    return 0 ;
  }
}
// ============================================================================
void HistogramSvc::update1Ddefs ( Property& )
{
  // check and remove the leading '/stat/'
  removeLeading  ( m_defs1D , "/stat/" ) ;
}
// ============================================================================
// finalize the service
StatusCode HistogramSvc::finalize     ()
{
  if ( !m_mods1D.empty() )
  {
    MsgStream log ( msgSvc () , name () ) ;
    if (msgLevel(MSG::DEBUG))
      log << MSG::DEBUG
          << " Substituted histograms #" << m_mods1D.size() << " : " << endmsg;
    for ( std::set<std::string>::const_iterator ih = m_mods1D.begin() ;
          m_mods1D.end() != ih ; ++ih )
    {
      if (msgLevel(MSG::DEBUG))
        log << MSG::DEBUG << " Path='" << (*ih) << "'" ;
      Histo1DMap::const_iterator im = m_defs1D.find( *ih ) ;
      if ( m_defs1D.end() != im ) { log << "  " << im->second ; }
    }
    m_mods1D.clear() ;
  }
  return DataSvc::finalize () ;
}
// ============================================================================
// The END
// ============================================================================
