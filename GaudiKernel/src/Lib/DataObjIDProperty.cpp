
// stl includes
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <map>
#include <sstream>

// StoreGate includes
#include "GaudiKernel/DataObjIDProperty.h"

using namespace Gaudi;
using namespace Gaudi::Parsers;
using namespace Gaudi::Utils;

namespace Gaudi
{
  namespace Parsers
  {

    StatusCode parse( DataObjID& v, const std::string& s )
    {
      // default values
      StatusCode sc( StatusCode::FAILURE );
      std::string prop;
      sc = Gaudi::Parsers::parse( prop, s );

      if ( sc.isSuccess() ) {
        // split the string in 1 or 2 words:
        typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
        boost::char_separator<char> sep( "(), " );
        tokenizer tokens( prop, sep );
        int nToks( distance( tokens.begin(), tokens.end() ) );
        auto it = tokens.begin();

        if ( nToks == 1 ) {
          // Gaudi style /path/to/object
          std::string k = *it;
          boost::erase_all( k, "'" );
          v = DataObjID( k );

        } else if ( nToks == 2 ) {
          // ATLAS style (clid, 'key') or ('ClassName', 'key')
          CLID c( 0 );
          std::string cn( *it );
          DataObjID id;

          try {
            c = std::stoi( *it );
          } catch ( const std::invalid_argument& /*e*/ ) {
            // not a number
            boost::erase_all( cn, "'" );
            c = 0;
          }

          ++it;
          std::string k = *it;
          boost::erase_all( k, "'" );
          ++it;

          if ( c != 0 ) {
            v = DataObjID( c, k );
          } else {
            v = DataObjID( cn, k );
          }
        } else {
          std::cerr << "Unable to instantiate a DataObjID from a Property " << s << " :: Format is bad" << std::endl;
          sc = StatusCode::FAILURE;
          return sc;
        }
      }
      return sc;
    }

    //---------------------------------------------------------------------------

    StatusCode parse( DataObjIDColl& v, const std::string& s )
    {
      // default values
      StatusCode sc( StatusCode::FAILURE );
      std::string prop;
      sc = Gaudi::Parsers::parse( prop, s );

      if ( sc.isSuccess() ) {

        // Reset Collection
        v.clear();

        bool isGaudi( true );

        // Gaudi style [ '/path/to/data', '/other/data' ] or
        // ATLAS style [ ('ClassName', 'key') , (ClassID, 'key2') ]
        if ( s.find( "(" ) != std::string::npos ) {
          isGaudi = false;
        }

        // split the string in 1 or 2 words:
        typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
        boost::char_separator<char> sep( "[](), " );
        tokenizer tokens( prop, sep );
        auto it = tokens.begin();

        CLID c( 0 );
        std::string cn;

        while ( it != tokens.end() ) {

          if ( isGaudi ) {

            std::string k = *it;
            boost::erase_all( k, "'" );
            v.emplace( DataObjID( k ) );
            ++it;

          } else {

            try {
              c = std::stoi( *it );
            } catch ( const std::invalid_argument& /*e*/ ) {
              // class name, not ClassID
              cn = *it;
              boost::erase_all( cn, "'" );
              c = 0;
            }

            ++it;
            std::string k = *it;
            boost::erase_all( k, "'" );
            ++it;

            if ( c != 0 ) {
              v.emplace( DataObjID( c, k ) );
            } else {
              v.emplace( DataObjID( cn, k ) );
            }
          }
        }
      }
      return sc;
    }

  } //> ns Parsers

  namespace Utils
  {

    std::ostream& toStream( const DataObjID& v, std::ostream& o )
    {
      if ( v.clid() == 0 ) {
        o << "'" << v.key() << "'";
      } else {
        o << "(" << v.clid() << ",'" << v.key() << "')";
      }
      return o;
    }

    std::ostream& toStream( const DataObjIDColl& v, std::ostream& o )
    {
      o << "[";
      for ( auto& i : v ) {
        toStream( i, o );
        o << ",";
      }
      o << "]";
      return o;
    }

  } //> ns Utils

} //> ns Gaudi

//---------------------------------------------------------------------------

DataObjIDProperty::DataObjIDProperty( const std::string& name, DataObjID& ref )
    : PropertyWithHandlers( name, typeid( DataObjID ) ), m_pValue( &ref )
{
}

//---------------------------------------------------------------------------

DataObjIDProperty::~DataObjIDProperty() {}

//---------------------------------------------------------------------------

StatusCode DataObjIDProperty::fromString( const std::string& s )
{
  if ( !Gaudi::Parsers::parse( *m_pValue, s ).isSuccess() ) {
    return StatusCode::FAILURE;
  }
  return useUpdateHandler() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

//---------------------------------------------------------------------------

bool DataObjIDProperty::setValue( const DataObjID& value )
{
  m_pValue->operator=( value );
  return useUpdateHandler();
}

//---------------------------------------------------------------------------

std::string DataObjIDProperty::toString() const
{
  useReadHandler();
  std::ostringstream o;
  Gaudi::Utils::toStream( *m_pValue, o );
  return o.str();
}

//---------------------------------------------------------------------------
void DataObjIDProperty::toStream( std::ostream& out ) const
{
  useReadHandler();
  out << this->toString();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

DataObjIDCollProperty::DataObjIDCollProperty( const std::string& name, DataObjIDColl& ref )
    : PropertyWithHandlers( name, typeid( DataObjIDColl ) ), m_pValue( &ref )
{
}

//---------------------------------------------------------------------------

DataObjIDCollProperty::~DataObjIDCollProperty() {}

//---------------------------------------------------------------------------

StatusCode DataObjIDCollProperty::fromString( const std::string& s )
{
  if ( !Gaudi::Parsers::parse( *m_pValue, s ).isSuccess() ) {
    return StatusCode::FAILURE;
  }
  return useUpdateHandler() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

//---------------------------------------------------------------------------

bool DataObjIDCollProperty::setValue( const DataObjIDColl& value )
{
  m_pValue->operator=( value );
  return useUpdateHandler();
}

//---------------------------------------------------------------------------

std::string DataObjIDCollProperty::toString() const
{
  useReadHandler();
  std::ostringstream o;
  Gaudi::Utils::toStream( *m_pValue, o );
  return o.str();
}

//---------------------------------------------------------------------------
void DataObjIDCollProperty::toStream( std::ostream& out ) const
{
  useReadHandler();
  out << this->toString();
}
