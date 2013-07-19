// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <vector>
#include <map>
#include <string>
#include <utility>
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================

#ifdef __ICC
// disable icc remark #177: declared but never referenced
#pragma warning(disable:177)
#endif

/** @file
 *  simple DEMO-file for "extended properties",
 *  implementation file for class ExtendedProperties
 *  @author Alexander MAZUROV alexander.mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date  2006-03-21
 */


/** @class ExtendedProperties
 *  simple DEMO-file for "extended properties",
 *  implementation file for class ExtendedProperties
 *  @author Alexander MAZUROV alexander.mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date  2006-03-21
 */

class ExtendedProperties
  : public GaudiAlgorithm
{
public:
  StatusCode execute() ;

  ExtendedProperties
  ( const std::string& name ,
    ISvcLocator*       pSvc )
    : GaudiAlgorithm ( name , pSvc )
    , m_1()
    , m_2()
    , m_3()
    , m_4()
    , m_5()
    , m_6()
    , m_7()
    , m_8()
    , m_9()
    , m_10()
    , m_11()
    , m_12()
    , m_13()
    , m_14()
  {
    declareProperty ( "PairDD"                   , m_1 ) ;

    declareProperty ( "VectorOfPairsDD"          , m_2 ) ;
    declareProperty ( "VectorOfVectorsString"    , m_3 ) ;
    declareProperty ( "VectorOfVectorsDouble"    , m_4 ) ;

    declareProperty ( "MapIntDouble"             , m_5 ) ;
    declareProperty ( "MapStringString"          , m_6 ) ;
    declareProperty ( "MapStringInt"             , m_7 ) ;
    declareProperty ( "MapStringDouble"          , m_8 ) ;
    declareProperty ( "MapStringVectorOfStrings" , m_9 ) ;

    declareProperty ( "PairII"                   , m_10) ;
    declareProperty ( "MapStringVectorOfDoubles" , m_11) ;
    declareProperty ( "MapStringVectorOfInts"    , m_12) ;

    declareProperty ( "MapIntInt"                , m_13) ;
    declareProperty ( "VectorOfPairsII"          , m_14) ;

    declareProperty ( "MapIntString"             , m_15) ;
    declareProperty ( "MapUIntString"            , m_16) ;

    declareProperty ( "EmptyMap"                 , m_20) ;
    declareProperty ( "EmptyVector"              , m_21) ;

    setProperty ("PropertiesPrint", "true").ignore() ;

    m_20["key"] = "value";
    m_21.push_back(123);

  }
  /// destruictor: virtual and protected
  virtual ~ExtendedProperties(){}
private:
  // default constructor is disabled
  ExtendedProperties();
  // copy constructor is disabled
  ExtendedProperties ( const ExtendedProperties& );
  // assignement operator is disabled
  ExtendedProperties& operator=( const ExtendedProperties& );
private:
  std::pair<double,double>               m_1 ;

  std::vector<std::pair<double,double> > m_2 ;
  std::vector<std::vector<std::string> > m_3 ;
  std::vector<std::vector<double> >      m_4 ;

  std::map<int,double>                   m_5 ;
  std::map<std::string,std::string>      m_6 ;
  std::map<std::string,int>              m_7 ;
  std::map<std::string,double>           m_8 ;
  std::map<std::string,std::vector<std::string> > m_9 ;

  std::pair<int,int>  m_10 ;
  std::map<std::string,std::vector<double> > m_11 ;
  std::map<std::string,std::vector<int> >    m_12 ;

  std::map<int,int>    m_13 ;
  std::vector<std::pair<int,int> >  m_14 ;


  std::map<int,std::string>            m_15 ;
  std::map<unsigned int,std::string>   m_16 ;

  std::map<std::string, std::string>   m_20;
  std::vector<int> m_21;
};
// ============================================================================
/// factory
// ============================================================================
DECLARE_COMPONENT(ExtendedProperties)
// ============================================================================
namespace
{
  template <class TYPE>
  inline SimplePropertyRef<TYPE> _prop ( TYPE& value )
  {
    // construct a readable name
    std::string name = System::typeinfoName ( typeid( value ) ) ;
    std::string::size_type ipos = name.find("std::") ;
    while ( std::string::npos != ipos )
    {
      name.erase( ipos , 5 ) ;
      ipos = name.find("std::") ;
    }
    ipos = name.find(" ") ;
    while ( std::string::npos != ipos )
    {
      name.erase( ipos , 1 ) ;
      ipos = name.find(" ") ;
    }
    ipos = name.find("const") ;
    while ( std::string::npos != ipos )
    {
      name.erase( ipos , 5 ) ;
      ipos = name.find("const") ;
    }
    ipos = name.find(",allocator<") ;
    while( std::string::npos != ipos )
    {
      std::string::size_type ip2 = ipos + 11;
      int ip3 = 1 ;
      for ( ; ip2 < name.size() ; ++ip2 )
      {
        if ( '<' == name[ip2] ) { ip3+=1 ; }
        if ( '>' == name[ip2] ) { ip3-=1 ; }
        if ( 0 == ip3 ) { break ; }
      }
      name.erase( ipos , ip2 + 1 - ipos ) ;
      ipos = name.find(",allocator<") ;
    }
    if ( std::string::npos != name.find("map<") )
    {
      ipos = name.find(",less<") ;
      while( std::string::npos != ipos )
      {
        std::string::size_type ip2 = ipos + 6;
        int ip3 = 1 ;
        for ( ; ip2 < name.size() ; ++ip2 )
        {
          if ( '<' == name[ip2] ) { ip3+=1 ; }
          if ( '>' == name[ip2] ) { ip3-=1 ; }
          if ( 0 == ip3 ) { break ; }
        }
        name.erase( ipos , ip2 + 1 - ipos ) ;
        ipos = name.find(",less<") ;
      }
    }
    ipos = name.find(">>") ;
    while ( std::string::npos != ipos )
    {
      name.replace( ipos , 2 , "> >" ) ;
      ipos = name.find(">>") ;
    }
    return SimplePropertyRef<TYPE> ( name , value ) ;
  }
}
// ============================================================================
StatusCode ExtendedProperties::execute()
{
  always() << " My Properties : " << endmsg ;

  always () << " \t" << _prop ( m_1  ) << endmsg ;
  always () << " \t" << _prop ( m_2  ) << endmsg ;
  always () << " \t" << _prop ( m_3  ) << endmsg ;
  always () << " \t" << _prop ( m_4  ) << endmsg ;
  always () << " \t" << _prop ( m_5  ) << endmsg ;
  always () << " \t" << _prop ( m_6  ) << endmsg ;
  always () << " \t" << _prop ( m_7  ) << endmsg ;
  always () << " \t" << _prop ( m_8  ) << endmsg ;
  always () << " \t" << _prop ( m_9  ) << endmsg ;
  always () << " \t" << _prop ( m_10 ) << endmsg ;
  always () << " \t" << _prop ( m_11 ) << endmsg ;
  always () << " \t" << _prop ( m_12 ) << endmsg ;
  always () << " \t" << _prop ( m_13 ) << endmsg ;
  always () << " \t" << _prop ( m_14 ) << endmsg ;
  always () << " \t" << _prop ( m_14 ) << endmsg ;
  always () << " \t" << _prop ( m_15 ) << endmsg ;
  always () << " \t" << _prop ( m_16 ) << endmsg ;

  always () << " \t" << SimplePropertyRef<std::map<std::string, std::string> >( "EmptyMap", m_20 ) << endmsg ;
  always () << " \t" << SimplePropertyRef<std::vector<int> >( "EmptyVector", m_21 ) << endmsg ;

  // some properties could be created from other (convertible) types:
  SimpleProperty<short>     m1 ( "a" , 0  ) ;
  SimpleProperty<double>    m2 ( "b" , m1 ) ;

  // some properties could be assigned from other (convertible) types
  SimpleProperty<int>       m3 ( "c" , 0  ) ;
  m3 = m1 ;

  float i = 10 ;
  SimplePropertyRef<float> m4 ( "d" , i )  ;

  m4 = 12 ;


  return StatusCode::SUCCESS ;
}
// ============================================================================
// The END
// ============================================================================
