#ifndef GAUDIKERNEL_VCTGRAMMARS_H 
#define GAUDIKERNEL_VCTGRAMMARS_H 1
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/Grammars.h"
// ============================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Point4DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"
#include "GaudiKernel/Vector4DTypes.h"
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Parsers
  {
    // ========================================================================
    using namespace boost::spirit ;
    // ========================================================================
    using namespace phoenix ;
    // ========================================================================
    /** @class Pnt3DGrammar 
     *  The grammar for 3D-point 
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    template <class ScalarGrammarT, class POINT>
    class Pnt3DGrammar : public grammar 
    <
      Pnt3DGrammar<ScalarGrammarT,POINT>, 
      typename ClosureGrammar<POINT>::context_t
    >
    {
    public:
      // ======================================================================
      /// the actual type of result 
      typedef POINT                                                  ResultT  ;
      typedef ClosureGrammar<POINT>                                  ClosureT ;
      // ======================================================================
    public:
      // ======================================================================
      /// callback. Action when we match inner value
      void matchX ( double value) const { this->val().SetX ( value ) ; }
      void matchY ( double value) const { this->val().SetY ( value ) ; }
      void matchZ ( double value) const { this->val().SetZ ( value ) ; }
      // ======================================================================
    public:
      // ======================================================================
      template <typename ScannerT>
      struct definition 
      {
        definition ( Pnt3DGrammar const & self ) 
        {
          
          inner = 
            !( ( str_p("x") | "X" | "pX" | "Px" | "PX" ) >> ':' )  >> 
            scalar [ boost::bind(&Pnt3DGrammar::matchX,&self,_1) ] >> 
            ',' >> 
            !( ( str_p("y") | "Y" | "pY" | "Py" | "PY" ) >> ':' )  >> 
            scalar [ boost::bind(&Pnt3DGrammar::matchY,&self,_1) ] >>
            ',' >> 
            !( ( str_p("z") | "Z" | "pZ" | "Pz" | "PZ" ) >> ':' )  >> 
            scalar [ boost::bind(&Pnt3DGrammar::matchZ,&self,_1) ] ;
          
          pnt = 
            ( str_p ("(") >> inner >> ')' ) | 
            ( str_p ("[") >> inner >> ']' ) ;

        }
        /// MANDATORY method:
        rule<ScannerT> const & start() const { return pnt ; }
        // rules: 
        rule<ScannerT>   pnt , inner ;
        ScalarGrammarT   scalar ;
        // ====================================================================
      };
      // ======================================================================      
    } ;
    // ========================================================================
    /** @class Pnt4DGrammar 
     *  The grammar for 4D-point 
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-05
     */
    template <class ScalarGrammarT, class POINT>
    class Pnt4DGrammar : public grammar 
    <
      Pnt4DGrammar<ScalarGrammarT,POINT>, 
      typename ClosureGrammar<POINT>::context_t
    >
    {
    public:
      // ======================================================================
      /// the actual type of result 
      typedef POINT                                                  ResultT  ;
      typedef ClosureGrammar<POINT>                                  ClosureT ;
      // ======================================================================
    public:
      // ======================================================================
      /// callback. Action when we match inner value
      void matchX ( double value) const { this->val().SetPx ( value ) ; }
      void matchY ( double value) const { this->val().SetPy ( value ) ; }
      void matchZ ( double value) const { this->val().SetPz ( value ) ; }
      void matchT ( double value) const { this->val().SetE  ( value ) ; }
      // ======================================================================
    public:
      // ======================================================================
      template <typename ScannerT>
      struct definition 
      {
        definition ( Pnt4DGrammar const & self ) 
        {
          
          x  = !( ( str_p("x") | "X" | "pX" | "Px" | "PX" ) && ':' )  
            >> scalar [ boost::bind(&Pnt4DGrammar::matchX,&self,_1) ]  ;
          y  = !( ( str_p("y") | "Y" | "pY" | "Py" | "PY" ) && ':' )  
            >> scalar [ boost::bind(&Pnt4DGrammar::matchY,&self,_1) ]  ;
          z  =  !( ( str_p("z") | "Z" | "pZ" | "Pz" | "PZ" ) && ':' ) 
            >> scalar [ boost::bind(&Pnt4DGrammar::matchZ,&self,_1) ]  ;
          e  =  !( ( str_p("t") | "T" | "e"  | "E"         ) && ':' ) 
            >> scalar [ boost::bind(&Pnt4DGrammar::matchT,&self,_1) ] ;
          
          p3 = x >> ',' >> y >> ',' >> z ;
          
          inner = 
            (                p3        >> ( str_p(',') | ';' ) >> e ) | 
            ( str_p("(") >>  p3 >> ')' >> ( str_p(',') | ';' ) >> e ) | 
            ( str_p("[") >>  p3 >> ']' >> ( str_p(',') | ';' ) >> e ) | 
            ( e >> ( str_p(",") | ';') >>  '(' >> p3 >> ')'         ) |  
            ( e >> ( str_p(",") | ';') >>  '[' >> p3 >> ']'         ) ;

          pnt = 
            ( str_p ("(") >>  inner >> ')' ) |
            ( str_p ("[") >>  inner >> ']' ) ;
          
        }
        /// MANDATORY method:
        rule<ScannerT> const & start() const { return pnt ; }
        // rules: 
        rule<ScannerT>   x,y,z,e,p3,inner, pnt ;
        ScalarGrammarT   scalar ;
        // ====================================================================
      };
      // ======================================================================      
    } ;
    // ========================================================================
  } //                                          end of namespace Gaudi::Parsers 
  // ==========================================================================
} //                                                     end of namespace Gaudi 
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIKERNEL_VCTGRAMMARS_H
// ============================================================================
