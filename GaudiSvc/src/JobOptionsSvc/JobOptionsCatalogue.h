// $Id: JobOptionsCatalogue.h,v 1.7 2007/05/24 14:41:22 hmd Exp $
// ============================================================================
// CVS tag $Name:  $ 
// ============================================================================
#ifndef JOBOPTIONSSVC_JOBOPTIONSCATALOGUE_H 
#define JOBOPTIONSSVC_JOBOPTIONSCATALOGUE_H 1
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

/** @class JobOptionsCatalogue
 *
 *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-05-13
 */
class JobOptionsCatalogue
{
public:
  typedef std::vector<const Property*> PropertiesT;
  typedef std::map<std::string, PropertiesT > ObjectsT;
  JobOptionsCatalogue();
  virtual ~JobOptionsCatalogue();  
  
  StatusCode addProperty( const std::string& client, 
                                             const Property* property );
  
  StatusCode removeProperty( const std::string& client, 
                                                  const std::string& name );
  const PropertiesT* getProperties( const std::string& client) const;
  std::vector<std::string> getClients() const;
public:
  /// dump the content of catalogue to std::ostream 
  std::ostream& fillStream ( std::ostream& o ) const ;
private:
  PropertiesT* findProperties(const std::string& client) const;
  bool findProperty(PropertiesT* props,
                    const std::string& name,PropertiesT::iterator& result);
  ObjectsT* m_catalogue;
};
// ============================================================================
/// printoput operator 
// ============================================================================
std::ostream& operator<<( std::ostream& o , const JobOptionsCatalogue& c ) ;
// ============================================================================

// ============================================================================
// The END 
// ============================================================================
#endif // JOBOPTIONSSVC_JOBOTIONSCATALOGUE_H
// ============================================================================
