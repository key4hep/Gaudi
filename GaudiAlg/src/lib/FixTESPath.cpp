#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/MsgStream.h"
#include "boost/tokenizer.hpp"
#include <memory>
#include <numeric>
#include <string>
#include <string_view>

namespace {
  // update DataHandles to point to full TES location
  //
  class DHHFixer : public IDataHandleVisitor {
    std::function<std::string( std::string_view )> m_f;

  public:
    DHHFixer( std::function<std::string( std::string_view )> f ) : m_f( std::move( f ) ) {}

    void visit( const IDataHandleHolder* idhh ) override {
      if ( !idhh ) return;

      std::string r;
      for ( auto h : idhh->inputHandles() ) {
        r = m_f( h->objKey() );
        if ( r != h->objKey() ) h->updateKey( r );
      }
      for ( auto h : idhh->outputHandles() ) {
        r = m_f( h->objKey() );
        if ( r != h->objKey() ) h->updateKey( r );
      }
    }
  };

} // namespace

namespace FixTESPathDetails {

  // ============================================================================
  // Returns the full correct event location given the rootInTes settings
  // ============================================================================
  std::string fullTESLocation( std::string_view location, std::string_view rit ) {
    // The logic is:
    // if no R.I.T., give back location
    // if R.I.T., this is the mapping:
    // (note that R.I.T. contains a trailing '/')
    //  location       -> result
    //  -------------------------------------------------
    //  ""             -> R.I.T.[:-1]      ("rit")
    //  "/Event"       -> R.I.T.[:-1]      ("rit")
    //  "/Event/MyObj" -> R.I.T. + "MyObj" ("rit/MyObj")
    //  "MyObj"        -> R.I.T. + "MyObj" ("rit/MyObj")
    return rit.empty()
               ? std::string{location}
               : location.empty() || ( location == "/Event" )
                     ? std::string{rit.substr( 0, rit.size() - 1 )}
                     : location.compare( 0, 7, "/Event/" ) == 0
                           ? std::string{rit}.append( location.substr( 7 ) )
                           : location.compare( 0, 1, "/" ) == 0 ? std::string{rit}.append( location.substr( 1 ) )
                                                                : std::string{rit}.append( location );
  }

  std::unique_ptr<IDataHandleVisitor> fixDataHandlePath( std::string_view rit, std::string rootName, MsgStream* dbg ) {
    if ( !rootName.empty() && '/' != rootName.back() ) rootName += "/";
    return std::make_unique<DHHFixer>( [rit, rootName = std::move( rootName ), dbg]( std::string_view location ) {
      auto tokens = boost::tokenizer<boost::char_separator<char>>{location, boost::char_separator<char>{":"}};
      auto result =
          std::accumulate( tokens.begin(), tokens.end(), std::string{}, [&]( std::string s, std::string_view tok ) {
            std::string r = fullTESLocation( tok, rit );
            // check whether we have an absolute path if yes use it - else prepend DataManager Root
            if ( r[0] != '/' ) r = rootName + r;
            return s.empty() ? r : s + ':' + r;
          } );
      if ( result != location && dbg ) ( *dbg ) << "Changing " << location << " to " << result << endmsg;
      return result;
    } );
  }
} // namespace FixTESPathDetails
