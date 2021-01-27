/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
//	HistogramPersistencySvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Event data persistency service
//               This specialized service only deals with event related
//               data
//
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define PERSISTENCYSVC_HISTOGRAMPERSISTENCYSVC_CPP
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
// ============================================================================
// local
// ============================================================================
#include "HistogramPersistencySvc.h"
// ============================================================================
// AIDA
// ============================================================================
/// @FIXME: AIDA interfaces visibility
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define class class GAUDI_API
#ifdef __clang__
#  pragma clang diagnostic pop
#endif
#include "AIDA/IBaseHistogram.h"
#undef class
// ============================================================================
// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( HistogramPersistencySvc )

// ============================================================================
// Finalize the service.
StatusCode HistogramPersistencySvc::finalize() {
  //
  if ( !( m_convert.empty() && m_exclude.empty() ) ) { // print message if any of the two properties is used
    info() << "Histograms Converted/Excluded: " << m_converted.size() << "/" << m_excluded.size() << endmsg;
  }
  if ( msgLevel( MSG::DEBUG ) ) {
    if ( !m_excluded.empty() ) {
      auto& log = debug();
      log << "Excluded  Histos : #" << m_excluded.size();
      for ( const auto& item : m_excluded ) { log << std::endl << "  '" << item << "'"; }
      log << endmsg;
    }
    //
    if ( !m_converted.empty() ) {
      auto& log = debug();
      log << "Converted Histos : #" << m_converted.size();
      for ( const auto& item : m_converted ) { log << std::endl << "  '" << item << "'"; }
      log << endmsg;
    }
  }
  return PersistencySvc::finalize();
}
// ============================================================================
// Initialize the service.
// ============================================================================
StatusCode HistogramPersistencySvc::initialize() {
  StatusCode status = PersistencySvc::initialize();
  return status.isSuccess() ? reinitialize() : status;
}
// ============================================================================
// Reinitialize the service.
// ============================================================================
StatusCode HistogramPersistencySvc::reinitialize() {
  // Obtain the IProperty of the ApplicationMgr
  auto prpMgr = serviceLocator()->as<IProperty>();
  if ( !prpMgr ) {
    fatal() << "IProperty interface not found in ApplicationMgr." << endmsg;
    return StatusCode::FAILURE;
  } else {
    setProperty( prpMgr->getProperty( "HistogramPersistency" ) ).ignore();
  }

  // To keep backward compatibility, we set the property of conversion service
  // into JobOptions catalogue
  if ( !m_outputFile.empty() ) {
    auto&             opts       = serviceLocator()->getOptsSvc();
    const std::string outputFile = '"' + m_outputFile + '"';
    if ( m_histPersName == "ROOT" ) {
      opts.set( "RootHistSvc.OutputFile", outputFile );
    } else if ( m_histPersName == "HBOOK" ) {
      opts.set( "HbookHistSvc.OutputFile", outputFile );
    }
  }

  // Load the Histogram persistency service that's required as default
  setConversionSvc( nullptr ).ignore();
  if ( m_histPersName == "ROOT" ) {
    setConversionSvc( service( "RootHistSvc" ) ).ignore();
    if ( !conversionSvc() ) { return StatusCode::FAILURE; }
    enable( true );
  } else if ( m_histPersName == "HBOOK" ) {
    setConversionSvc( service( "HbookHistSvc" ) ).ignore();
    if ( !conversionSvc() ) { return StatusCode::FAILURE; }
    enable( true );
  } else if ( m_histPersName == "NONE" ) {
    enable( false );
    if ( msgLevel( MSG::DEBUG ) ) debug() << "Histograms saving not required." << endmsg;
  } else {
    setConversionSvc( service( m_histPersName ) ).ignore();
    if ( !conversionSvc() ) { return StatusCode::FAILURE; }
    enable( true );
    if ( m_warnings ) { warning() << "Unknown Histogram Persistency Mechanism " << m_histPersName << endmsg; }
  }
  return StatusCode::SUCCESS;
}
// ============================================================================
namespace {
  // ==========================================================================
  /// invalid name
  const std::string s_NULL = "<NULL>";
  // ==========================================================================
  /** check the match of the full name and the pattern
   *  @param obj the object
   *  @param pat the pattern
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   */
  // ==========================================================================
  inline bool match( const std::string& name, const std::string& pat ) {
    // the most primitive match
    return std::string::npos != name.find( pat );
  }
  // ==========================================================================
  /** get the ful name of data object
   *  @param obj the object
   *  @return the full name
   */
  inline const std::string& oname( const DataObject* obj ) {
    if ( !obj ) { return s_NULL; }
    auto reg = obj->registry();
    return reg ? reg->identifier() : obj->name();
  }
  // ==========================================================================
  /** check the match of the full name of data object with the pattern
   *  @param obj the object
   *  @param pat the pattern
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   */
  inline bool match( const DataObject* obj, const std::string& pat ) { return obj && match( oname( obj ), pat ); }
  // ==========================================================================
} // namespace
// ============================================================================
// Convert the transient object to the requested representation.
// ============================================================================
StatusCode HistogramPersistencySvc::createRep( DataObject* pObj, IOpaqueAddress*& refpAddr ) {
  // enable the conversion
  enable( true );
  // conversion is possible ?
  if ( "NONE" == m_histPersName ) {
    enable( false );
    return PersistencySvc::createRep( pObj, refpAddr ); // RETURN
  }
  // histogram ?
  if ( dynamic_cast<AIDA::IBaseHistogram*>( pObj ) ) {

    auto match_pObj = [&]( const std::string& s ) { return match( pObj, s ); };
    // Empty ConvertHistos property means convert all
    bool select = ( m_convert.empty() || std::any_of( m_convert.begin(), m_convert.end(), match_pObj ) ) &&
                  std::none_of( m_exclude.begin(), m_exclude.end(), match_pObj );
    //
    enable( select );
    //
    const auto& path = oname( pObj );
    //
    if ( !select ) {
      m_excluded.insert( path );
    } else {
      m_converted.insert( path );
    }
  }
  //
  return PersistencySvc::createRep( pObj, refpAddr ); // RETURN
}
// ============================================================================
// Standard Constructor
// ============================================================================
HistogramPersistencySvc::HistogramPersistencySvc( const std::string& name, ISvcLocator* svc )
    : PersistencySvc( name, svc ) {
  // bypass update handler
  m_svcNames.value() = std::vector<std::string>{{"RootHistSvc"}};
}

// ============================================================================
// The END
// ============================================================================
