// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/PropertyList.h,v 1.2 2000/12/13 12:57:20 mato Exp $
#ifndef GAUDIKERNEL_PROPERTYLIST_H
#define GAUDIKERNEL_PROPERTYLIST_H

// Include Files
#include "Property.h"
#include "GaudiKernel/IProperty.h"
#include <vector>

/** @class PropertyList PropertyList.h GaudiKernel/PropertyList.h

    List of properties.

    @author David Quarrie
    @author Pere Mato
*/
class GAUDI_API PropertyList : public Property {
public:
  /// Constructor
	PropertyList();
  /// Constructor giving a name
	PropertyList( const std::string& name );
  /// Virtual destructor
	virtual ~PropertyList();

  /// Accessor members (const)
  bool isRef() const { return false; }

  /// Add a property to the list
  void addProperty( const std::string& name, Property& p );
  /// Add a property to the list
	void addProperty( Property& p );
  /// Remove a property from the list
	void removeProperty( const std::string& name );
  /// Remove a property from the list
	void removeProperty( Property& p );

	/// Property implementation: set property
  bool assign( const Property& p );
	/// Property implementation: get property
  bool load( Property& p) const;

	/// Property implementation: set property
  bool assign( const std::string& name, const Property& p );
	/// Property implementation: get property
  bool load( const std::string& name, Property& p) const;

  /// Get the property with a given name
  const Property& getProperty( const std::string& name) const;
  /// Get the complete list of properties
  const std::vector<Property*>& getProperties() const;
private:
	std::vector<Property*> m_propertyList;
};

#endif // GAUDIKERNEL_PROPERTYLIST_H

