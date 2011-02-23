// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/CommandProperty.h,v 1.3 2001/02/23 16:12:09 dquarrie Exp $
#ifndef GAUDIKERNEL_COMMANDPROPERTY_H
#define GAUDIKERNEL_COMMANDPROPERTY_H

// Include files
#include "Property.h"

/** @class CommandProperty CommandProperty.h GaudiKernel/CommandProperty.h

    Command property base class (abstract). The CommandProperty is a subclass of
    StringProperty that has a callback when the value is changed. Not very
    useful until we have a scripting language

    @author David Quarrie
*/
class GAUDI_API CommandProperty: public StringProperty {
public:
  /// Constructors and destructors
	CommandProperty();
	CommandProperty( const std::string& name );
	CommandProperty( const std::string& name, const std::string& value );
	virtual ~CommandProperty();
  /// Set value method
	virtual bool set( const std::string& value );
  /// User call back function
	virtual void handler( const std::string& value ) = 0;
};

#endif // GAUDIKERNEL_COMMANDPROPERTY_H

