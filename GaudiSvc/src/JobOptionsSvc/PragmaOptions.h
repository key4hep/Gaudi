// $Id:$
//$Id:$
// ===========================================================================
#ifndef JOBOPTIONSVC_PRAGMAOPTIONS_H_
#define JOBOPTIONSVC_PRAGMAOPTIONS_H_
// ===========================================================================
// Include files
// ===========================================================================
// STD & STL
// ===========================================================================
// ===========================================================================
// Boost
// ===========================================================================
// ===========================================================================
// Gaudi
// ===========================================================================
// ===========================================================================
// Local
// ===========================================================================
// ===========================================================================

namespace  Gaudi { namespace Parsers {
// ===========================================================================
class PragmaOptions {
 public:
  PragmaOptions(): m_is_print(true), m_is_print_options(false),
    m_is_print_tree(false){}

  bool is_print() const { return m_is_print;}
  void setIsPrint(bool is_print) { m_is_print = is_print; }

  bool is_print_options() const { return m_is_print_options;}
  void setIsPrintOptions(bool is_print_options)
  { m_is_print_options = is_print_options; }

  bool is_print_tree() const { return m_is_print_tree;}
  void setIsPrintTree(bool is_print_tree)
   { m_is_print_tree = is_print_tree; }

  const std::string& dumpFile() const { return m_dump_file;}
  void setDumpFile(const std::string& dump_file) {
    m_dump_file = dump_file;
  }

  bool IsPrintOptions() { return is_print_options() && is_print();}
  bool IsPrintTree() { return is_print_tree() && is_print();}
  bool HasDumpFile() { return !m_dump_file.empty();}

 private:
  bool m_is_print;
  bool m_is_print_options;
  bool m_is_print_tree;
  std::string m_dump_file;
};
// ===========================================================================
} /*  Gaudi */ }  /* Parsers */
// ===========================================================================
#endif
