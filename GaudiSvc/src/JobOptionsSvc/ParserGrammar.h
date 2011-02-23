// $Id: ParserGrammar.h,v 1.10 2007/10/02 09:42:59 marcocle Exp $
// ============================================================================
#ifndef JOBOPTIONSSVC_PARSERGRAMMAR_H
#define JOBOPTIONSSVC_PARSERGRAMMAR_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <vector>
// ============================================================================
// Boost
// ============================================================================
#include <boost/version.hpp>
#if BOOST_VERSION >= 103800
// FIXME: Move to the new boost::spirit::classic namespace
#if !defined(BOOST_SPIRIT_USE_OLD_NAMESPACE)
#define BOOST_SPIRIT_USE_OLD_NAMESPACE
#endif
#include <boost/spirit/include/classic.hpp>
#else
#include <boost/spirit.hpp>
#endif
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Grammars.h"
#include "GaudiKernel/ToStream.h"
// ============================================================================
// Local
// ============================================================================
#include "ParserUtils.h"
#include "ParserActions.h"
// ============================================================================
namespace Gaudi
{
  namespace Parsers
  {
    // ========================================================================
    using namespace boost::spirit ;
    using namespace phoenix;
    // ========================================================================
    typedef boost::spirit::position_iterator<std::string::const_iterator>
    IteratorT;
    // ========================================================================
    /** @class IdentifierGrammar
     *  Recognize alphanumeric strings and _ starting with alpha:
     *  Example, abc, ab_cd123, a_12
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    class IdentifierGrammar : public grammar
    <
      IdentifierGrammar, ClosureGrammar<std::string>::context_t
    >
    {
    public:
      typedef std::string ResultT;
    public:
      template <typename ScannerT>
      struct definition
      {
        definition( IdentifierGrammar const &self )
        {
          //-----------------------------------------------------------------
          // KEYWORDS
          //-----------------------------------------------------------------
          keywords =
            "#include" ,
            "#units"   ,
            "#ifdef"   ,
            "#ifndef"  ,
            "#endif"   ,
            "#else"    ,
            "WIN32"    ;
          identifier
            = (lexeme_d[ (alpha_p >> *(alnum_p | '_'))
                         - (keywords >>
                            (anychar_p - (alnum_p | '_')))])
            [self.val = construct_<std::string>(arg1,arg2)];
        }
        symbols<> keywords;
        rule<ScannerT> const& start() const
        { return identifier; }
        rule<ScannerT> identifier;
      };
    };
    // ========================================================================
    /** @class PropertyGrammar
     *  Recognize property
     *  Example, A.B, A::B::C.D, A::B.C::D.E
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2007-09-24
     */
    class PropertyGrammar: public grammar
    	<
    		PropertyGrammar,
    		ClosureGrammar<std::vector<std::string> >::context_t
    	>
    {
    	public:
          typedef std::vector<std::string> ResultT;
    	public:
    	      template <typename ScannerT>
    	      struct definition
    	      {
    	        definition(PropertyGrammar const &self )
    	        {
    				ns = (ident >> *("::">>ident))[ns.val = construct_<std::string>(arg1,arg2)]>>".";
    				property = +(ns[PushBack(self.val)]) >> ident[PushBack(self.val)];

    	        }
    	        IdentifierGrammar ident;
    	        rule<ScannerT> const& start() const
    	        { return property; }
    	        rule<ScannerT,ClosureGrammar<std::string>::context_t>  ns;
    	        rule<ScannerT> property;
    	      };
    };
    // ========================================================================
    /** @class RealUnitsGrammar
     *
     *  The valid represenation of reals with units are
     *  - 1, 1.0, 1.0e+2, 1e-2, 0.5mm, 0.5 mm, 0.5*mm
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    class RealUnitsGrammar : public boost::spirit::grammar
    < RealUnitsGrammar,AttributesClosureGrammar<std::string,
                             boost::tuple<IteratorT, long double> >::context_t >
    {
    public:
      typedef long double ResultT;
      RealUnitsGrammar():m_parser(NULL){}
    public:
      template <typename ScannerT>
      struct definition
      {
        definition(RealUnitsGrammar const &self)
        {
          real_literal
            = (
               longest_d[grInt[boost::bind(&RealUnitsGrammar::matchReal,&self,_1)]
                |grReal[boost::bind(&RealUnitsGrammar::matchReal,&self,_1)]]
               >> eps_p[boost::bind(&RealUnitsGrammar::matchPosition,&self,_2)]
               >>!(!ch_p('*')
               >>grUnit
               [boost::bind(&RealUnitsGrammar::matchUnit,&self,_1)]
               )) [boost::bind(&RealUnitsGrammar::matchRealUnits,&self)];
        }
        RealGrammar<long double> grReal;
        IntGrammar<int> grInt;
        IdentifierGrammar grUnit;
        boost::spirit::rule<ScannerT> const& start() const
        { return real_literal; }
        boost::spirit::rule<ScannerT> real_literal;
      };
      // Action when we recognize real with unit
      void matchRealUnits() const
      {
        val() = Gaudi::Utils::toString ( attrs().get<1>() );
      }

      // Action when we recognize real (without unit)
      void matchReal(ResultT value) const
      { attrs().get<1>()=value; }

      // Action for saving position
      void matchPosition(IteratorT value) const
      { attrs().get<0>()=value; }

      // Action when we recognize unit
      void matchUnit(const std::string& unit) const
      {
        if (NULL == m_parser) return;
        file_position fpos = attrs().get<0>().get_position();
        Position pos(fpos.file,fpos.line,fpos.column);

        attrs().get<1>() *= m_parser->matchUnit(unit,pos);
      }

      /** Set up pointer to Parser
       *  @param parser Pouinter to parser
       */
      void setParser(Parser* parser){m_parser = parser;}
    private:
      Parser* m_parser;
    };
    // ========================================================================
    /** @class UnitsFileGrammar
     *
     *  The valid represenation units file:
     *  <c>
     *  1 cm = 10
     *  1 m = 1000
     *  </c>
     *
     *  @see Gaudi::Parsers::RealUnitsGrammar
     *  @see Gaudi::Parsers::IdentifierGrammar
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    class UnitsFileGrammar : public boost::spirit::grammar
    <
      UnitsFileGrammar, AttributesClosureGrammar<IteratorT,
                             boost::tuple<std::string,double> >::context_t >
    {
    public:
      /** Constructor
       *
       *  @param parser Pointer to parser object
       */
      UnitsFileGrammar(Parser* parser)
        : m_parser(parser) {}
    public:
      /// Action called when matched unit definition
      void matchUnit() const
      {
        file_position fpos = val().get_position();
        m_parser->matchUnitEntry
          (attrs().get<0>(),attrs().get<1>(),
           Position(fpos.file,fpos.line,fpos.column));
        attrs().get<1>() = 1;
      }
      /** Action called when number matched in left part of definition
       *
       *  @param real Matched number
       */
      void matchLeftReal(double real) const
      { attrs().get<1>() = real; }

      /** Action called when new unit name matched
       *
       *  @param newunit Matched name
       */
      void matchLeftUnit(std::string newunit) const
      { attrs().get<0>() = newunit; }

      /** Action called when right part number (possible with unit) matched
       *
       *  @param value String representation of number
       */
      void matchRight(std::string value) const
      { attrs().get<1>() =
          boost::lexical_cast<double>(value)/attrs().get<1>(); }
    public:
      template <typename ScannerT>
      struct definition
      {
        definition(UnitsFileGrammar const &self)
        {
          boost::tuples::get<1>(self.attrs()) = 1;
          grUnit.setParser(self.parser());
          units_file = *(unit[boost::bind(&UnitsFileGrammar::matchUnit,&self)]);
          unit =
            (real_p[boost::bind(&UnitsFileGrammar::matchLeftReal,&self,_1)]
             >> eps_p)[self.val=arg1] >> !ch_p('*')
             >>grNewUnit[boost::bind(&UnitsFileGrammar::matchLeftUnit,&self,_1)]
             >>"=">>grUnit[boost::bind(&UnitsFileGrammar::matchRight,&self,_1)];
        }
        RealUnitsGrammar grUnit;
        IdentifierGrammar grNewUnit;
        boost::spirit::rule<ScannerT> const& start() const
        { return units_file;}
        boost::spirit::rule<ScannerT> units_file,unit;
      };
    public:
      /// Pointer to parser
      Parser* parser() const{ return m_parser;}
    private:
      Parser* m_parser;
    };
    // ========================================================================
    typedef AttributesClosureGrammar
    <boost::tuple<std::string,std::vector<std::string> > ,
     boost::tuple<int> > ValueClosureT;
    // ========================================================================
    /** @class ValueGrammar
     *
     *  Grammar recognize value that can appear in right side of '=' operator
     *  in job options file.
     *
     *  Grammar's EBNF
     *  <c>
     *  value ::= vectorvalue | vector_type | property_link.
     *  vectorvalue ::= pairvalue | literalvalue | mapvalue.
     *  literalvalue ::= realunits_grammar | boolean_grammar | string_grammar.
     *  mapvalue ::= literalvalue ('=' | ':') value.
     *  pairvalue ::= '(' vectorvalue ',' vectorvalue ')'.
     *  vector_type ::= ('{' vectorvalue_list '}') | ('[' vectorvalue_list ']').
     *  vectorvalue_list ::= (vector_value (',' vector_value)*)?.
     *  property_link = '@' identifier_grammar ('.' identifier_grammar)+.
     *  </c>
     *
     *  Grammar has two attributes
     *  - boost::tuple<std::string,std::vector<std::string>
     *    0 element contains string representation of top value
     *    1 element contains vector representation of value (if it's vector)
     *  - boost::tuple<int> nested level of vector
     *
     *  @see Gaudi::Parsers::RealUnitsGrammar for realunits_grammar
     *  @see Gaudi::Parsers::BoolGrammar for boolean_grammar
     *  @see Gaudi::Parsers::StringGrammar for string_grammar
     *  @see Gaudi::Parsers::IdentifierGrammar for identifier_grammar
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    class ValueGrammar : public boost::spirit::grammar
    <ValueGrammar, ValueClosureT::context_t>
    {
    public:
      ValueGrammar() : m_parser(NULL){}
    public:
      /** Action called when brace is matched
       *
       *  @param isopen true - if this is '{' or '[' brase, false - '}' or ']'
       */
      void matchBrace(bool isopen) const
      {
        if ( isopen ){ attrs().get<0>()++; }
        else         { attrs().get<0>()--; }
      }
      /** Action called when vector value matched
       *
       *  @param value string representation of value
       */
      void matchVectorValue(std::string value) const
      { if(attrs().get<0>()==1){val().get<1>().push_back(value); } }

      /** Action called when right side of '=' top value is matched
       *
       *  @param value string representation of value
       */
      void matchValue(std::string value) const{ val().get<0>() = value; }

      /// Pointer to parser
      Parser* parser() const { return m_parser;}

      /// Set poinet to parser
      void setParser(Parser* parser){ m_parser = parser; }

    public:

      template <typename ScannerT>
      struct definition
      {
        definition(ValueGrammar const &self)
        {
          realunits_grammar.setParser(self.parser());
          //-----------------------------------------------------------------
          // OPERATORS
          //-----------------------------------------------------------------
          chlit<>     O_DOT('.');
          chlit<>     O_COMMA(',');
          value_rule = value[boost::bind(&ValueGrammar::matchValue,&self,_1)];
          value =
            vectorvalue[value.val=arg1]
            | vector_type[value.val="["+arg1+"]"]
            | property_link[value.val=arg1]
            ;
          // ------------------------------------------------------------------
          vectorvalue_list
            = !(
                value[vectorvalue_list.val=arg1][boost::bind(&ValueGrammar::matchVectorValue,&self,_1)]
                >> *(',' >>value[vectorvalue_list.val+=","+arg1][boost::bind(&ValueGrammar::matchVectorValue,&self,_1)])
                );
          // ------------------------------------------------------------------
          vector_type =
            (
             ch_p('{')[boost::bind(&ValueGrammar::matchBrace,&self,true)]
             >> vectorvalue_list
             [vector_type.val=arg1]
             >> ch_p('}')[boost::bind(&ValueGrammar::matchBrace,&self,false)]
             )
            |
            (ch_p('[') [boost::bind(&ValueGrammar::matchBrace,&self,true)]
             >> vectorvalue_list
             [vector_type.val=arg1]
             >> ch_p(']')[boost::bind(&ValueGrammar::matchBrace,&self,false)]
             );
          // ------------------------------------------------------------------
          vectorvalue =
            listvalue[vectorvalue.val=arg1]
            | longest_d[
                        literalvalue[vectorvalue.val=arg1]
                        | mapvalue[vectorvalue.val=arg1]
            ];
          // ------------------------------------------------------------------
          listvalue = '(' >> vectorvalue[listvalue.val="("+arg1]
                          >> *(O_COMMA[listvalue.val+=","]
                          >> vectorvalue[listvalue.val+=arg1]) >>ch_p(')')[listvalue.val+=")"];
          // ------------------------------------------------------------------
          mapvalue = literalvalue[mapvalue.val=arg1]
            >> (ch_p('=')[mapvalue.val+="="] | ch_p(':')[mapvalue.val+=":"])
            >> value[mapvalue.val+=arg1]
            ;
          // ------------------------------------------------------------------
          literalvalue =
            realunits_grammar[literalvalue.val = arg1]
            |
            boolean_grammar[AssignBoolToString(literalvalue.val)]
            |
            string_grammar[literalvalue.val = std::string("\"")+arg1+std::string("\"")];
          // ------------------------------------------------------------------
          property_link = ('@'>>property_grammar)
            [property_link.val=construct_<std::string>(arg1,arg2)];
          // ------------------------------------------------------------------
        }
        PropertyGrammar   property_grammar;
        StringGrammar     string_grammar;
        BoolGrammar       boolean_grammar;
        IntGrammar<long>  int_grammar;
        RealUnitsGrammar  realunits_grammar;

        boost::spirit::rule<ScannerT> value_rule;
        boost::spirit::rule
        <ScannerT,ClosureGrammar<std::string>::context_t> value,literalvalue,
          vectorvalue,listvalue,mapvalue,vectorvalue_list, vector_type,
          property_link;
        boost::spirit::rule<ScannerT> const& start() const { return value_rule; }
      };
      // ============================================================================
    private:
      Parser* m_parser;
    };
    // ========================================================================
    typedef AttributesClosureGrammar
    <IteratorT,boost::tuple<bool,std::string,std::vector<std::string> > > ParserClosureT;
    // ========================================================================
    /** @class ParserGrammar
     *
     *  Grammar recognize job options file
     *  <c>
     *  Grammar's EBNF:
     *  <c>
     *  job_options_file ::= (platform_statement | platform_dependency)*.
     *  platform_statement ::= assertable_statement | pragma_statement.
     *  assertable_statement ::= include_statement | units_statement |
     *               assign_statement.
     *  platform_dependency ::= ("#ifdef" | "#ifndef")
     *    "WIN32" platform_statement* ("#else" platform_statement*)? "#endif.
     *  include_statement ::= "#include" string_grammar.
     *  units_statement ::= "#units" string_grammar.
     *  pragma_statement ::= pragma | printopt_statement;
     *  printopt_statement ::= "#printOptions" "full".
     *  pragma ::= "#pragma" "print" ("on" | "ON" | "off" | "OFF").
     *  assign_statement ::=
     *    identifier_grammar ('.' identifier_grammar)+
     *      ('='|'+='|'-=') value_grammar ';'.
     *
     *  Grammar has two attributes
     *  - IteratorT
     *      current parser position.
     *  - boost::tuple<bool,string,vector<string> >
     *    0 element - true if we can process platform statements, false - if
     *                we can't process.
     *    1 element - string representation of value on the right side of '='
     *    2 element - vector representation of value on the right side of '='
     *                (if it's vector)
     *
     *  @see Gaudi::Parsers::ValueGrammar for value_grammar
     *  @see Gaudi::Parsers::StringGrammar for string_grammar
     *  @see Gaudi::Parsers::IdentifierGrammar for identifier_grammar
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    class ParserGrammar :
      public grammar<ParserGrammar, ParserClosureT::context_t> {
    public:
      /** Constructor
       *
       *  @param parser Pointer to parser
       */
      ParserGrammar(Parser* parser){m_parser=parser;}
      virtual ~ParserGrammar() {}

      /// Get pointer to parser
      Parser* parser() const {return m_parser;}

      /** Action called when include statement matched
       *
       *  @param fileName Name of file in include statement
       */
      void matchInclude(const std::string& fileName) const{
        if(!doActions()) return;
        file_position fpos = this->val().get_position();
        Position pos(fpos.file,fpos.line,fpos.column);
        m_parser->matchInclude(fileName,pos);
      }
      /** Action called when units statement matched
       *  @param fileName Name of file in units statement
       */
      void matchUnits(const std::string& fileName) const{
        if(!doActions()) return;
        file_position fpos = this->val().get_position();
        Position pos(fpos.file,fpos.line,fpos.column);
        m_parser->matchUnits(fileName,pos);
      }
      /** Match assign statement
       *
       *  @param params[0]-params[N-3]  Name of object
       *  @param params[N-2]  Delimiter
       *  @param params[N-1]  Option name
       *  @param params[N]  Operation
       */
      void matchAssign(std::vector<std::string> params) const{
        if(!doActions()) return;

        // position where statement appears
        file_position fpos = val().get_position();
        Position pos(fpos.file,fpos.line,fpos.column);

        // get value representation. It's two modes string and vector of strings
        std::string value = attrs().get<1>();
        std::vector<std::string> vectorValues = attrs().get<2>();

        // check if value is vector
        bool isVector = false;
        if(vectorValues.size()>0 || value=="[]"){
          isVector = true;
        }else{
          vectorValues.push_back(value);
        }

        // extract object, option name and sign
        std::string objectName;
        std::string delim;
        std::vector< std::string >::const_iterator cur;
        for ( cur  = params.begin();
              *(cur+1) != "#"; cur++ ){
          objectName += delim + *cur;
          delim = ".";
        }

        std::string optionName = *cur;
        std::string sign = *(cur+2);

        Parser::Sign oper;
        if(sign == "+="){
          oper = Parser::S_PLUSASSIGN;
        }else if(sign == "-="){
          oper = Parser::S_MINUSASSIGN;
        }else{
          oper = Parser::S_ASSIGN;
        }

        m_parser->matchAssign
          (objectName, optionName, oper,
           vectorValues,pos,isVector);
        vectorValues.clear();
      }
      // ======================================================================
      /** Action called when platform dependency statement matched
       *
       *  @param directive - name of directive
       */
      void matchPlatform(const std::string& directive) const{
        bool iswin = Gaudi::Parsers::Utils::isWin();
        if(directive=="endif" || (directive=="ifdef" && iswin)
          || (directive=="ifndef" && !iswin)){
          attrs().get<0>() = true;
        } else {
          attrs().get<0>() = !attrs().get<0>();
        }
      }
      // ======================================================================
      /** Action called when print pragma matched
       *
       *  @param on true - printing is on, false - printing is off
       */
      void matchPrint(bool on) const{
        file_position fpos = this->val().get_position();
        Position pos(fpos.file,fpos.line,fpos.column);
        m_parser->setIsPrint(on,pos);
      }
      // ======================================================================
      /** Action called when printOptions
       *
       *  @param on true - printing is on, false - printing is off
       */
      void matchPrintOptions(bool on) const{
        file_position fpos = this->val().get_position();
        Position pos(fpos.file,fpos.line,fpos.column);
        m_parser->setIsPrintOptions(on,pos);
      }
      // ======================================================================
      /** Action called when value in assign statement matched
       *
       *  @param value Tuple of string and vector representation of value
       */
      void matchValue(boost::tuple<std::string,std::vector<std::string> > value) const{
        if(!doActions()) return;
        attrs().get<1>() = value.get<0>();
        attrs().get<2>() = value.get<1>();
      }
      // ======================================================================
      /** Check if we can execute platform statements
       *  @return true - we can execute platform statements, false - we can't.
       */
      bool doActions() const{ return  attrs().get<0>();}
      // ======================================================================
      template <typename ScannerT>
      struct definition {

        definition(ParserGrammar const &self) {
          value_grammar.setParser(self.parser());
          boost::tuples::get<0>(self.attrs()) = true;
          // ------------------------------------------------------------------
          // OPERATORS
          // ------------------------------------------------------------------
          chlit<>     O_SEMI(';');
          chlit<>     O_DOT('.');

          strlit<>    O_ASSIGN("=");
          strlit<>    O_PLUSASSIGN("+=");
          strlit<>    O_MINUSASSIGN("-=");

          // ------------------------------------------------------------------
          // TOKENS
          // ------------------------------------------------------------------
					strlit<> T_SHELL("#!");
          strlit<> T_IFDEF("#ifdef");
          strlit<> T_IFNDEF("#ifndef");
          strlit<> T_ELSE("#else");
          strlit<> T_ENDIF("#endif");
          strlit<> T_WIN32("WIN32");
          strlit<> T_INCLUDE("#include");
          strlit<> T_UNITS("#units");
          strlit<> T_PRAGMA("#pragma");
          strlit<> T_PRINTOPTIONS("#printOptions");
          // ------------------------------------------------------------------
          job_options_file =
            !shell_statement >> *(platform_statement | platform_dependency);
          // ------------------------------------------------------------------
					shell_statement = comment_p(T_SHELL);
          // ------------------------------------------------------------------
			    platform_statement = assertable_statement | pragma_statement;
          // ------------------------------------------------------------------
          assertable_statement =
            include_statement
            [boost::bind(&ParserGrammar::matchInclude,&self,_1)]
            | units_statement
            [boost::bind(&ParserGrammar::matchUnits,&self,_1)]
            | assign_statement
            [boost::bind(&ParserGrammar::matchAssign,&self,_1)];
          // ------------------------------------------------------------------
          assertion_statement =
            T_IFDEF
            [boost::bind(&ParserGrammar::matchPlatform,&self,"ifdef")]
            |
            T_IFNDEF
            [boost::bind(&ParserGrammar::matchPlatform,&self,"ifndef")];
          // ------------------------------------------------------------------
          platform_dependency =
            assertion_statement
              >> T_WIN32
              >> *platform_statement
              >> !(
                   T_ELSE[boost::bind(&ParserGrammar::matchPlatform,&self,"else")]
                   >> *platform_statement
                   )
              >> T_ENDIF
            [boost::bind(&ParserGrammar::matchPlatform,&self,"endif")];
          // ------------------------------------------------------------------
          include_statement =
            T_INCLUDE
              >> (string_grammar[include_statement.val=arg1]>>eps_p)
            [self.val=arg1];
          // ------------------------------------------------------------------
          units_statement =
            T_UNITS
              >> (string_grammar[units_statement.val=arg1]>>eps_p)
            [self.val=arg1];;
          // ------------------------------------------------------------------
          pragma_statement = pragma | printopt_statement;
          // ------------------------------------------------------------------
          pragma = (T_PRAGMA>>eps_p)[self.val=arg1] >> str_p("print")
                            >> ((str_p("on") | str_p("ON"))
                                [boost::bind(&ParserGrammar::matchPrint,&self,true)]
                                | (str_p("off") | str_p("OFF"))
                                [boost::bind(&ParserGrammar::matchPrint,&self,false)]) ;
          // ------------------------------------------------------------------
          printopt_statement = ((T_PRINTOPTIONS>>eps_p)[self.val = arg1] >>
            !as_lower_d["full"])
            [boost::bind(&ParserGrammar::matchPrintOptions,&self,true)];
          // ------------------------------------------------------------------
          assign_statement =
        	  property_grammar[assign_statement.val=arg1]
              >> eps_p[PushBack(assign_statement.val,"#")]
              >> (
                  O_ASSIGN[PushBack(assign_statement.val,"=")]
                  |O_PLUSASSIGN[PushBack(assign_statement.val,"+=")]
                  |O_MINUSASSIGN[PushBack(assign_statement.val,"-=")]
                  )
              >>  value_grammar[boost::bind(&ParserGrammar::matchValue,&self,_1)]
              >> O_SEMI;
          // ------------------------------------------------------------------
        }
        // --------------------------------------------------------------------
        PropertyGrammar property_grammar;
        IdentifierGrammar identifier_grammar;
        StringGrammar string_grammar;
        ValueGrammar value_grammar;
        // --------------------------------------------------------------------
        rule<ScannerT,ClosureGrammar<std::string>::context_t>  include_statement,
          units_statement;

        rule<ScannerT,ClosureGrammar<std::vector<std::string> >::context_t> assign_statement;

        rule<ScannerT>
        job_options_file,
					shell_statement,
          platform_statement,
          assertable_statement,
          assertion_statement,
          platform_dependency,
          pragma_statement,
          pragma,
          printopt_statement;
        // --------------------------------------------------------------------
        rule<ScannerT> const& start() const {
          return job_options_file;
        }
        // --------------------------------------------------------------------
      };
      // ======================================================================
    private:
      Parser* m_parser;
    };
  } // end of namespace Parsers
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // JOBOPTIONSSVC_PARSERGRAMMAR_H
// ============================================================================

