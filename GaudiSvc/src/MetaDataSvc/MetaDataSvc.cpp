/*
 * MetaDataSvc.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: Ana Trisovic
 */

// Framework include files
#include "GaudiKernel/MetaData.h"

#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"

#include "MetaDataSvc.h"

using Gaudi::MetaDataSvc;

namespace
{
  const auto get_name = []( const auto* i ) { return i->name(); };

  struct Identity {
    template <typename T>
    auto operator()( T&& t ) const -> decltype( auto )
    {
      return std::forward<T>( t );
    }
  };

  template <typename Iterator, typename Sep, typename Projection = Identity>
  std::string join( Iterator first, Iterator last, Sep sep, Projection proj = {} )
  {
    std::string s;
    if ( first != last ) s += proj( *first++ );
    for ( ; first != last; ++first ) {
      s += sep;
      s += proj( *first );
    }
    return s;
  }
  template <typename Container, typename Sep, typename Projection = Identity>
  std::string join( const Container& c, Sep sep, Projection proj = {} )
  {
    return join( begin( c ), end( c ), std::move( sep ), std::move( proj ) );
  }
}

DECLARE_COMPONENT( MetaDataSvc )

StatusCode MetaDataSvc::start()
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "started" << endmsg;
  return collectData();
}
MetaData* MetaDataSvc::getMetaData() { return new MetaData( m_metadata ); }
std::map<std::string, std::string> MetaDataSvc::getMetaDataMap() { return m_metadata; }
StatusCode MetaDataSvc::collectData()
{
  for ( const auto* name : {"ApplicationMgr", "MessageSvc", "NTupleSvc"} ) {
    auto svc = service<IProperty>( name );
    if ( !svc.isValid() ) continue;
    const auto prefix = name + std::string{"."};
    for ( const auto* prop : svc->getProperties() ) {
      m_metadata[prefix + prop->name()] = prop->toString();
    }
  }

  /*
   * TOOLS
   * */
  SmartIF<IToolSvc> tSvc( serviceLocator()->service( "ToolSvc" ) );
  if ( tSvc.isValid() ) {
    m_metadata["ToolSvc"] = join( tSvc->getInstances( "" ), ", " );
  }

  /*
   * SERVICES
   * */
  m_metadata["ISvcLocator.Services"] = join( serviceLocator()->getServices(), ", ", get_name );

  /*
   * ALGORITHMS
   * */
  SmartIF<IAlgManager> algMan( serviceLocator() );
  m_metadata["IAlgManager.Algorithms"] = join( algMan->getAlgorithms(), ", ", get_name );

  /*
   * JOB OPTIONS SERVICE
   * */
  auto joSvc = service<IProperty>( "JobOptionsSvc" );
  if ( !joSvc.isValid() ) return StatusCode::FAILURE;
  for ( const auto* prop : joSvc->getProperties() ) {
    m_metadata["JobOptionsSvc." + prop->name()] = prop->toString();
  }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Metadata collected:\n";
    for ( const auto& item : m_metadata ) {
      debug() << item.first << ':' << item.second << '\n';
    }
    debug() << endmsg;
  }

  return StatusCode::SUCCESS;
}
