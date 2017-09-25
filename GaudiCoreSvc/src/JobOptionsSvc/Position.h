#ifndef JOBOPTIONSVC_POSITION_H_
#define JOBOPTIONSVC_POSITION_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
// ============================================================================
namespace Gaudi
{
  namespace Parsers
  {
    // ============================================================================
    class Position final
    {
    public:
      Position() = default;
      Position( std::string filename, unsigned line, unsigned column )
          : filename_( std::move( filename ) ), line_( line ), column_( column )
      {
      }
      const std::string& filename() const { return filename_; }
      unsigned line() const { return line_; }
      unsigned column() const { return column_; }
      void set_filename( std::string filename ) { filename_ = std::move( filename ); }
      std::string ToString() const;
      bool Exists() const { return line_ != 0; }

    private:
      std::string filename_;
      unsigned line_   = 0;
      unsigned column_ = 0;
    };
    // ============================================================================
  } /* Gaudi */
} /* Parsers */
// ============================================================================
#endif // JOBOPTIONSVC_POSITION_H_
