// ============================================================================
// STD:
// ============================================================================
#include <assert.h>
// ============================================================================
#include "IncludedFiles.h"
#include "Position.h"
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
bool gp::IncludedFiles::IsIncluded( const std::string& filename ) const {
  return container_.find( filename ) != container_.end();
}
// ============================================================================
bool gp::IncludedFiles::AddFile( std::string filename, Position from ) {
  return container_.emplace( std::move( filename ), std::move( from ) ).second;
}
// ============================================================================
bool gp::IncludedFiles::GetPosition( const std::string& filename, const Position** pos ) const {
  auto iter = container_.find( filename );
  if ( iter == container_.end() ) return false;
  *pos = &iter->second;
  assert( pos );
  return true;
}
// ============================================================================
