#include "GaudiKernel/DataObjectHandleProperty.h"

#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/DataObjectHandleBase.h"

#include <sstream>
#include <map>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

// using namespace Gaudi;
// using namespace Gaudi::Parsers;
// using namespace Gaudi::Utils;




namespace Gaudi {
  namespace Parsers {

    StatusCode
    parse(DataObjectHandleBase& v, const std::string& s) {

      // std::cerr << "parse(DataObjectHandleBase) : " << s << std::endl;

      StatusCode sc(StatusCode::FAILURE);
      std::string prop;
      sc = Gaudi::Parsers::parse(prop, s);

      if (sc.isSuccess()) {

        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep("|");
        tokenizer tokens(prop, sep);
        int nToks(distance(tokens.begin(), tokens.end()));
        auto it = tokens.begin();

        if (nToks < 3 || nToks > 4) {
          std::cerr << "Unable to instantiante DataObjectHandle from Property: " << s
                    << " :: Format is bad" << std::endl;
          return StatusCode::FAILURE;
        }

        std::string k = *it;
        boost::erase_all(k,"'");
        boost::erase_all(k,"(");
        boost::erase_all(k,")");

        ++it;

        int m = std::stoi(*it);
        ++it;

        if (v.mode() != m) {
          std::cerr << "ERROR: mismatch in access mode for DataObjectHandle "
                    << v.fullKey() << "  " << v.mode() << " " << m << std::endl;
          return StatusCode::FAILURE;
        }


        bool o = bool(std::stoi(*it));

        v.setOptional(o);
        v.setKey( DataObjID(k) );

        // std::cout << "--> key: " << k << "  mode: " << m << "  opt: " << o << std::endl;

        if (nToks == 4) {
          // now parse the alt addresses
          ++it;
          std::string alt = *it;
          std::vector<std::string> av = v.alternativeDataProductNames();
          boost::char_separator<char> sep2("&");
          tokenizer tok2(alt,sep2);

          for (auto it2 : tok2 ) {
            std::string aa = it2;
            av.push_back(aa);
            // std::cout << "          altA: " << aa << std::endl;
          }

          v.setAlternativeDataProductNames(av);

        }
      }

      return StatusCode::SUCCESS;

    }
  }

  namespace Utils {

    std::ostream&
    toStream(const DataObjectHandleBase& v, std::ostream& o) {
      o << v;
      //      std::cerr << "toStream(DataObjectHandleBase) : " << v << std::endl;
      return o;
    }

  }
}

//---------------------------------------------------------------------------

DataObjectHandleProperty::DataObjectHandleProperty( const std::string& name,
                                      DataObjectHandleBase& ref )
  : PropertyWithHandlers( name, typeid( DataObjectHandleBase ) ),
    m_pValue( &ref )
{}

//---------------------------------------------------------------------------

DataObjectHandleProperty::~DataObjectHandleProperty()
{}

//---------------------------------------------------------------------------

StatusCode
DataObjectHandleProperty::fromString(const std::string& s)
{
  if (!Gaudi::Parsers::parse(*m_pValue, s).isSuccess()) {
    return StatusCode::FAILURE;
  }
  return useUpdateHandler()
    ? StatusCode::SUCCESS
    : StatusCode::FAILURE;
}


const std::string
DataObjectHandleProperty::pythonRepr() const {
  //  std::cerr << "DataObjectHandleProperty::pythonRepr() " << std::endl;
  return "DataObjectHandleBase(\"" + toString() + "\")";
}


//---------------------------------------------------------------------------

bool
DataObjectHandleProperty::setValue( const DataObjectHandleBase& value )
{
  m_pValue->operator=(value);
  return useUpdateHandler();
}

//---------------------------------------------------------------------------

std::string
DataObjectHandleProperty::toString( ) const
{
  useReadHandler();
  std::ostringstream o;
  Gaudi::Utils::toStream(*m_pValue, o);
  return o.str();
}

//---------------------------------------------------------------------------
void
DataObjectHandleProperty::toStream(std::ostream& out) const
{
  useReadHandler();
  out << this->toString();
}

//---------------------------------------------------------------------------

DataObjectHandleProperty&
DataObjectHandleProperty::operator=( const DataObjectHandleBase& value )
{
  setValue( value );
  return *this;
}

//---------------------------------------------------------------------------

DataObjectHandleProperty*
DataObjectHandleProperty::clone() const
{
  return new DataObjectHandleProperty( *this );
}

//---------------------------------------------------------------------------

bool
DataObjectHandleProperty::load( Property& destination ) const
{
  return destination.assign( *this );
}

//---------------------------------------------------------------------------

bool
DataObjectHandleProperty::assign( const Property& source )
{
  return fromString( source.toString() ).isSuccess();
}

//---------------------------------------------------------------------------

const DataObjectHandleBase&
DataObjectHandleProperty::value() const
{
  useReadHandler();
  return *m_pValue;
}
