// ===========================================================================
// Include files
// ===========================================================================
// STD & STL
// ===========================================================================
#include <map>
// ===========================================================================
// GaudiKernel
// ===========================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/Map.h"
#include "GaudiKernel/HashMap.h"
// ===========================================================================
// GaudiAlg
// ===========================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ===========================================================================
namespace Gaudi
{
  // =========================================================================
  namespace Examples
  {
    // =======================================================================
    /** @class StringKeyEx   StringKeys/StirngKeyEx.cpp
     *  Simple example which illustrate the usage of class StringKey
     *  @see Gaudi::StringKey
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-07
     */
    class StringKeyEx : public GaudiAlgorithm
    {
    public:
      // ======================================================================
      /// execution of the algorithm
      virtual StatusCode execute () ;
      // ======================================================================
    public:
      // ======================================================================
      /** standard constructor
       *  @param name algorithm instance name
       *  @param pSvc pointer to Service Locator
       */
      StringKeyEx
      ( const std::string& name ,                //    algorithm instance name
        ISvcLocator*       pSvc )                // pointer to Service Locator
        : GaudiAlgorithm ( name , pSvc )
        , m_key  ()
        , m_keys ()
      {
        declareProperty ( "Key"  , m_key  , "The string key" ) ;
        declareProperty ( "Keys" , m_keys , "The vector of keys" ) ;
      }
      /// virtual & protected destructor
      virtual ~StringKeyEx () {}             // virtual & protected destructor
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled
      StringKeyEx () ;                    // the default constructor is disabled
      /// the copy  constructor is disabled
      StringKeyEx ( const StringKeyEx& ) ;  // the copy  constructor is disabled
      /// the assignment operator is disabled
      StringKeyEx& operator=( const StringKeyEx& ) ;          // no assignment
      // ======================================================================
    private:
      // ======================================================================
      typedef Gaudi::StringKey  Key  ;
      typedef std::vector<Key>  Keys ;
      // ======================================================================
      /// get the key ;
      Key  m_key  ;                                              // get the key
      /// get the keys
      Keys m_keys ;                                             // get the keys
      // ======================================================================
    } ;
    // ========================================================================
  } //                                         end of namespace Gaudi::Examples
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// Execution method
// ============================================================================
StatusCode Gaudi::Examples::StringKeyEx::execute()
{
  // 1. check the settings of key from the properties
  always() << "The Key  : " << Gaudi::Utils::toString ( m_key  ) << endmsg ;
  always() << "The Keys : " << Gaudi::Utils::toString ( m_keys ) << endmsg ;
  //

  // prepare some maps
  typedef std::map<std::string,int>              MAP1 ;
  typedef GaudiUtils::Map<std::string,int>       MAP2 ;
  typedef GaudiUtils::HashMap<std::string,int>   MAP3 ;
  typedef GaudiUtils::VectorMap<std::string,int> MAP4 ;

  typedef std::map<Key,int>                      MAP01 ;
  typedef GaudiUtils::Map<Key,int>               MAP02 ;
  typedef GaudiUtils::HashMap<Key,int>           MAP03 ;
  typedef GaudiUtils::VectorMap<Key,int>         MAP04 ;

  MAP1 map1 ;
  MAP2 map2 ;
  MAP3 map3 ;
  MAP4 map4 ;

  MAP01 map01 ;
  MAP02 map02 ;
  MAP03 map03 ;
  MAP04 map04 ;

  for ( Keys::const_iterator it = m_keys.begin() ; m_keys.end() != it ; ++it )
  {
    int index = it - m_keys.begin() ;

    map1.insert ( std::make_pair ( *it , index ) ) ;
    map2.insert ( std::make_pair ( *it , index ) ) ;
    map3.insert ( std::make_pair ( *it , index ) ) ;
    map4.insert ( std::make_pair ( *it , index ) ) ;

    map01.insert ( std::make_pair ( *it , index ) ) ;
    map02.insert ( std::make_pair ( *it , index ) ) ;
    map03.insert ( std::make_pair ( *it , index ) ) ;
    map04.insert ( std::make_pair ( *it , index ) ) ;

  }

  always() << "Map 1:" << Gaudi::Utils::toString ( map1  ) << endmsg ;
  always() << "Map 2:" << Gaudi::Utils::toString ( map2  ) << endmsg ;
  always() << "Map 3:" << Gaudi::Utils::toString ( map3  ) << endmsg ;
  always() << "Map 4:" << Gaudi::Utils::toString ( map4  ) << endmsg ;

  always() << "Map01:" << Gaudi::Utils::toString ( map01 ) << endmsg ;
  always() << "Map02:" << Gaudi::Utils::toString ( map02 ) << endmsg ;
  always() << "Map03:" << Gaudi::Utils::toString ( map03 ) << endmsg ;
  always() << "Map04:" << Gaudi::Utils::toString ( map04 ) << endmsg ;

  always() << "check for       StringKey " << Gaudi::Utils::toString ( m_key ) << endmsg ;

  always () << " In Map 1: " << Gaudi::Utils::toString ( map1.end() != map1.find ( m_key ) ) << endmsg ;
  always () << " In Map 2: " << Gaudi::Utils::toString ( map2.end() != map2.find ( m_key ) ) << endmsg ;
  always () << " In Map 3: " << Gaudi::Utils::toString ( map3.end() != map3.find ( m_key ) ) << endmsg ;
  always () << " In Map 4: " << Gaudi::Utils::toString ( map4.end() != map4.find ( m_key ) ) << endmsg ;

  always () << " In Map01: " << Gaudi::Utils::toString ( map01.end() != map01.find ( m_key ) ) << endmsg ;
  always () << " In Map02: " << Gaudi::Utils::toString ( map02.end() != map02.find ( m_key ) ) << endmsg ;
  always () << " In Map03: " << Gaudi::Utils::toString ( map03.end() != map03.find ( m_key ) ) << endmsg ;
  always () << " In Map04: " << Gaudi::Utils::toString ( map04.end() != map04.find ( m_key ) ) << endmsg ;

  std::string akey = "rrr" ;

  always() << "check for std::string key " << Gaudi::Utils::toString ( akey ) << endmsg ;

  always () << " In Map 1: " << Gaudi::Utils::toString ( map1.end() != map1.find ( akey ) ) << endmsg ;
  always () << " In Map 2: " << Gaudi::Utils::toString ( map2.end() != map2.find ( akey ) ) << endmsg ;
  always () << " In Map 3: " << Gaudi::Utils::toString ( map3.end() != map3.find ( akey ) ) << endmsg ;
  always () << " In Map 4: " << Gaudi::Utils::toString ( map4.end() != map4.find ( akey ) ) << endmsg ;

  always () << " In Map01: " << Gaudi::Utils::toString ( map01.end() != map01.find ( akey ) ) << endmsg ;
  always () << " In Map02: " << Gaudi::Utils::toString ( map02.end() != map02.find ( akey ) ) << endmsg ;
  always () << " In Map03: " << Gaudi::Utils::toString ( map03.end() != map03.find ( akey ) ) << endmsg ;
  always () << " In Map04: " << Gaudi::Utils::toString ( map04.end() != map04.find ( akey ) ) << endmsg ;



  return StatusCode::SUCCESS ;
}
// ============================================================================
/// the factory:
using Gaudi::Examples::StringKeyEx;
DECLARE_COMPONENT(StringKeyEx)
// ============================================================================
// The END
// ========-===================================================================
