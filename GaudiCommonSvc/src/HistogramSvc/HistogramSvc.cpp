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
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#  pragma warning( disable : 2259 )
#endif
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#  pragma warning( disable : 4996 )
#endif

// STD & STL
#include <cstdlib>
#include <sstream>
#include <stdexcept>

// GaudiKernel
#include <Gaudi/Property.h>
#include <GaudiKernel/AttribStringParser.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/GenericAddress.h>
#include <GaudiKernel/IConversionSvc.h>

// Local
#include "GaudiPI.h"
#include <GaudiCommonSvc/HistogramSvc.h>

namespace {
  using namespace AIDA;
  using std::string;

  string histoAddr( const string& name ) {
    if ( name.starts_with( "/stat/" ) ) { return string( name, 6 ); }
    return name;
  }

  string histoAddr( const DataObject* obj, const string& rel ) {
    if ( !obj ) { return rel; }
    IRegistry* reg = obj->registry();
    if ( !reg ) { return rel; }
    const string& name = reg->identifier();
    //
    if ( rel.empty() ) { return histoAddr( name ); }
    if ( '/' == name[name.size() - 1] || '/' == rel[0] ) { return histoAddr( name + rel ); }
    return histoAddr( name + "/" + rel );
  }

  size_t removeLeading( HistogramSvc::Histo1DMap& m, const string& lead = "/stat/" ) {
    auto it = std::find_if( m.begin(), m.end(), [&lead]( const auto& i ) { return 0 == i.first.find( lead ); } );
    if ( it == m.end() ) return 0;
    string            addr = string( it->first, lead.size() );
    Gaudi::Histo1DDef hdef = it->second;
    m.erase( it );                       // remove
    m[addr] = hdef;                      // insert
    return 1 + removeLeading( m, lead ); // return
  }
} // namespace

//------------------------------------------------------------------------------
StatusCode HistogramSvc::registerObject( const string& full, IBaseHistogram* obj ) {
  std::pair<string, string> split = i_splitPath( full );
  return registerObject( split.first, split.second, obj );
}

//------------------------------------------------------------------------------
StatusCode HistogramSvc::registerObject( DataObject* pPar, const string& obj, IBaseHistogram* hObj ) {
  // Set the histogram id
  if ( obj[0] == SEPARATOR ) {
    // hObj->setTitle(obj.substr(1) + "|" + hObj->title());
    if ( !hObj->annotation().addItem( "id", obj.substr( 1 ) ) ) hObj->annotation().setValue( "id", obj.substr( 1 ) );
  } else {
    // hObj->setTitle(obj + "|" + hObj->title());
    if ( !hObj->annotation().addItem( "id", obj ) ) hObj->annotation().setValue( "id", obj );
  }
  // Register the histogram in the histogram data store
  return DataSvc::registerObject( pPar, obj, ::detail::cast( hObj ) );
}

// Helper for 2D projections
AIDA::IHistogram2D* HistogramSvc::i_project( const string& nameAndTitle, const IHistogram3D& h, const string& dir ) {
  TH3D* h3d = Gaudi::getRepresentation<IHistogram3D, TH3D>( h );
  if ( h3d ) {
    TH2D* h2d = dynamic_cast<TH2D*>( h3d->Project3D( dir.c_str() ) );
    if ( h2d ) {
      std::pair<DataObject*, AIDA::IHistogram2D*> r = Gaudi::createH2D( serviceLocator(), nameAndTitle, h2d );
      if ( r.second && registerObject( nameAndTitle, r.second ).isSuccess() ) { return r.second; }
    }
  }
  return nullptr;
}

//------------------------------------------------------------------------------
// ASCII output
//------------------------------------------------------------------------------
std::ostream& HistogramSvc::print( IBaseHistogram* h, std::ostream& s ) const {
  Gaudi::HistogramBase* b = dynamic_cast<Gaudi::HistogramBase*>( h );
  if ( b ) return b->print( s );
  error() << "Unknown histogram type: Cannot cast to Gaudi::HistogramBase." << endmsg;
  return s;
}

//------------------------------------------------------------------------------
std::ostream& HistogramSvc::write( IBaseHistogram* h, std::ostream& s ) const {
  Gaudi::HistogramBase* b = dynamic_cast<Gaudi::HistogramBase*>( h );
  if ( b ) return b->write( s );
  error() << "Unknown histogram type: Cannot cast to Gaudi::HistogramBase." << endmsg;
  return s;
}

//------------------------------------------------------------------------------
int HistogramSvc::write( IBaseHistogram* h, const char* file_name ) const {
  Gaudi::HistogramBase* b = dynamic_cast<Gaudi::HistogramBase*>( h );
  if ( b ) return b->write( file_name );
  error() << "Unknown histogram type: Cannot cast to Gaudi::HistogramBase." << endmsg;
  return 0;
}

//------------------------------------------------------------------------------
std::pair<string, string> HistogramSvc::i_splitPath( const string& full ) {
  string tmp = full;
  if ( tmp[0] != SEPARATOR ) {
    tmp.insert( tmp.begin(), SEPARATOR );
    tmp.insert( tmp.begin(), m_rootName.begin(), m_rootName.end() );
  }
  // Remove trailing "/" from newPath if it exists
  if ( tmp.rfind( SEPARATOR ) == tmp.length() - 1 ) { tmp.erase( tmp.length() - 1, 1 ); }
  int sep = tmp.rfind( SEPARATOR );
  return { tmp.substr( 0, sep ), tmp.substr( sep + 1 ) };
}

//------------------------------------------------------------------------------
DataObject* HistogramSvc::createPath( const string& newPath ) {
  string tmpPath = newPath;
  if ( tmpPath[0] != SEPARATOR ) {
    tmpPath.insert( tmpPath.begin(), SEPARATOR );
    tmpPath.insert( tmpPath.begin(), m_rootName.begin(), m_rootName.end() );
  }
  // Remove trailing "/" from newPath if it exists
  if ( tmpPath.rfind( SEPARATOR ) == tmpPath.length() - 1 ) { tmpPath.erase( tmpPath.rfind( SEPARATOR ), 1 ); }
  DataObject* pObject = nullptr;
  StatusCode  sc      = findObject( tmpPath, pObject );
  if ( sc.isSuccess() ) { return pObject; }
  int    sep = tmpPath.rfind( SEPARATOR );
  string rest( tmpPath, sep + 1, tmpPath.length() - sep );
  string subPath( tmpPath, 0, sep );
  if ( 0 != sep ) {
    createPath( subPath );
  } else {
    error() << "Unable to create the histogram path" << endmsg;
    return nullptr;
  }
  pObject = createDirectory( subPath, rest );
  return pObject;
}

//------------------------------------------------------------------------------
DataObject* HistogramSvc::createDirectory( const string& parentDir, const string& subDir ) {
  auto directory = std::make_unique<DataObject>();
  if ( directory ) {
    DataObject* pnode;
    StatusCode  status = retrieveObject( parentDir, pnode );
    if ( !status.isSuccess() ) {
      error() << "Unable to create the histogram directory: " << parentDir << "/" << subDir << endmsg;
      return nullptr;
    }
    status = DataSvc::registerObject( pnode, subDir, directory.get() );
    if ( !status.isSuccess() ) {
      error() << "Unable to create the histogram directory: " << parentDir << "/" << subDir << endmsg;
      return nullptr;
    }
  }
  return directory.release();
}

//------------------------------------------------------------------------------
HistogramSvc::~HistogramSvc() {
  setDataLoader( nullptr ).ignore();
  clearStore().ignore();
}

//------------------------------------------------------------------------------
StatusCode HistogramSvc::connectInput( const string& ident ) {
  using Parser       = Gaudi::Utils::AttribStringParser;
  DataObject* pO     = nullptr;
  StatusCode  status = this->findObject( m_rootName.value(), pO );
  if ( status.isSuccess() ) {
    string::size_type loc = ident.find( " " );
    string            filename, auth, svc = "", typ = "";
    string            logname = ident.substr( 0, loc );
    for ( auto attrib : Parser( ident.substr( loc + 1 ) ) ) {
      switch ( ::toupper( attrib.tag[0] ) ) {
      case 'F': // FILE='<file name>'
      case 'D': // DATAFILE='<file name>'
        filename = std::move( attrib.value );
        break;
      case 'T': // TYP='<HBOOK,ROOT,OBJY,...>'
        typ = std::move( attrib.value );
        break;
      default:
        break;
      }
    }
    if ( typ.length() > 0 ) {
      // Now add the registry entry to the store
      string entryname = m_rootName;
      entryname += '/';
      entryname += logname;
      GenericAddress* pA = nullptr;
      switch ( ::toupper( typ[0] ) ) {
      case 'H':
        pA = new GenericAddress( HBOOK_StorageType, CLID_StatisticsFile, filename, entryname, 0, 'O' );
        break;
      case 'R':
        pA = new GenericAddress( ROOT_StorageType, CLID_StatisticsFile, filename, entryname, 0, 'O' );
        break;
      }
      if ( pA ) {
        status = registerAddress( pO, logname, pA );
        if ( status.isSuccess() ) {
          info() << "Added stream file:" << filename << " as " << logname << endmsg;
          return status;
        }
        pA->release();
      }
    }
  }
  error() << "Cannot add " << ident << " invalid filename!" << endmsg;
  return StatusCode::FAILURE;
}

//------------------------------------------------------------------------------
StatusCode HistogramSvc::initialize() {
  StatusCode status = DataSvc::initialize();
  // Set root object
  if ( status.isSuccess() ) {
    auto rootObj = std::make_unique<DataObject>();
    status       = setRoot( "/stat", rootObj.get() );
    if ( status.isFailure() ) {
      error() << "Unable to set hstogram data store root." << endmsg;
      return status;
    }
    rootObj.release();
    auto svc = service<IConversionSvc>( "HistogramPersistencySvc", true );
    if ( !svc ) {
      error() << "Could not find HistogramPersistencySvc." << endmsg;
      return StatusCode::FAILURE;
    }
    setDataLoader( svc.get() ).ignore();
    // Connect all input streams (if any)
    for ( auto& j : m_input ) {
      status = connectInput( j );
      if ( !status.isSuccess() ) return status;
    }
  }
  if ( !m_defs1D.empty() ) {
    info() << " Predefined 1D-Histograms: " << endmsg;
    for ( const auto& ih : m_defs1D ) {
      info() << " Path='" << ih.first << "'"
             << " Description " << ih.second << endmsg;
    }
  }
  return status;
}

//------------------------------------------------------------------------------
StatusCode HistogramSvc::reinitialize() { return StatusCode::SUCCESS; }

//------------------------------------------------------------------------------
IHistogram1D* HistogramSvc::sliceX( const string& name, const IHistogram2D& h, int idxY1, int idxY2 ) {
  std::pair<DataObject*, IHistogram1D*> o( nullptr, nullptr );
  try {
    int firstbin = Gaudi::Axis::toRootIndex( idxY1, h.yAxis().bins() );
    int lastbin  = Gaudi::Axis::toRootIndex( idxY2, h.yAxis().bins() );
    o            = Gaudi::slice1DX( name, h, firstbin, lastbin );
  } catch ( ... ) {
    throw GaudiException( "Cannot cast 2D histogram to H2D to create sliceX `" + name + "'!", "HistogramSvc",
                          StatusCode::FAILURE );
  }
  if ( o.first && registerObject( name, o.second ).isSuccess() ) { return o.second; }
  delete o.first;
  throw GaudiException( "Cannot create sliceX `" + name + "' of 2D histogram!", "HistogramSvc", StatusCode::FAILURE );
}

//------------------------------------------------------------------------------
IHistogram1D* HistogramSvc::sliceY( const string& name, const IHistogram2D& h, int indexX1, int indexX2 ) {
  std::pair<DataObject*, IHistogram1D*> o( nullptr, nullptr );
  try {
    int firstbin = Gaudi::Axis::toRootIndex( indexX1, h.xAxis().bins() );
    int lastbin  = Gaudi::Axis::toRootIndex( indexX2, h.xAxis().bins() );
    o            = Gaudi::slice1DY( name, h, firstbin, lastbin );
  } catch ( ... ) {
    throw GaudiException( "Cannot create sliceY `" + name + "'!", "HistogramSvc", StatusCode::FAILURE );
  }
  // name stands here for the fullPath of the histogram
  if ( o.first && registerObject( name, (IBaseHistogram*)o.second ).isSuccess() ) { return o.second; }
  delete o.first;
  throw GaudiException( "Cannot create sliceY `" + name + "' of 2D histogram!", "HistogramSvc", StatusCode::FAILURE );
}

//------------------------------------------------------------------------------
bool HistogramSvc::destroy( IBaseHistogram* hist ) {
  StatusCode sc = unregisterObject( dynamic_cast<IHistogram*>( hist ) );
  if ( !sc.isSuccess() ) return false;
  delete hist;
  return true;
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( DataObject* pPar, const string& rel, const string& title, int nx, double lowx,
                                        double upx ) {
  if ( m_defs1D.empty() ) {
    return i_book( pPar, rel, title,
                   Gaudi::createH1D( serviceLocator(), buildHistoPath( pPar, rel ), title, nx, lowx, upx ) );
  }
  string hn     = histoAddr( pPar, rel );
  auto   ifound = m_defs1D.find( hn );
  if ( m_defs1D.end() == ifound ) {
    return i_book( pPar, rel, title,
                   Gaudi::createH1D( serviceLocator(), buildHistoPath( pPar, rel ), title, nx, lowx, upx ) );
  }
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << " Redefine the parameters for the histogram '" + hn + "' to be " << ifound->second << endmsg;
  }
  m_mods1D.insert( hn );
  return i_book( pPar, rel, ifound->second.title(),
                 Gaudi::createH1D( serviceLocator(), buildHistoPath( pPar, rel ), ifound->second.title(),
                                   ifound->second.bins(), ifound->second.lowEdge(), ifound->second.lowEdge() ) );
}

// ============================================================================
// constructor
// ============================================================================
HistogramSvc::HistogramSvc( const string& nam, ISvcLocator* svc ) : base_class( nam, svc ) {
  m_rootName = "/stat";
  m_rootCLID = CLID_DataObject;
}

// ============================================================================
void HistogramSvc::update1Ddefs() {
  // check and remove the leading '/stat/'
  removeLeading( m_defs1D.value(), "/stat/" );
}

std::string HistogramSvc::buildHistoPath( DataObject const* pPar, std::string const& rel ) {
  std::string const& path = pPar->registry()->identifier();
  std::string const& root = rootName();
  return path.substr( root.size() ) + '/' + rel;
}

// ============================================================================
// finalize the service
// ============================================================================
StatusCode HistogramSvc::finalize() {
  if ( !m_mods1D.empty() ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << " Substituted histograms #" << m_mods1D.size() << " : " << endmsg;
    for ( const auto& ih : m_mods1D ) {
      if ( msgLevel( MSG::DEBUG ) ) debug() << " Path='" << ih << "'";
      auto im = m_defs1D.find( ih );
      if ( m_defs1D.end() != im ) { debug() << "  " << im->second; }
    }
    m_mods1D.clear();
  }
  return DataSvc::finalize();
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( const string& par, const string& rel, const string& title, int nx, double lowx,
                                        double upx ) {
  return book( createPath( par ), rel, title, nx, lowx, upx );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( const string& par, int hID, const string& title, int nx, double lowx,
                                        double upx ) {
  return book( par, std::to_string( hID ), title, nx, lowx, upx );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( DataObject* pPar, int hID, const string& title, int nx, double lowx,
                                        double upx ) {
  return book( pPar, std::to_string( hID ), title, nx, lowx, upx );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( const std::pair<string, string>& loc, const string& title, int nx, double lowx,
                                        double upx ) {
  return book( loc.first, loc.second, title, nx, lowx, upx );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( const string& full, const string& title, int nx, double lowx, double upx ) {
  return book( i_splitPath( full ), title, nx, lowx, upx );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const string& par, const string& rel, const string& title, int nx,
                                          double lowx, double upx, const string& opt ) {
  return bookProf( createPath( par ), rel, title, nx, lowx, upx, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const string& par, int hID, const string& title, int nx, double lowx,
                                          double upx, const string& opt ) {
  return bookProf( par, std::to_string( hID ), title, nx, lowx, upx, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( DataObject* pPar, int hID, const string& title, int nx, double lowx,
                                          double upx, const string& opt ) {
  return bookProf( pPar, std::to_string( hID ), title, nx, lowx, upx, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::pair<string, string>& loc, const string& title, int nx,
                                          double lowx, double upx, const string& opt ) {
  return bookProf( loc.first, loc.second, title, nx, lowx, upx, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const string& full, const string& title, int nx, double lowx, double upx,
                                          const string& opt ) {
  return bookProf( i_splitPath( full ), title, nx, lowx, upx, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( DataObject* pPar, const string& rel, const string& title, int nx, double lowx,
                                          double upx, const string& opt ) {
  return i_book(
      pPar, rel, title,
      Gaudi::createProf1D( serviceLocator(), buildHistoPath( pPar, rel ), title, nx, lowx, upx, 0, 0, opt ) );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const string& par, const string& rel, const string& title, int nx,
                                          double lowx, double upx, double upper, double lower, const string& opt ) {
  return bookProf( createPath( par ), rel, title, nx, lowx, upx, upper, lower, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const string& par, int hID, const string& title, int nx, double lowx,
                                          double upx, double upper, double lower, const string& opt ) {
  return bookProf( par, std::to_string( hID ), title, nx, lowx, upx, upper, lower, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( DataObject* pPar, int hID, const string& title, int nx, double lowx,
                                          double upx, double upper, double lower, const string& opt ) {
  return bookProf( pPar, std::to_string( hID ), title, nx, lowx, upx, upper, lower, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::pair<string, string>& loc, const string& title, int nx,
                                          double lowx, double upx, double upper, double lower, const string& opt ) {
  return bookProf( loc.first, loc.second, title, nx, lowx, upx, upper, lower, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const string& full, const string& title, int nx, double lowx, double upx,
                                          double upper, double lower, const string& opt ) {
  return bookProf( i_splitPath( full ), title, nx, lowx, upx, upper, lower, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( DataObject* pPar, const string& rel, const string& title, int nx, double lowx,
                                          double upx, double upper, double lower, const string& opt ) {
  return i_book(
      pPar, rel, title,
      Gaudi::createProf1D( serviceLocator(), buildHistoPath( pPar, rel ), title, nx, lowx, upx, upper, lower, opt ) );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( const std::string& par, int hID, const std::string& title, Edges e ) {
  return book( par, std::to_string( hID ), title, e );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( DataObject* pPar, int hID, const std::string& title, Edges e ) {
  return book( pPar, std::to_string( hID ), title, e );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( const std::string& par, const std::string& rel, const std::string& title,
                                        Edges e ) {
  return book( createPath( par ), rel, title, e );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( const std::pair<std::string, std::string>& loc, const std::string& title,
                                        Edges e ) {
  return book( loc.first, loc.second, title, e );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( const std::string& full, const std::string& title, Edges e ) {
  return book( i_splitPath( full ), title, e );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::book( DataObject* pPar, const std::string& rel, const std::string& title, Edges e ) {
  return i_book( pPar, rel, title, Gaudi::createH1D( serviceLocator(), buildHistoPath( pPar, rel ), title, e ) );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::string& full, const std::string& title, Edges e ) {
  return bookProf( i_splitPath( full ), title, e );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::string& par, const std::string& rel, const std::string& title,
                                          Edges e ) {
  return bookProf( createPath( par ), rel, title, e );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::string& par, int hID, const std::string& title, Edges e ) {
  return bookProf( par, std::to_string( hID ), title, e );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( DataObject* pPar, int hID, const std::string& title, Edges e ) {
  return bookProf( pPar, std::to_string( hID ), title, e );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::pair<std::string, std::string>& loc, const std::string& title,
                                          Edges e ) {
  return bookProf( loc.first, loc.second, title, e );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( DataObject* pPar, const std::string& rel, const std::string& title,
                                          Edges e ) {
  return i_book( pPar, rel, title,
                 Gaudi::createProf1D( serviceLocator(), buildHistoPath( pPar, rel ), title, e, 0, 0 ) );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::string& full, const std::string& title, Edges e, double upper,
                                          double lower ) {
  return bookProf( i_splitPath( full ), title, e, upper, lower );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::string& par, const std::string& rel, const std::string& title,
                                          Edges e, double upper, double lower ) {
  return bookProf( createPath( par ), rel, title, e, upper, lower );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::string& par, int hID, const std::string& title, Edges e,
                                          double upper, double lower ) {
  return bookProf( par, std::to_string( hID ), title, e, upper, lower );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( DataObject* pPar, int hID, const std::string& title, Edges e, double upper,
                                          double lower ) {
  return bookProf( pPar, std::to_string( hID ), title, e, upper, lower );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( const std::pair<std::string, std::string>& loc, const std::string& title,
                                          Edges e, double upper, double lower ) {
  return bookProf( loc.first, loc.second, title, e, upper, lower );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::bookProf( DataObject* pPar, const std::string& rel, const std::string& title, Edges e,
                                          double upper, double lower ) {
  return i_book( pPar, rel, title,
                 Gaudi::createProf1D( serviceLocator(), buildHistoPath( pPar, rel ), title, e, upper, lower ) );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( const std::string& full, const std::string& title, int nx, double lowx,
                                        double upx, int ny, double lowy, double upy ) {
  return book( i_splitPath( full ), title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( const std::string& par, const std::string& rel, const std::string& title,
                                        int nx, double lowx, double upx, int ny, double lowy, double upy ) {
  return book( createPath( par ), rel, title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( const std::string& par, int hID, const std::string& title, int nx, double lowx,
                                        double upx, int ny, double lowy, double upy ) {
  return book( par, std::to_string( hID ), title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( const std::pair<std::string, std::string>& loc, const std::string& title,
                                        int nx, double lowx, double upx, int ny, double lowy, double upy ) {
  return book( loc.first, loc.second, title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( DataObject* pPar, int hID, const std::string& title, int nx, double lowx,
                                        double upx, int ny, double lowy, double upy ) {
  return book( pPar, std::to_string( hID ), title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( DataObject* pPar, const std::string& rel, const std::string& title, int nx,
                                        double lowx, double upx, int ny, double lowy, double upy ) {
  return i_book(
      pPar, rel, title,
      Gaudi::createH2D( serviceLocator(), buildHistoPath( pPar, rel ), title, nx, lowx, upx, ny, lowy, upy ) );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& full, const std::string& title, int nx, double lowx,
                                          double upx, int ny, double lowy, double upy, double upper, double lower ) {
  return bookProf( i_splitPath( full ), title, nx, lowx, upx, ny, lowy, upy, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& par, const std::string& rel, const std::string& title,
                                          int nx, double lowx, double upx, int ny, double lowy, double upy,
                                          double upper, double lower ) {
  return bookProf( createPath( par ), rel, title, nx, lowx, upx, ny, lowy, upy, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::pair<std::string, std::string>& loc, const std::string& title,
                                          int nx, double lowx, double upx, int ny, double lowy, double upy,
                                          double upper, double lower ) {
  return bookProf( loc.first, loc.second, title, nx, lowx, upx, ny, lowy, upy, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& par, int hID, const std::string& title, int nx,
                                          double lowx, double upx, int ny, double lowy, double upy, double upper,
                                          double lower ) {
  return bookProf( par, std::to_string( hID ), title, nx, lowx, upx, ny, lowy, upy, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( DataObject* pPar, int hID, const std::string& title, int nx, double lowx,
                                          double upx, int ny, double lowy, double upy, double upper, double lower ) {
  return bookProf( pPar, std::to_string( hID ), title, nx, lowx, upx, ny, lowy, upy, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( DataObject* pPar, const std::string& rel, const std::string& title, int nx,
                                          double lowx, double upx, int ny, double lowy, double upy, double upper,
                                          double lower ) {
  return i_book( pPar, rel, title,
                 Gaudi::createProf2D( serviceLocator(), buildHistoPath( pPar, rel ), title, nx, lowx, upx, ny, lowy,
                                      upy, upper, lower ) );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& full, const std::string& title, int nx, double lowx,
                                          double upx, int ny, double lowy, double upy ) {
  return bookProf( i_splitPath( full ), title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& par, const std::string& rel, const std::string& title,
                                          int nx, double lowx, double upx, int ny, double lowy, double upy ) {
  return bookProf( createPath( par ), rel, title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::pair<std::string, std::string>& loc, const std::string& title,
                                          int nx, double lowx, double upx, int ny, double lowy, double upy ) {
  return bookProf( loc.first, loc.second, title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& par, int hID, const std::string& title, int nx,
                                          double lowx, double upx, int ny, double lowy, double upy ) {
  return bookProf( par, std::to_string( hID ), title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( DataObject* pPar, int hID, const std::string& title, int nx, double lowx,
                                          double upx, int ny, double lowy, double upy ) {
  return bookProf( pPar, std::to_string( hID ), title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( DataObject* pPar, const std::string& rel, const std::string& title, int nx,
                                          double lowx, double upx, int ny, double lowy, double upy ) {
  return i_book(
      pPar, rel, title,
      Gaudi::createProf2D( serviceLocator(), buildHistoPath( pPar, rel ), title, nx, lowx, upx, ny, lowy, upy, 0, 0 ) );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( const std::string& full, const std::string& title, Edges x, Edges y ) {
  return book( i_splitPath( full ), title, x, y );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( const std::string& par, const std::string& rel, const std::string& title,
                                        Edges x, Edges y ) {
  return book( createPath( par ), rel, title, x, y );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( const std::string& par, int hID, const std::string& title, Edges x, Edges y ) {
  return book( par, std::to_string( hID ), title, x, y );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( const std::pair<std::string, std::string>& loc, const std::string& title,
                                        Edges x, Edges y ) {
  return book( loc.first, loc.second, title, x, y );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( DataObject* pPar, int hID, const std::string& title, Edges x, Edges y ) {
  return book( pPar, std::to_string( hID ), title, x, y );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::book( DataObject* pPar, const std::string& rel, const std::string& title, Edges x,
                                        Edges y ) {
  return i_book( pPar, rel, title, Gaudi::createH2D( serviceLocator(), buildHistoPath( pPar, rel ), title, x, y ) );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& full, const std::string& title, Edges x, Edges y ) {
  return bookProf( i_splitPath( full ), title, x, y );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& par, const std::string& rel, const std::string& title,
                                          Edges x, Edges y ) {
  return bookProf( createPath( par ), rel, title, x, y );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& par, int hID, const std::string& title, Edges x,
                                          Edges y ) {
  return bookProf( par, std::to_string( hID ), title, x, y );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( DataObject* pPar, int hID, const std::string& title, Edges x, Edges y ) {
  return bookProf( pPar, std::to_string( hID ), title, x, y );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::pair<std::string, std::string>& loc, const std::string& title,
                                          Edges x, Edges y ) {
  return bookProf( loc.first, loc.second, title, x, y );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( DataObject* pPar, const std::string& rel, const std::string& title, Edges x,
                                          Edges y ) {
  return i_book( pPar, rel, title,
                 Gaudi::createProf2D( serviceLocator(), buildHistoPath( pPar, rel ), title, x, y, 0, 0 ) );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& full, const std::string& title, Edges x, Edges y,
                                          double upper, double lower ) {
  return bookProf( i_splitPath( full ), title, x, y, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& par, const std::string& rel, const std::string& title,
                                          Edges x, Edges y, double upper, double lower ) {
  return bookProf( createPath( par ), rel, title, x, y, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::string& par, int hID, const std::string& title, Edges x, Edges y,
                                          double upper, double lower ) {
  return bookProf( par, std::to_string( hID ), title, x, y, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( DataObject* pPar, int hID, const std::string& title, Edges x, Edges y,
                                          double upper, double lower ) {
  return bookProf( pPar, std::to_string( hID ), title, x, y, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( const std::pair<std::string, std::string>& loc, const std::string& title,
                                          Edges x, Edges y, double upper, double lower ) {
  return bookProf( loc.first, loc.second, title, x, y, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::bookProf( DataObject* pPar, const std::string& rel, const std::string& title, Edges x,
                                          Edges y, double upper, double lower ) {
  return i_book( pPar, rel, title,
                 Gaudi::createProf2D( serviceLocator(), buildHistoPath( pPar, rel ), title, x, y, upper, lower ) );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( const std::string& full, const std::string& title, int nx, double lowx,
                                        double upx, int ny, double lowy, double upy, int nz, double lowz, double upz ) {
  return book( i_splitPath( full ), title, nx, lowx, upx, ny, lowy, upy, nz, lowz, upz );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( const std::string& par, const std::string& rel, const std::string& title,
                                        int nx, double lowx, double upx, int ny, double lowy, double upy, int nz,
                                        double lowz, double upz ) {
  return book( createPath( par ), rel, title, nx, lowx, upx, ny, lowy, upy, nz, lowz, upz );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( const std::string& par, int hID, const std::string& title, int nx, double lowx,
                                        double upx, int ny, double lowy, double upy, int nz, double lowz, double upz ) {
  return book( par, std::to_string( hID ), title, nx, lowx, upx, ny, lowy, upy, nz, lowz, upz );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( DataObject* pPar, int hID, const std::string& title, int nx, double lowx,
                                        double upx, int ny, double lowy, double upy, int nz, double lowz, double upz ) {
  return book( pPar, std::to_string( hID ), title, nx, lowx, upx, ny, lowy, upy, nz, lowz, upz );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( const std::pair<std::string, std::string>& loc, const std::string& title,
                                        int nx, double lowx, double upx, int ny, double lowy, double upy, int nz,
                                        double lowz, double upz ) {
  return book( loc.first, loc.second, title, nx, lowx, upx, ny, lowy, upy, nz, lowz, upz );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( DataObject* pPar, const std::string& rel, const std::string& title, int nx,
                                        double lowx, double upx, int ny, double lowy, double upy, int nz, double lowz,
                                        double upz ) {
  return i_book( pPar, rel, title,
                 Gaudi::createH3D( serviceLocator(), buildHistoPath( pPar, rel ), title, nx, lowx, upx, ny, lowy, upy,
                                   nz, lowz, upz ) );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( const std::string& full, const std::string& title, Edges x, Edges y, Edges z ) {
  return book( i_splitPath( full ), title, x, y, z );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( const std::string& par, const std::string& rel, const std::string& title,
                                        Edges x, Edges y, Edges z ) {
  return book( createPath( par ), rel, title, x, y, z );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( const std::string& par, int hID, const std::string& title, Edges x, Edges y,
                                        Edges z ) {
  return book( par, std::to_string( hID ), title, x, y, z );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( DataObject* pPar, int hID, const std::string& title, Edges x, Edges y,
                                        Edges z ) {
  return book( pPar, std::to_string( hID ), title, x, y, z );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( const std::pair<std::string, std::string>& loc, const std::string& title,
                                        Edges x, Edges y, Edges z ) {
  return book( loc.first, loc.second, title, x, y, z );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::book( DataObject* pPar, const std::string& rel, const std::string& title, Edges x,
                                        Edges y, Edges z ) {
  return i_book( pPar, rel, title, Gaudi::createH3D( serviceLocator(), buildHistoPath( pPar, rel ), title, x, y, z ) );
}

// ============================================================================
StatusCode HistogramSvc::registerObject( const std::string& parent, const std::string& rel, Base* obj ) {
  return registerObject( createPath( parent ), rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::registerObject( Base* pPar, const std::string& rel, Base* obj ) {
  return registerObject( ::detail::cast( pPar ), rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::unregisterObject( Base* obj ) { return unregisterObject( ::detail::cast( obj ) ); }

// ============================================================================
StatusCode HistogramSvc::unregisterObject( Base* obj, const std::string& objectPath ) {
  return unregisterObject( ::detail::cast( obj ), objectPath );
}

// ============================================================================
StatusCode HistogramSvc::unregisterObject( Base* obj, int item ) {
  return unregisterObject( ::detail::cast( obj ), item );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( IRegistry* pReg, const std::string& path, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).retrieve( pReg, path, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( IRegistry* pReg, const std::string& path, AIDA::IProfile1D*& obj ) {
  return Helper( this ).retrieve( pReg, path, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( IRegistry* pReg, const std::string& path, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).retrieve( pReg, path, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( IRegistry* pReg, const std::string& path, AIDA::IProfile2D*& obj ) {
  return Helper( this ).retrieve( pReg, path, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( IRegistry* pReg, const std::string& path, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).retrieve( pReg, path, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& full, AIDA::IProfile1D*& obj ) {
  return Helper( this ).retrieve( full, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& full, AIDA::IProfile2D*& obj ) {
  return Helper( this ).retrieve( full, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& full, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).retrieve( full, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& full, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).retrieve( full, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& full, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).retrieve( full, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, const std::string& rel,

                                         AIDA::IProfile1D*& obj ) {
  return Helper( this ).retrieve( parent, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, const std::string& rel, AIDA::IProfile2D*& obj ) {
  return Helper( this ).retrieve( parent, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, const std::string& rel, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).retrieve( parent, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, const std::string& rel, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).retrieve( parent, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, const std::string& rel, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).retrieve( parent, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, int item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).retrieve( parent, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, int item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).retrieve( parent, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, int item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).retrieve( parent, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, int item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).retrieve( parent, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( const std::string& parent, int item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).retrieve( parent, item, obj );
}
// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, const std::string& item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, const std::string& item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, const std::string& item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, const std::string& item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, const std::string& item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, int item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, int item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, int item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, int item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( DataObject* par, int item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).retrieve( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, int item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, int item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, int item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, int item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, int item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, const std::string& item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, const std::string& item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, const std::string& item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, const std::string& item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::retrieveObject( Base* par, const std::string& item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).retrieve( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( IRegistry* pReg, const std::string& path, AIDA::IProfile1D*& obj ) {
  return Helper( this ).find( pReg, path, obj );
}
// ============================================================================
StatusCode HistogramSvc::findObject( IRegistry* pReg, const std::string& path, AIDA::IProfile2D*& obj ) {
  return Helper( this ).find( pReg, path, obj );
}
// ============================================================================
StatusCode HistogramSvc::findObject( IRegistry* pReg, const std::string& path, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).find( pReg, path, obj );
}
// ============================================================================
StatusCode HistogramSvc::findObject( IRegistry* pReg, const std::string& path, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).find( pReg, path, obj );
}
// ============================================================================
StatusCode HistogramSvc::findObject( IRegistry* pReg, const std::string& path, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).find( pReg, path, obj );
}
// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& full, AIDA::IProfile1D*& obj ) {
  return Helper( this ).find( full, obj );
}
// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& full, AIDA::IProfile2D*& obj ) {
  return Helper( this ).find( full, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& full, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).find( full, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& full, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).find( full, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& full, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).find( full, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, const std::string& rel, AIDA::IProfile1D*& obj ) {
  return Helper( this ).find( par, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, const std::string& rel, AIDA::IProfile2D*& obj ) {
  return Helper( this ).find( par, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, const std::string& rel, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).find( par, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, const std::string& rel, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).find( par, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, const std::string& rel, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).find( par, rel, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, int item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, int item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, int item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, int item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( const std::string& par, int item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, int item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, int item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, int item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, int item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, int item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, const std::string& item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, const std::string& item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, const std::string& item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, const std::string& item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( DataObject* par, const std::string& item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).find( par, item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, int item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, int item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, int item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, int item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, int item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, const std::string& item, AIDA::IProfile1D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, const std::string& item, AIDA::IProfile2D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, const std::string& item, AIDA::IHistogram1D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, const std::string& item, AIDA::IHistogram2D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
StatusCode HistogramSvc::findObject( Base* par, const std::string& item, AIDA::IHistogram3D*& obj ) {
  return Helper( this ).find( ::detail::cast( par ), item, obj );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::projectionX( const std::string& name, const AIDA::IHistogram2D& h ) {
  return sliceX( name, h, IAxis::UNDERFLOW_BIN, IAxis::OVERFLOW_BIN );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::projectionY( const std::string& name, const AIDA::IHistogram2D& h ) {
  return sliceY( name, h, IAxis::UNDERFLOW_BIN, IAxis::OVERFLOW_BIN );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::sliceX( const std::string& name, const AIDA::IHistogram2D& h, int indexY ) {
  return sliceX( name, h, indexY, indexY );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::sliceY( const std::string& name, const AIDA::IHistogram2D& h, int indexX ) {
  return sliceY( name, h, indexX, indexX );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::add( const std::string& nameAndTitle, const AIDA::IHistogram1D& a,
                                       const AIDA::IHistogram1D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH1::Add, 1. );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::subtract( const std::string& nameAndTitle, const AIDA::IHistogram1D& a,
                                            const AIDA::IHistogram1D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH1::Add, -1. );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::multiply( const std::string& nameAndTitle, const AIDA::IHistogram1D& a,
                                            const AIDA::IHistogram1D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH1::Multiply );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::divide( const std::string& nameAndTitle, const AIDA::IHistogram1D& a,
                                          const AIDA::IHistogram1D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH1::Divide );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::add( const std::string& nameAndTitle, const AIDA::IHistogram2D& a,
                                       const AIDA::IHistogram2D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH2D::Add, 1. );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::subtract( const std::string& nameAndTitle, const AIDA::IHistogram2D& a,
                                            const AIDA::IHistogram2D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH2D::Add, -1. );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::multiply( const std::string& nameAndTitle, const AIDA::IHistogram2D& a,
                                            const AIDA::IHistogram2D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH2D::Multiply );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::divide( const std::string& nameAndTitle, const AIDA::IHistogram2D& a,
                                          const AIDA::IHistogram2D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH2D::Divide );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::add( const std::string& nameAndTitle, const AIDA::IHistogram3D& a,
                                       const AIDA::IHistogram3D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH3D::Add, 1. );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::subtract( const std::string& nameAndTitle, const AIDA::IHistogram3D& a,
                                            const AIDA::IHistogram3D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH3D::Add, -1. );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::multiply( const std::string& nameAndTitle, const AIDA::IHistogram3D& a,
                                            const AIDA::IHistogram3D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH3D::Multiply );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::divide( const std::string& nameAndTitle, const AIDA::IHistogram3D& a,
                                          const AIDA::IHistogram3D& b ) {
  return Helper::act( createCopy( nameAndTitle, a ), b, &TH3D::Divide );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::projectionXY( const std::string& nameAndTitle, const AIDA::IHistogram3D& h ) {
  return i_project( nameAndTitle, h, "xy" );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::projectionXZ( const std::string& nameAndTitle, const AIDA::IHistogram3D& h ) {
  return i_project( nameAndTitle, h, "xz" );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::projectionYZ( const std::string& nameAndTitle, const AIDA::IHistogram3D& h ) {
  return i_project( nameAndTitle, h, "yz" );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::createHistogram1D( const std::string& name, const std::string& title, int nx,
                                                     double lowx, double upx ) {
  return book( name, title, nx, lowx, upx );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::createHistogram1D( const std::string& name, const std::string& title, int nx,
                                                     double lowx, double upx, const std::string& /*opt*/ ) {
  return book( name, title, nx, lowx, upx );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::createHistogram1D( const std::string& name, const std::string& title, const Edges& x,
                                                     const std::string& /*opt*/ ) {
  return book( name, title, x );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::createHistogram1D( const std::string& nameAndTitle, int nx, double lowx,
                                                     double upx ) {
  return book( nameAndTitle, nameAndTitle, nx, lowx, upx );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::createCopy( const std::string& full, const AIDA::IHistogram1D& h ) {
  return createCopy( i_splitPath( full ), h );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::createCopy( const std::string& par, const std::string& rel,
                                              const AIDA::IHistogram1D& h ) {
  return createCopy( createPath( par ), rel, h );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::createCopy( const std::pair<std::string, std::string>& loc,
                                              const AIDA::IHistogram1D&                  h ) {
  return createCopy( loc.first, loc.second, h );
}

// ============================================================================
AIDA::IHistogram1D* HistogramSvc::createCopy( DataObject* pPar, const std::string& rel, const AIDA::IHistogram1D& h ) {
  return i_book( pPar, rel, h.title(), Gaudi::createH1D( serviceLocator(), buildHistoPath( pPar, rel ), h ) );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::createHistogram2D( const std::string& name, const std::string& title, int nx,
                                                     double lowx, double upx, int ny, double lowy, double upy ) {
  return book( name, title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::createHistogram2D( const std::string& name, const std::string& title, int nx,
                                                     double lowx, double upx, int ny, double lowy, double upy,
                                                     const std::string& /*opt*/ ) {
  return book( name, title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::createHistogram2D( const std::string& name, const std::string& title, const Edges& x,
                                                     const Edges& y, const std::string& /*opt*/ ) {
  return book( name, title, x, y );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::createHistogram2D( const std::string& nameAndTitle, int nx, double lowx, double upx,
                                                     int ny, double lowy, double upy ) {
  return book( nameAndTitle, nameAndTitle, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::createCopy( const std::string& full, const AIDA::IHistogram2D& h ) {
  return createCopy( i_splitPath( full ), h );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::createCopy( const std::string& par, const std::string& rel,
                                              const AIDA::IHistogram2D& h ) {
  return createCopy( createPath( par ), rel, h );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::createCopy( const std::pair<std::string, std::string>& loc,
                                              const AIDA::IHistogram2D&                  h ) {
  return createCopy( loc.first, loc.second, h );
}

// ============================================================================
AIDA::IHistogram2D* HistogramSvc::createCopy( DataObject* pPar, const std::string& rel, const AIDA::IHistogram2D& h ) {
  return i_book( pPar, rel, h.title(), Gaudi::createH2D( serviceLocator(), buildHistoPath( pPar, rel ), h ) );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::createHistogram3D( const std::string& name, const std::string& title, int nx,
                                                     double lowx, double upx, int ny, double lowy, double upy, int nz,
                                                     double lowz, double upz ) {
  return book( name, title, nx, lowx, upx, ny, lowy, upy, nz, lowz, upz );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::createHistogram3D( const std::string& name, const std::string& title, int nx,
                                                     double lowx, double upx, int ny, double lowy, double upy, int nz,
                                                     double lowz, double upz, const std::string& /*opt*/ ) {
  return book( name, title, nx, lowx, upx, ny, lowy, upy, nz, lowz, upz );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::createHistogram3D( const std::string& name, const std::string& title, const Edges& x,
                                                     const Edges& y, const Edges& z, const std::string& /*opt*/ ) {
  return book( name, title, x, y, z );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::createHistogram3D( const std::string& nameAndTitle, int nx, double lowx, double upx,
                                                     int ny, double lowy, double upy, int nz, double lowz,
                                                     double upz ) {
  return book( nameAndTitle, nameAndTitle, nx, lowx, upx, ny, lowy, upy, nz, lowz, upz );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::createCopy( const std::string& full, const AIDA::IHistogram3D& h ) {
  return createCopy( i_splitPath( full ), h );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::createCopy( const std::string& par, const std::string& rel,
                                              const AIDA::IHistogram3D& h ) {
  return createCopy( createPath( par ), rel, h );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::createCopy( const std::pair<std::string, std::string>& loc,
                                              const AIDA::IHistogram3D&                  h ) {
  return createCopy( loc.first, loc.second, h );
}

// ============================================================================
AIDA::IHistogram3D* HistogramSvc::createCopy( DataObject* pPar, const std::string& rel, const AIDA::IHistogram3D& h ) {
  return i_book( pPar, rel, h.title(), Gaudi::createH3D( serviceLocator(), buildHistoPath( pPar, rel ), h ) );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createProfile1D( const std::string& name, const std::string& title, int nx, double lowx,
                                                 double upx, const std::string& opt ) {
  return bookProf( name, title, nx, lowx, upx, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createProfile1D( const std::string& name, const std::string& title, int nx, double lowx,
                                                 double upx, double upper, double lower, const std::string& opt ) {
  return bookProf( name, title, nx, lowx, upx, upper, lower, opt );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createProfile1D( const std::string& name, const std::string& title, const Edges& x,
                                                 const std::string& /* opt */ ) {
  return bookProf( name, title, x );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createProfile1D( const std::string& name, const std::string& title, const Edges& x,
                                                 double upper, double lower, const std::string& /* opt */ ) {
  return bookProf( name, title, x, upper, lower );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createProfile1D( const std::string& nametit, int nx, double lowx, double upx ) {
  return bookProf( nametit, nametit, nx, lowx, upx, "s" );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createProfile1D( const std::string& nametit, int nx, double lowx, double upx,
                                                 double upper, double lower ) {
  return bookProf( nametit, nametit, nx, lowx, upx, upper, lower, "s" );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createCopy( const std::string& full, const AIDA::IProfile1D& h ) {
  return createCopy( i_splitPath( full ), h );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createCopy( const std::string& par, const std::string& rel,
                                            const AIDA::IProfile1D& h ) {
  return createCopy( createPath( par ), rel, h );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createCopy( const std::pair<std::string, std::string>& loc,
                                            const AIDA::IProfile1D&                    h ) {
  return createCopy( loc.first, loc.second, h );
}

// ============================================================================
AIDA::IProfile1D* HistogramSvc::createCopy( DataObject* pPar, const std::string& rel, const AIDA::IProfile1D& h ) {
  return i_book( pPar, rel, h.title(), Gaudi::createProf1D( serviceLocator(), buildHistoPath( pPar, rel ), h ) );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createProfile2D( const std::string& name, const std::string& title, int nx, double lowx,
                                                 double upx, int ny, double lowy, double upy ) {
  return bookProf( name, title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createProfile2D( const std::string& name, const std::string& title, int nx, double lowx,
                                                 double upx, int ny, double lowy, double upy,
                                                 const std::string& /*opt*/ ) {
  return bookProf( name, title, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createProfile2D( const std::string& name, const std::string& title, const Edges& x,
                                                 const Edges& y, const std::string& /*opt*/ ) {
  return bookProf( name, title, x, y );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createProfile2D( const std::string& nameAndTitle, int nx, double lowx, double upx,
                                                 int ny, double lowy, double upy ) {
  return bookProf( nameAndTitle, nameAndTitle, nx, lowx, upx, ny, lowy, upy );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createProfile2D( const std::string& name, const std::string& title, int nx, double lowx,
                                                 double upx, int ny, double lowy, double upy, double upper,
                                                 double lower ) {
  return bookProf( name, title, nx, lowx, upx, ny, lowy, upy, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createProfile2D( const std::string& name, const std::string& title, int nx, double lowx,
                                                 double upx, int ny, double lowy, double upy, double upper,
                                                 double lower, const std::string& /*opt*/ ) {
  return bookProf( name, title, nx, lowx, upx, ny, lowy, upy, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createProfile2D( const std::string& name, const std::string& title, const Edges& x,
                                                 const Edges& y, double upper, double lower,
                                                 const std::string& /*opt*/ ) {
  return bookProf( name, title, x, y, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createProfile2D( const std::string& nameAndTitle, int nx, double lowx, double upx,
                                                 int ny, double lowy, double upy, double upper, double lower ) {
  return bookProf( nameAndTitle, nameAndTitle, nx, lowx, upx, ny, lowy, upy, upper, lower );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createCopy( const std::string& full, const AIDA::IProfile2D& h ) {
  return createCopy( i_splitPath( full ), h );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createCopy( const std::string& par, const std::string& rel,
                                            const AIDA::IProfile2D& h ) {
  return createCopy( createPath( par ), rel, h );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createCopy( const std::pair<std::string, std::string>& loc,
                                            const AIDA::IProfile2D&                    h ) {
  return createCopy( loc.first, loc.second, h );
}

// ============================================================================
AIDA::IProfile2D* HistogramSvc::createCopy( DataObject* pPar, const std::string& rel, const AIDA::IProfile2D& h ) {
  return i_book( pPar, rel, h.title(), Gaudi::createProf2D( serviceLocator(), buildHistoPath( pPar, rel ), h ) );
}
