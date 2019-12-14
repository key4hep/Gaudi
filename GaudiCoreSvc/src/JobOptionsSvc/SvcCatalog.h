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
#ifndef JOBOPTIONSSVC_SVCCATALOG_H
#define JOBOPTIONSSVC_SVCCATALOG_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <map>
#include <string>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Property.h"
#include "GaudiKernel/StatusCode.h"
// ===========================================================================

/** @class SvcCatalog
 *
 *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-05-13
 */
class SvcCatalog final {
public:
  typedef std::vector<const Gaudi::Details::PropertyBase*> PropertiesT;
  SvcCatalog() = default;
  ~SvcCatalog();

  StatusCode addProperty( const std::string& client, const Gaudi::Details::PropertyBase* property );

  StatusCode                          removeProperty( const std::string& client, const std::string& name );
  const PropertiesT*                  getProperties( const std::string& client ) const;
  std::vector<std::string>            getClients() const;
  const Gaudi::Details::PropertyBase* getProperty( const std::string& client, const std::string& name ) const;

public:
  /// dump the content of catalog to std::ostream
  std::ostream& fillStream( std::ostream& o ) const;

private:
  const PropertiesT*                           findProperties( const std::string& client ) const;
  PropertiesT*                                 findProperties( const std::string& client );
  std::pair<bool, PropertiesT::const_iterator> findProperty( const PropertiesT& props, const std::string& name ) const;
  std::pair<bool, PropertiesT::iterator>       findProperty( PropertiesT& props, const std::string& name );
  std::map<std::string, PropertiesT>           m_catalog;
};
// ============================================================================
/// printoput operator
// ============================================================================
std::ostream& operator<<( std::ostream& o, const SvcCatalog& c );
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif // JOBOPTIONSSVC_SVCCATALOG_H
