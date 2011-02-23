// $Id: Position.h,v 1.2 2007/05/24 14:41:21 hmd Exp $
// ============================================================================
// CVS tag $Name:  $ 
// ============================================================================
#ifndef JOBOPTIONSSVC_POSITION_H 
#define JOBOPTIONSSVC_POSITION_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <string>
// ============================================================================
namespace Gaudi{
  namespace Parsers {
    /** @class Position Position.h
     *
     *  Parser position entity
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2006-05-14
     */
    class Position
    {
    public:
      /** Constructor
       * @param fileName Name of file
       * @param line Line number
       * @param column Column number
       **/
      Position
      ( const std::string& fileName = "" ,
        const int          line     = 0  ,
        const int          column   = 0  )
        : m_fileName  ( fileName )
        , m_line      ( line     )
        , m_column    ( column   )
      {}
      /// Name of file
      std::string fileName() const{return m_fileName;}
      /// Line number
      int line() const{return m_line;}
      /// Column number
      int column() const{return m_column;}
    private:
      std::string m_fileName;
      int m_line;
      int m_column;
    };
  } // end of namespace Parsers
} // end of namespace Gaudi

// ============================================================================
// The END 
// ============================================================================
#endif // JOBOPTIONSSVC_POSITION_H
// ============================================================================
