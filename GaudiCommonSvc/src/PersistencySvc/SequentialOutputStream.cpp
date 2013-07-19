// boost
#include <boost/numeric/conversion/bounds.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

// Framework include files
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "SequentialOutputStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT( SequentialOutputStream )

using namespace std;
namespace bf = boost::filesystem;
using boost::lexical_cast;
using boost::bad_lexical_cast;

//=============================================================================
SequentialOutputStream::SequentialOutputStream( const string& name,
						ISvcLocator* svc )
: OutputStream( name, svc ), m_events( 0 ), m_iFile( 1 )
{
   declareProperty( "EventsPerFile", m_eventsPerFile
		    = boost::numeric::bounds< unsigned int>::highest() );
   declareProperty( "NumericFilename", m_numericFilename = false );
   declareProperty( "NumbersAdded", m_nNumbersAdded = 6 );
}

//=============================================================================
StatusCode SequentialOutputStream::writeObjects()
{
   try {
      makeFilename();
   } catch ( const GaudiException& except ) {
      MsgStream log(msgSvc(), name());
      log << MSG::ERROR << except.message() << endmsg;
      return StatusCode::FAILURE;
   }
   return OutputStream::writeObjects();
}

//=============================================================================
StatusCode SequentialOutputStream::execute()
{
   // Clear any previously existing item list
   clearSelection();
   // Test whether this event should be output
   if ( isEventAccepted() )  {
      StatusCode sc = writeObjects();
      clearSelection();
      m_events++;
      return sc;
   }
   return StatusCode::SUCCESS;
}

//=============================================================================
void SequentialOutputStream::makeFilename()
{
   if ( m_events % m_eventsPerFile != 0 ) return;

   bf::path outputPath( m_outputName );
   string filename = outputPath.filename().string();
   bf::path dir = outputPath.parent_path();
   string stem = outputPath.stem().string();
   string extension = outputPath.extension().string();

   if ( !dir.empty() ) {
      if ( !bf::exists( dir ) ) {
         stringstream stream;
         stream << "Directory " << dir << " does not exist.";
         throw GaudiException( stream.str(), "error", StatusCode::FAILURE );
      }
   }

   if ( m_numericFilename ) {
      if ( m_events == 0 ) {
         try {
            m_iFile = lexical_cast< unsigned int >( stem );
         } catch( const bad_lexical_cast& /* cast */ ) {
            stringstream stream;
            stream << "Filename " << filename
                   << " is not a number, which was needed.";
            throw GaudiException( stream.str(), "error", StatusCode::FAILURE );
         }
      }
      stringstream iFileStream;
      iFileStream << m_iFile;
      string iFile( iFileStream.str() );
      unsigned int length = 0;

      if ( stem.length() > iFile.length() ) {
         length = stem.length() - iFile.length();
      }

      stringstream name;
      if ( !dir.empty() ) {
         name << dir << "/";
      }
      for ( unsigned int i = 0; i < length; ++i ) {
         name << "0";
      }
      name << iFile << extension;
      m_outputName = name.str();
   } else {
      if ( m_iFile != 1 ) {
         size_t pos = stem.rfind( "_" );
         stem = stem.substr( 0, pos );
      }

      stringstream iFileStream;
      iFileStream << m_iFile;
      string iFile( iFileStream.str() );

      unsigned int length = 0;
      if ( m_nNumbersAdded > iFile.length() ) {
         length = m_nNumbersAdded - iFile.length();
      }

      stringstream name;
      name << dir << "/" << stem;
      for ( unsigned int i = 0; i < length; ++i ) {
         if ( i == 0 ) name << "_";
         name << "0";
      }
      name << iFile << extension;
      m_outputName = name.str();
   }
   ++m_iFile;
}
