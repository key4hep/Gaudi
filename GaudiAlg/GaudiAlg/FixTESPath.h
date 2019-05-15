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
  template <typename U = BASE, typename = std::enable_if_t<std::is_base_of<Gaudi::Algorithm, BASE>::value, U>>
  FixTESPath( const std::string& name, ISvcLocator* pSvcLocator ) : BASE( name, pSvcLocator ) {}

  /// Tool constructor - SFINAE-ed to insure this constructor is only defined
  /// if BASE derives from AlgTool.
  template <typename U = BASE, typename = std::enable_if_t<std::is_base_of<AlgTool, BASE>::value, U>>
  FixTESPath( const std::string& type, const std::string& name, const IInterface* ancestor )
      : BASE( type, name, ancestor ) {
    // setup context from parent if available
    if ( const auto* gAlg = dynamic_cast<const FixTESPath<Algorithm>*>( ancestor ); gAlg ) {
      this->setProperty( "RootInTES", gAlg->rootInTES() ).ignore();
    } else if ( const auto* gTool = dynamic_cast<const FixTESPath<AlgTool>*>( ancestor ); gTool ) {
      this->setProperty( "RootInTES", gTool->rootInTES() ).ignore();
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
