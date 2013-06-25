#ifndef GAUDIALG_HBOOKNAME_H
#define GAUDIALG_HBOOKNAME_H 1
// Include files
#include <string>
#include <algorithm>

/** @file
 *  few useful function to construct names of Hbook histograms
 *  and directories
 *  functions are imported from
 *  Tools/LoKi and Calo/CaloUtils packages
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2002-07-25
 */

namespace
{
  /** Simple function to convert any valid Gaudi address
   *  (name in Transient Store)
   *  to address, which is simultaneously valid for Hbook directory
   *
   *    examples:
   *   "verylongname"  -->  "verylong/name"
   *
   *  @param   old    old address
   *  @param   maxLen maximum allowed length of directory name (16 for Hbook)
   *  @return  new  address
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   06/07/2001
   */
  inline std::string dirHbookName
  ( const std::string& addr        ,
    const int          maxLen = 16 )
  {
    // ignore empty locations
    if( addr.empty() ) { return std::string(); }
    //
    std::string old( addr );
    // remove long names
    if( 0 < maxLen &&  maxLen < (int) old.size() )
    {
      std::string::iterator p1,p2;
      p1 = old.begin();
      const char sep('/');
      while( old.end() != p1 )
      {
        p1 =
          std::find_if( p1        ,
                        old.end() ,
                        std::bind2nd(std::not_equal_to<char>(),sep));
        p2 = std::find( p1 , old.end() , sep ) ;
        if( ( p2 - p1 ) <= (int) maxLen  ) { p1 = p2 ; continue ; }
        old.insert( p1 + maxLen , sep ) ;
        p1 = old.begin() ;
      }
    }
    ///
    return old;
  }

} // end of anonymous namespace

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_HBOOKNAME_H
// ============================================================================
