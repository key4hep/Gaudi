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
#ifndef JOBOPTIONSVC_CATALOG_H_
#define JOBOPTIONSVC_CATALOG_H_
// ============================================================================
// STD & STL:
// ============================================================================
#include <iostream>
#include <map>
#include <string>
#include <vector>
// ============================================================================
// Boost:
// ============================================================================
#include <boost/ptr_container/ptr_set.hpp>
// ============================================================================
// Local:
// ============================================================================
#include "Property.h"
#include "PropertyName.h"
#include "PropertyValue.h"
// ============================================================================
// Namespace aliases:
// ============================================================================
// ...
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ============================================================================
    class Catalog final {
    public:
      typedef boost::ptr_set<Property, Property::LessThen>    PropertySet;
      typedef std::map<std::string, PropertySet, std::less<>> CatalogSet;

      typedef CatalogSet::value_type     value_type;
      typedef CatalogSet::iterator       iterator;
      typedef CatalogSet::const_iterator const_iterator;

      iterator       begin() { return catalog_.begin(); }
      const_iterator begin() const { return catalog_.begin(); }
      iterator       end() { return catalog_.end(); }
      const_iterator end() const { return catalog_.end(); }

      std::vector<std::string> ClientNames() const;
      bool                     Add( Property* property );
      template <typename Value>
      bool        Add( std::string client, std::string property, const Value& value );
      Property*   Find( std::string_view client, std::string_view name );
      std::string ToString() const;
      /// print the content of the catalogue to std::ostream
      std::ostream& fillStream( std::ostream& out ) const;

    private:
      CatalogSet catalog_;
    };
    // ============================================================================
    /// printout operator
    // ============================================================================
    inline std::ostream& operator<<( std::ostream& o, const Catalog& c ) { return c.fillStream( o ); }
    // ============================================================================
  } // namespace Parsers
} // namespace Gaudi

template <typename Value>
inline bool Gaudi::Parsers::Catalog::Add( std::string client, std::string property, const Value& value ) {
  return Add( new Property( PropertyName( std::move( client ), std::move( property ) ), PropertyValue( value ) ) );
}

// ============================================================================
#endif // JOBOPTIONSVC_CATALOG_H_
