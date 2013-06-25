// ============================================================================
// INCLUDE
// ============================================================================
#include "Messages.h"
// ============================================================================
// BOOST:
// ============================================================================
#include <boost/foreach.hpp>
#include <boost/format.hpp>
// ============================================================================
// Namespaces:
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
// ============================================================================
void gp::Messages::AddMessage(MSG::Level level,
        const std::string& message) {
  stream_ << level << message << endmsg;
}
// ============================================================================
void gp::Messages::AddMessage(MSG::Level level,
        const Position& pos, const std::string& message) {
  if (pos.filename() != m_currentFilename) {
    stream_ << level << "# =======> " << pos.filename() << endmsg;
    m_currentFilename = pos.filename();
  }
  std::string str_msg = str(boost::format("(%1%,%2%): %3%")
                % pos.line() % pos.column() % message);
  stream_ << level << "# " << str_msg << endmsg;
}
// ============================================================================
