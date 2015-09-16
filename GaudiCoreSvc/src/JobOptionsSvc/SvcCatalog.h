// ============================================================================
#ifndef JOBOPTIONSSVC_SVCCATALOG_H
#define JOBOPTIONSSVC_SVCCATALOG_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <map>
#include <vector>
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Property.h"
// ===========================================================================

/** @class SvcCatalog
 *
 *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-05-13
 */
class SvcCatalog final
{
public:
  typedef std::vector<const Property*> PropertiesT;
  SvcCatalog() = default;
  ~SvcCatalog();

  StatusCode addProperty( const std::string& client,
                          const Property* property );

  StatusCode removeProperty( const std::string& client,
                             const std::string& name );
  const PropertiesT* getProperties( const std::string& client) const;
  std::vector<std::string> getClients() const;
public:
  /// dump the content of catalog to std::ostream
  std::ostream& fillStream ( std::ostream& o ) const ;
private:
  const PropertiesT* findProperties(const std::string& client) const;
  PropertiesT* findProperties(const std::string& client) ;
  std::pair<bool, PropertiesT::iterator> findProperty(PropertiesT& props, const std::string& name);
  std::map<std::string, PropertiesT> m_catalog;
};
// ============================================================================
/// printoput operator
// ============================================================================
std::ostream& operator<<( std::ostream& o , const SvcCatalog& c ) ;
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif // JOBOPTIONSSVC_SVCCATALOG_H
// ===========================================================================
