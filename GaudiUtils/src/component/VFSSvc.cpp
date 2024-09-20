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
#include <GaudiKernel/HashMap.h>
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/IFileAccess.h>
#include <GaudiKernel/IToolSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Service.h>
#include <list>

/** @class VFSSvc VFSSvc.h
 *
 *  Simple service that allows to read files independently from the storage.
 *  The service uses tools to resolve URLs and serve the files as input streams.
 *  The basic implementations read from the filesystem, and simple extensions allow to
 *  read from databases, web...
 *
 *  @author Marco Clemencic
 *  @date   2008-01-18
 */

class VFSSvc : public extends<Service, IFileAccess> {
public:
  /// Inherited constructor
  using extends::extends;
  /// Initialize Service
  StatusCode initialize() override;
  /// Finalize Service
  StatusCode finalize() override;

  /// @see IFileAccess::open
  std::unique_ptr<std::istream> open( std::string const& url ) override;

  /// @see IFileAccess::protocols
  const std::vector<std::string>& protocols() const override;

private:
  Gaudi::Property<std::vector<std::string>> m_urlHandlersNames{
      this, "FileAccessTools", { { "FileReadTool" } }, "List of tools implementing the IFileAccess interface." };
  Gaudi::Property<std::string> m_fallBackProtocol{ this, "FallBackProtocol", "file",
                                                   "URL prefix to use if the prefix is not present." };

  /// Protocols registered
  std::vector<std::string> m_protocols;

  /// Map of the tools handling the known protocols.
  GaudiUtils::HashMap<std::string, std::vector<IFileAccess*>> m_urlHandlers;

  /// Handle to the tool service.
  SmartIF<IToolSvc> m_toolSvc;

  /// List of acquired tools (needed to release them).
  std::vector<IAlgTool*> m_acquiredTools;
};

DECLARE_COMPONENT( VFSSvc )

//------------------------------------------------------------------------------
StatusCode VFSSvc::initialize() {
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) return sc;

  m_toolSvc = serviceLocator()->service( "ToolSvc" );
  if ( !m_toolSvc ) {
    error() << "Cannot locate ToolSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  IAlgTool* tool;
  for ( const auto& i : m_urlHandlersNames ) {
    // retrieve the tool and the pointer to the interface
    sc = m_toolSvc->retrieve( i, IAlgTool::interfaceID(), tool, nullptr, true );
    if ( sc.isFailure() ) {
      error() << "Cannot get tool " << i << endmsg;
      return sc;
    }
    m_acquiredTools.push_back( tool ); // this is one tool that we will have to release
    auto hndlr = SmartIF<IFileAccess>( tool );
    if ( !hndlr ) {
      error() << i << " does not implement IFileAccess" << endmsg;
      return StatusCode::FAILURE;
    }
    // We do not need to increase the reference count for the IFileAccess interface
    // because we hold the tool by its IAlgTool interface.
    // loop over the list of supported protocols and add them to the map (for quick access)
    for ( const auto& prot : hndlr->protocols() ) m_urlHandlers[prot].emplace_back( hndlr.get() );
  }

  // Now let's check if we can handle the fallback protocol
  if ( m_urlHandlers.find( m_fallBackProtocol ) == m_urlHandlers.end() ) {
    error() << "No handler specified for fallback protocol prefix " << m_fallBackProtocol.value() << endmsg;
    return StatusCode::FAILURE;
  }

  // Note: the list of handled protocols will be filled only if requested

  return sc;
}
//------------------------------------------------------------------------------
StatusCode VFSSvc::finalize() {
  m_urlHandlers.clear(); // clear the map
  m_protocols.clear();

  if ( m_toolSvc ) {
    // release the acquired tools (from the last acquired one)
    while ( !m_acquiredTools.empty() ) {
      m_toolSvc->releaseTool( m_acquiredTools.back() ).ignore();
      m_acquiredTools.pop_back();
    }
    m_toolSvc.reset(); // release the tool service
  }
  return Service::finalize();
}
//------------------------------------------------------------------------------
std::unique_ptr<std::istream> VFSSvc::open( std::string const& url ) {
  // get the url prefix endpos
  auto pos = url.find( "://" );

  if ( url.npos == pos ) {
    // no url prefix, try fallback protocol
    return VFSSvc::open( std::string{ m_fallBackProtocol }.append( "://" ).append( url ) );
  }

  const std::string url_prefix( url, 0, pos );
  const auto        handlers = m_urlHandlers.find( url_prefix );
  if ( handlers == m_urlHandlers.end() ) {
    // if we do not have a handler for the URL prefix,
    // use the fall back one
    return VFSSvc::open( std::string{ m_fallBackProtocol }.append( url.substr( pos ) ) );
  }

  std::unique_ptr<std::istream> out; // this might help RVO
  // try the hendlers for the protocol one after the other until one succeds
  for ( auto hndlr : handlers->second ) {
    out = hndlr->open( url );
    if ( out ) break;
  }
  return out;
}

//------------------------------------------------------------------------------
const std::vector<std::string>& VFSSvc::protocols() const {
  if ( m_protocols.empty() ) {
    // prepare the list of handled protocols
    std::transform( m_urlHandlers.begin(), m_urlHandlers.end(),
                    std::back_inserter( const_cast<VFSSvc*>( this )->m_protocols ),
                    []( const auto& pair ) { return pair.first; } );
  }
  return m_protocols;
}
