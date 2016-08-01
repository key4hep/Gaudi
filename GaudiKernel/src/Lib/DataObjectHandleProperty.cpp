#include "GaudiKernel/DataObjectHandleProperty.h"

#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/DataObjectHandleBase.h"

#include "GaudiKernel/System.h"


#include <sstream>
#include <map>
#include "boost/tokenizer.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/range/algorithm_ext/erase.hpp"
#include "boost/algorithm/string/classification.hpp"

namespace Gaudi {
  namespace Parsers {

    StatusCode
    parse(DataObjectHandleBase& v, const std::string& s) {

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
          //std::cerr << "Need either 3 or 4 tokens. Got: " << std::endl;
          return StatusCode::FAILURE;
        }

        std::string k = *it++;
        boost::remove_erase_if(k,boost::algorithm::is_any_of("'()"));

        int m = std::stoi(*it++);
        if (v.mode() != m) {
          // std::cerr << "WARNING: mismatch in access mode for DataObjectHandle "
          //           << v.fullKey() << "  " << v.mode() << " " << m 
          //           << " ignoring  the latter, specified value" << std::endl;
        }
        bool o = bool(std::stoi(*it++));

        v.setOptional(o);
        v.setKey( DataObjID(k) );

        // std::cout << "--> key: " << k << "  mode: " << m << "  opt: " << o << std::endl;

        if (nToks == 4) { // now parse the alt addresses
          std::string alt = *it++;
          boost::char_separator<char> sep2("&");
          tokenizer tok2(alt,sep2);
          // do we really want to _append_ instead of _assign_ ?
          std::vector<std::string> av = v.alternativeDataProductNames();
          for (auto it2 : tok2 ) av.push_back(it2);
          v.setAlternativeDataProductNames(av);
        }
      }

      return StatusCode::SUCCESS;

    }
  }

  namespace Utils {

    std::ostream&
    toStream(const DataObjectHandleBase& v, std::ostream& o) {
      return o << v;
    }

  }
}

//---------------------------------------------------------------------------

DataObjectHandleProperty::DataObjectHandleProperty( const std::string& name,
                                      DataObjectHandleBase& ref )
  : Property( name, typeid( DataObjectHandleBase ) ),
    m_pValue( &ref )
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


std::string
DataObjectHandleProperty::pythonRepr() const {
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
  return m_pValue->toString();
}

//---------------------------------------------------------------------------
void
DataObjectHandleProperty::toStream(std::ostream& out) const
{
  // implicitly invokes useReadHandler()
  out << toString();
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
