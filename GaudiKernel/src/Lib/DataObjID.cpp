#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include <Gaudi/Parsers/Factory.h>
#include <functional>
#include <iomanip>
#include <iostream>

namespace {
  std::string quote( const std::string& in ) {
    if ( !in.empty() && in.front() == in.back() && ( in.front() == '\'' || in.front() == '\"' ) ) return in;
    std::stringstream s;
    s << std::quoted( in );
    return s.str();
  }
} // namespace

namespace Gaudi {
  namespace Parsers {

    template <typename Iterator, typename Skipper>
    struct DataObjIDGrammar : qi::grammar<Iterator, DataObjID(), Skipper> {
      using ResultT = DataObjID;
      struct Operations {
        template <typename ClassID>
        void operator()( DataObjID& dest, const std::pair<ClassID, std::string>& arg ) const {
          dest = {arg.first, arg.second};
        }
        void operator()( DataObjID& dest, const std::string& arg ) const { dest = {arg}; }
      };
      DataObjIDGrammar() : DataObjIDGrammar::base_type( result ) {
        result = atlas1_style[op( qi::_val, qi::_1 )] | atlas2_style[op( qi::_val, qi::_1 )] |
                 gaudi_style[op( qi::_val, qi::_1 )];
      }
      typename Grammar_<Iterator, std::string, Skipper>::Grammar                          gaudi_style;
      typename Grammar_<Iterator, std::pair<unsigned int, std::string>, Skipper>::Grammar atlas1_style;
      typename Grammar_<Iterator, std::pair<std::string, std::string>, Skipper>::Grammar  atlas2_style;
      qi::rule<Iterator, DataObjID(), Skipper>                                            result;
      ph::function<Operations>                                                            op;
    };
    REGISTER_GRAMMAR( DataObjID, DataObjIDGrammar );
  } // namespace Parsers
} // namespace Gaudi

StatusCode parse( DataObjID& dest, const std::string& src ) { return Gaudi::Parsers::parse_( dest, quote( src ) ); }

IClassIDSvc*   DataObjID::p_clidSvc( nullptr );
std::once_flag DataObjID::m_ip;
namespace {
  auto getClidSvc = []( std::reference_wrapper<IClassIDSvc*> p ) {
    p.get() = Gaudi::svcLocator()->service<IClassIDSvc>( "ClassIDSvc" ).get();
  };
}

void DataObjID::setClid() {
  std::call_once( m_ip, getClidSvc, std::ref( p_clidSvc ) );

  if ( !p_clidSvc || p_clidSvc->getIDOfTypeName( m_className, m_clid ).isFailure() ) {
    m_clid      = 0;
    m_className = "UNKNOWN_CLASS:" + m_className;
  }
}

void DataObjID::setClassName() {
  std::call_once( m_ip, getClidSvc, std::ref( p_clidSvc ) );

  if ( !p_clidSvc || p_clidSvc->getTypeNameOfID( m_clid, m_className ).isFailure() ) {
    m_className = "UNKNOW_CLID:" + std::to_string( m_clid );
  }
}

void DataObjID::hashGen() {
  m_hash = ( std::hash<std::string>()( m_key ) );
  if ( m_clid != 0 ) {
    // this is a bit redundant since hash<int> is a pass-through
    m_hash ^= ( std::hash<CLID>()( m_clid ) << 1 );
  }
}

#include "GaudiKernel/ToStream.h"
std::ostream& toStream( const DataObjID& d, std::ostream& os ) {
  using Gaudi::Utils::toStream;
  return ( d.m_clid != 0 || !d.m_className.empty() ) ? toStream( std::tie( d.m_className, d.m_key ), os )
                                                     : toStream( d.m_key, os );
}

std::string DataObjID::fullKey() const {
  return ( m_clid == 0 && m_className.empty() ) ? m_key : ( m_className + '/' + m_key );
}

std::string Gaudi::Details::Property::StringConverter<DataObjIDColl>::toString( const DataObjIDColl& v ) {
  return Gaudi::Utils::toString( v );
}

DataObjIDColl Gaudi::Details::Property::StringConverter<DataObjIDColl>::fromString( const DataObjIDColl&,
                                                                                    const std::string& s ) {
  DataObjIDColl c;
  if ( !Gaudi::Parsers::parse_( c, s ).isSuccess() ) {
    throw std::invalid_argument( "cannot parse '" + s + "' to DataObjIDColl" );
  }
  return c;
}
