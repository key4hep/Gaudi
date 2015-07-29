// $Id:$
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
    auto iter = container_.find(filename);
    if (iter != container_.end()) {
        *pos = &iter->second;
        assert(pos);
        return true;
    }
    return false;
}
// ============================================================================
