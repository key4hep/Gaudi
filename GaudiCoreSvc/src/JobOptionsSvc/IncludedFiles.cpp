// $Id:$
// ============================================================================
// STD:
// ============================================================================
#include <assert.h>
// ============================================================================
// BOOST:
// ============================================================================
#include <boost/foreach.hpp>
// ============================================================================
#include "IncludedFiles.h"
#include "Position.h"
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
bool gp::IncludedFiles::IsIncluded(const std::string& filename) const {
    return container_.find(filename) != container_.end();
}
// ============================================================================
bool gp::IncludedFiles::AddFile(const std::string& filename,
        const Position& from){
    if (!IsIncluded(filename)) {
        container_.insert(Container::value_type(filename, from));
        return true;
    }
    return false;
}
// ============================================================================
bool gp::IncludedFiles::GetPosition(const std::string& filename,
        const Position** pos) const {
    Container::const_iterator iter = container_.find(filename);
    if (iter != container_.end()) {
        *pos = &iter->second;
        assert(pos != NULL);
        return true;
    }
    return false;
}
// ============================================================================
