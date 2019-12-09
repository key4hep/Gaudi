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
#ifndef GAUDIALG_FIXTESPATH_H
#define GAUDIALG_FIXTESPATH_H
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

namespace FixTESPathDetails {
  std::unique_ptr<IDataHandleVisitor> fixDataHandlePath( std::string_view rit, std::string rootName, MsgStream* dbg );
  std::string                         fullTESLocation( std::string_view location, std::string_view rit );
} // namespace FixTESPathDetails

template <class BASE>
class FixTESPath : public BASE {
public:
  /// Algorithm constructor - the SFINAE constraint below ensures that this is
  /// constructor is only defined if BASE derives from Algorithm
  template <typename U = BASE, typename = std::enable_if_t<std::is_base_of_v<Gaudi::Algorithm, BASE>, U>>
  FixTESPath( std::string name, ISvcLocator* pSvcLocator ) : BASE( std::move(name), pSvcLocator ) {}

  /// Tool constructor - SFINAE-ed to insure this constructor is only defined
  /// if BASE derives from AlgTool.
  template <typename U = BASE, typename = std::enable_if_t<std::is_base_of_v<AlgTool, BASE>, U>>
  FixTESPath( std::string type, std::string name, const IInterface* ancestor )
      : BASE( std::move(type), std::move(name), ancestor ) {
    // setup RootInTES from parent if available
    if ( const IProperty* ancestorProp = dynamic_cast<const IProperty*>( ancestor );
         ancestorProp && ancestorProp->hasProperty( "RootInTES" ) ) {
      this->setProperty( ancestorProp->getProperty( "RootInTES" ) ).ignore();
    }
  }

  StatusCode initialize() override {
    const StatusCode sc = BASE::initialize();
    if ( sc.isFailure() ) return sc;
    // TODO: just call 'acceptDHVisitor` and remove m_updateDataHandles...
    SmartIF<IDataManagerSvc> dataMgrSvc{BASE::evtSvc()};
    this->m_updateDataHandles = FixTESPathDetails::fixDataHandlePath(
        rootInTES(), dataMgrSvc->rootName(), BASE::msgLevel( MSG::DEBUG ) ? &this->debug() : nullptr );
    return sc;
  }

  /** @brief Returns the "rootInTES" string.
   *  Used as the directory root in the TES for which all data access refers to (both saving and retrieving).
   */
  const std::string& rootInTES() const { return m_rootInTES; }
  // ==========================================================================
  /// Returns the full correct event location given the rootInTes settings
  std::string fullTESLocation( std::string_view location, bool useRootInTES ) const {
    return FixTESPathDetails::fullTESLocation( location, useRootInTES ? rootInTES() : std::string_view{} );
  }
  // ==========================================================================
private:
  Gaudi::Property<std::string> m_rootInTES{this,
                                           "RootInTES",
                                           {},
                                           [=]( Gaudi::Details::PropertyBase& ) { // Check rootInTES ends with a '/'
                                             auto& rit = this->m_rootInTES.value();
                                             if ( !rit.empty() && rit.back() != '/' ) rit += '/';
                                           },
                                           "note: overridden by parent settings"};
};

#endif
