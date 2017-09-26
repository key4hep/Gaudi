// ============================================================================
// Include files
// ============================================================================
#include "Units.h"
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
bool gp::Units::Add( std::string name, double value ) { return Add( std::move( name ), value, Position() ); }
// ============================================================================
bool gp::Units::Add( std::string name, double value, const Position& pos )
{
  return units_.emplace( std::move( name ), ValueWithPosition( value, pos ) ).second;
}
// ============================================================================
bool gp::Units::Find( const std::string& name, double& result ) const
{
  ValueWithPosition r;
  if ( !Find( name, r ) ) return false;
  result = r.first;
  return true;
}
// ============================================================================
bool gp::Units::Find( const std::string& name, ValueWithPosition& result ) const
{
  auto it = units_.find( name );
  if ( it == units_.end() ) return false;
  result = it->second;
  return true;
}
