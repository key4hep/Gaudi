#include "GaudiKernel/ParsersFactory.h"
#include "GaudiMath/Interpolation.h"
#include <iomanip>
#include <cassert>

namespace {
  const char* table[] =  { "Linear",  "Polynomial"       ,
                           "Cspline", "Cspline_Periodic" ,
                           "Akima",   "Akima_Periodic"   };
}

using namespace GaudiMath::Interpolation;

namespace Gaudi { namespace Parsers {

template< typename Iterator, typename Skipper>
struct InterpolGrammar : qi::grammar<Iterator, GaudiMath::Interpolation::Type(), Skipper>
{
    typedef GaudiMath::Interpolation::Type  ResultT;
    InterpolGrammar() : InterpolGrammar::base_type( literal ) {
        literal = (qi::lit(table[Linear]))[qi::_val=Linear]
                | (qi::lit(table[Polynomial]))[qi::_val=Polynomial]
                | (qi::lit(table[Cspline]))[qi::_val=Cspline]
                | (qi::lit(table[Cspline_Periodic]))[qi::_val=Cspline_Periodic]
                | (qi::lit(table[Akima]))[qi::_val=Akima]
                | (qi::lit(table[Akima_Periodic]))[qi::_val=Akima_Periodic];
    }
    qi::rule<Iterator, ResultT(), Skipper> literal;
};
REGISTER_GRAMMAR(GaudiMath::Interpolation::Type, InterpolGrammar);

} }

namespace GaudiMath {
namespace Interpolation {

     StatusCode parse(Type& t, const std::string& in) {
         return Gaudi::Parsers::parse_(t,in);
     }
     std::ostream& toStream(const Type& t, std::ostream& os) {
         assert(0<=t && t<=Type::Akima_Periodic);
         return os << std::quoted( table[t], '\'' );
     }

} }
