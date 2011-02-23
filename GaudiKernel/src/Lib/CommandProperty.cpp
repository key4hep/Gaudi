// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/CommandProperty.cpp,v 1.2 2001/02/23 16:12:58 dquarrie Exp $

#include "GaudiKernel/CommandProperty.h"

CommandProperty::CommandProperty()
: StringProperty( ) {
}

CommandProperty::CommandProperty( const std::string& name )
: StringProperty( name ) {
}

CommandProperty::CommandProperty( const std::string& name,
                                  const std::string& value )
: StringProperty( name, value ) {
}

CommandProperty::~CommandProperty() {
}

bool CommandProperty::set( const std::string& value )
{
  bool result = StringProperty::set( value );
  handler( value );
  return result;
}
