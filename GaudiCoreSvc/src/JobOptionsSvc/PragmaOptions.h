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
// ===========================================================================
#ifndef JOBOPTIONSVC_PRAGMAOPTIONS_H_
#define JOBOPTIONSVC_PRAGMAOPTIONS_H_
// ===========================================================================
// Include files
// ===========================================================================
// STD & STL
// ===========================================================================
#include <string>
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

namespace Gaudi {
  namespace Parsers {
    // ===========================================================================
    class PragmaOptions final {
    public:
      PragmaOptions() = default;

      bool is_print() const { return m_is_print; }
      void setIsPrint( bool is_print ) { m_is_print = is_print; }

      bool is_print_options() const { return m_is_print_options; }
      void setIsPrintOptions( bool is_print_options ) { m_is_print_options = is_print_options; }

      bool is_print_tree() const { return m_is_print_tree; }
      void setIsPrintTree( bool is_print_tree ) { m_is_print_tree = is_print_tree; }

      const std::string& dumpFile() const { return m_dump_file; }
      void               setDumpFile( std::string dump_file ) { m_dump_file = std::move( dump_file ); }

      bool IsPrintOptions() { return is_print_options() && is_print(); }
      bool IsPrintTree() { return is_print_tree() && is_print(); }
      bool HasDumpFile() { return !m_dump_file.empty(); }

    private:
      bool        m_is_print         = true;
      bool        m_is_print_options = false;
      bool        m_is_print_tree    = false;
      std::string m_dump_file;
    };
    // ===========================================================================
  } // namespace Parsers
} // namespace Gaudi
// ===========================================================================
#endif
