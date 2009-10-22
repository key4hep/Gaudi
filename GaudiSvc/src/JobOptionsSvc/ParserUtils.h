// $Id: ParserUtils.h,v 1.7 2007/09/25 08:02:11 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $ 
// ============================================================================
#ifndef JOBOPTIONSSVC_PARSERUTILS_H 
#define JOBOPTIONSSVC_PARSERUTILS_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <string>
#include <vector>
#include <map>
#include <iostream>
// ============================================================================
// local
// ============================================================================
#include "Catalogue.h"
#include "Position.h"
// ============================================================================

/** @file
 *  Collection of technical internal classes to implement JobOptionsSvc 
 *
 *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-05-13
 */
// ============================================================================

namespace Gaudi
{
  namespace Parsers 
  {
    /** @class  Message ParserUtils.h
     *
     *  Parser message interface
     *
     *  @author  Alexander Mazurov
     *  @date  2006-02-17 
     */
    class Message 
    {
    public:
      enum Severity
        {
          E_ERROR   = 1 ,
          E_WARNING = 2 ,
          E_NOTICE  = 3 , 
          E_VERBOSE = 4
        };
      enum Code
        {	
          C_OK                         = 0 ,
          C_FILENOTFOUND               = 1 ,
          C_FILENOTOPENED              = 2 ,
          C_SYNTAXERROR                = 3 ,
          C_UNITNOTFOUND               = 4 ,
          C_PROPERTYNOTFOUND           = 5 ,
          C_CANNOTADDTONOTVECTOR       = 6 ,
          C_CANNOTREMOVEFROMNOTVECTOR  = 7 ,
          C_ZEROREMOVED                = 8 ,
          C_BADREFERENCE               = 9  
        };
    public:
      /** Standard creator
       *  @param severity Message severity 
       *  @param code Code of message
       *  @param message Message string
       */
      Message
      ( const Severity& severity, 
        const Code& code, 
        const std::string& message) 
        : m_severity ( severity )
        , m_code     ( code     )
        , m_message  ( message  )
      {};
      /// Message itself 
      const std::string& message(void) const{return m_message;}
      /// Severity of message 
      const Severity& severity(void) const{return m_severity;}
      /// Code of message
      const Code& code(void) const{return m_code;};
    private:
      Severity    m_severity ;
      Code        m_code     ;
      std::string m_message  ;
    };
    
    
    /** @class  Parser ParserUtils.h
     *
     *  Parser controller
     *
     *  @author  Alexander Mazurov
     *  @date  2006-02-17 
     */
    class Parser
    {
    public:
      typedef std::vector<Gaudi::Parsers::Message> MessagesStoreT;
      typedef std::map<std::string,double> UnitsStoreT;
      enum Sign
        {
          S_ASSIGN      = 0 ,
          S_PLUSASSIGN  = 1 ,
          S_MINUSASSIGN = 2
        };
      /// Default creator
      Parser (Catalogue& catalogue, std::vector<std::string>& included, std::ostream& m = std::cout ) ;
      /// Constructor from directories list for search files in
      Parser (Catalogue& catalogue, std::vector<std::string>& included, 
				const std::vector<std::string>& searchPath , 
				std::ostream& m = std::cout ) ;
      /// Constructor from directories list for search files in
      Parser (Catalogue& catalogue, std::vector<std::string>& included, 
				const std::string& searchPath ,
				std::ostream& m = std::cout  ) ;
    public:
      /** Start parse file
       *  @return StatusCode::SUCCESS if no errors
       *  @param fileName File to parse
       */
      StatusCode parse(const std::string& fileName);
      /// Return parser messages
      const std::vector<Message>& messages(){return m_messages;}
      /// Count of errors
      int errorsCount();
      UnitsStoreT& units(void) {return m_units;}
      /** Do actions when include file is matched 
       *  @param fileName File to include
       **/
      void matchInclude
      (const std::string& fileName,const Position& pos);
      /** Do actions when units file is matched 
       *  @param fileName File to include
       **/
      void matchUnits
      (const std::string& fileName,const Position& pos);
      /** Do actions when we match unit
       *  @return Value of unit (if unit recognized), 1 if not recognized
       *  @param unit Unit name
       **/
      long double matchUnit
      (const std::string& unit,const Position& pos);
      /** Do actions when we match unit entry in units file
       *  @param newUnit New unit 
       *  @param value value of new unit
       *  @param pos Position of entry
       **/
      void matchUnitEntry
      (const std::string& newUnit, 
       double value,
       const Position& pos);
      /** Do actions when we match property
       *  @param objName Object name
       *  @param propName Property name
       *  @param oper Operation ("=","+=","-=")
       *  @param value String value
       *  @param vectorValues Vector values
       **/
      void matchAssign
      (const std::string& objName,
       const std::string& propName,
       const Sign& oper, 
       const std::vector<std::string>& vectorValues,
       const Position& pos, bool isVector);
      /// Control printing
      void setIsPrint(bool on, const Position& pos);
      /// Control printing options
      void setIsPrintOptions(bool on, const Position& pos);
    private:
      bool m_isPrint;
      bool m_isPrintOptions;
      std::vector<std::string> m_searchPath;
      Catalogue&                m_catalogue;
      std::vector<std::string>& m_included;    
      MessagesStoreT m_messages;
      UnitsStoreT    m_units;
      std::ostream&  m_stream ;
    private:
      /// Init units
      void initUnits();
      /** Parse file from another file
       *  @return StatusCode::SUCCESS if no *syntax* errors appears
       *  @param fileName File to parse
       *  @pos Information about from what file we call parsing of file
       */
      StatusCode parseFile
      ( const std::string& fileName ,
        const Position& pos,
        bool isUnitFile=false);
      /// Resolve references
      void resolveReferences();
      /** Add message
       *  @param severity Message severity
       *  @param code Code of message
       *  @param message Message
       *  @param pos File position
       **/
      void addMessage
      ( const Message::Severity& severity, 
        const Message::Code& code, 
        const std::string& message, 
        const Position& pos);
      /** String representation of severity
       *  @return Representation of severity
       *  @param severity Severity
       **/
      std::string severityName
      (Message::Severity severity);
      /** Test if file already included
       *  @return true if file already included
       *  @param fileName File name
       **/
      bool isIncluded(const std::string& fileName);
      /// String representation of sign
      std::string sign(Sign aSign);
      /// String representation of value vector
      std::string valueToString
      (std::vector<std::string> value, bool isVector);
      /// Print information or not
      bool isPrint(){ return m_isPrint;}
      /// Print options
      void printOptions();
      /** Convert position to string
       *  @return string representation of position
       *  @param line Line
       *  @param column Column
       */
      std::string posString(int line, int column);
    };

    
    /** Parse job options fiel using the configured parser object
     *  @author Vanya BELYAEV ibelyaev@phsycis.syr.edu
     *  @date 2006-06-28
     */
    StatusCode parse
    ( Parser                parser    ,
      const std::string&    fileName  ,
      std::vector<Message>& msgs      ) ;
    
    /** Parse job options file
     *  @return StatusCode::SUCCESS if no errors
     *  @param filename Path to options file
     *  @param searchPath Directories for search include and units files
     *  @param Catalogue of  parser properties
	 *	@param included Already included files
     *  @param List of parser messages
     *  @return status code 
     *  @author  Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @date  2006-02-17 
     */
    StatusCode parse
    ( const std::string&                    filename   ,
      const std::vector<std::string>&       searchPath ,
      Catalogue&			catalogue  ,
	  std::vector<std::string>& included,
      std::vector<Message>& messages   ) ;
    
    /** Parse job options file
     *  @return StatusCode::SUCCESS if no errors
     *  @param filename Path to options file
     *  @param searchPath Directories for search include and units files
     *  @param Catalogue of  parser properties
	 *	@param included Already included files
     *  @param List of parser messages
     *  @return status code 
     *  @author  Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @date  2006-02-17 
     */
    StatusCode parse
    ( const std::string&                    filename   ,
      const std::string&                    searchPath ,
      Catalogue&            catalogue  ,
	  std::vector<std::string>& included,
      std::vector<Message>& messages   ) ;
    
    /** Parse job options file
     *  @return StatusCode::SUCCESS if no errors
     *  @param filename Path to options file
     *  @param Catalogue of  parser properties
	 *	@param included Already included files
     *  @param List of parser messages
     *  @return status code 
     *  @author  Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @date  2006-02-17 
     */
    StatusCode parse
    ( const std::string&                   filename  ,
      Catalogue&           catalogue ,
	  std::vector<std::string>& included,
      std::vector<Message>& messages  ) ;    

    namespace Utils 
    {
      // ======================================================================
      /** Get enviroment variable
       *  @return StatusCode::SUCCESS if variable exists
       *  @param envName Enviroment variable name
       *  @param envValue Result = enviroment value
       */
      StatusCode getEnv(const std::string& envName,std::string& envValue);
      // ======================================================================
      /** Search file
       *  @returl StatusCode::SUCCESS if file was founded
       *  @param fileInput Path to file which can be not completed
       *  @param addCurrent Add current program directory?
       *  @param dirs Directories in which we can search file
       *  @param fileOutPut Result - absolute path to file
       */
      StatusCode searchFile
      ( const std::string&              fileInput  , 
        bool                            addCurrent ,
        const std::vector<std::string>& dirs       , 
        std::string&                    fileOutput ) ;
      // ======================================================================
      /** Remove enviroment variables from string
       *  @return String without enviroment variables
       *  @param input String to process
       */
      std::string removeEnvironment(const std::string& input);
      // ======================================================================
      /// Check if os is Windows
      bool isWin();
      /// Get path separator
      std::string pathSeparator();
      // ======================================================================
      /** Extract paths separeted by '.' or ':' or ';'
       *  @return List of paths
       *  @param input String to process
       *  @param removeEnv Remove enviroment variables?
       */
      std::vector<std::string> 
      extractPath ( const std::string& input , bool removeEnv = true );
      // ======================================================================
      /** Read file to string
       *  @return StatusCode::SUCCESS if file was readen succesfully
       *  @param name Path to file
       *  @param result Result string
       */
      StatusCode readFile ( const std::string& name , std::string& result ) ;
      // ======================================================================
      /** Try to recognize string as value  of job options  type
       *  @return StatusCode::SUCCESS if string is recognized
       *  @param input string
       *  @param stringResult String representation of value 
       *  @param vectorResult Result vector. Values represented as strings
       **/
      StatusCode parseValue
      ( const std::string&        input        ,
        std::string&              stringResult ,
        std::vector<std::string>& vectorResult ) ;
      // ======================================================================
      
    } // end of namespace Utils
  } // end of namespace Parsers
} // end of namespace Gaudi

// ============================================================================
// The END 
// ============================================================================
#endif // JOBOPTIONSSVC_PARSERUTILS_H
// ============================================================================
