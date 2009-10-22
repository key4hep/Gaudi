// $Id: ParserUtils.cpp,v 1.7 2007/05/24 14:41:22 hmd Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.7 $ 
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
#include <fstream>
// ============================================================================
// Boost
// ============================================================================
#include <boost/filesystem/operations.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
// ============================================================================
// local
// ============================================================================
#include "ParserUtils.h"
#include "ParserGrammar.h"
// ============================================================================
namespace fs = boost::filesystem;
using namespace std;

// ============================================================================
/** @file
 *  implementation file for helper functions from 
 *  namespace Gaudi::Parsers and namespace Gaudi::Parsers:Utils 
 *  @see Gaudi::Parsers 
 *  @see Gaudi::Parsers::Utils 
 *
 *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-05-13
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse
( Gaudi::Parsers::Parser                parser    ,
  const std::string&                    fileName  ,
  std::vector<Gaudi::Parsers::Message>& msgs      )
{
  StatusCode res = parser.parse(fileName);
  msgs = parser.messages();
  return res;  
}
// ============================================================================
StatusCode Gaudi::Parsers::parse
( const string& filename,
  const vector<string>& searchPath,
  Gaudi::Parsers::Catalogue& catalogue,
  std::vector<std::string>& included,
  vector<Gaudi::Parsers::Message>& msgs)
{
  return parse
    ( Gaudi::Parsers::Parser(catalogue, included, searchPath) , 
      filename , msgs ) ;  
}
// ============================================================================
StatusCode Gaudi::Parsers::parse
( const string& filename,
  const string& searchPath,
  Gaudi::Parsers::Catalogue& catalogue,
  std::vector<std::string>& included,
  vector<Gaudi::Parsers::Message>& msgs)
{
  return parse
    ( Gaudi::Parsers::Parser (catalogue , included, searchPath ) , 
      filename ,  msgs ) ;  
}
// ============================================================================
StatusCode Gaudi::Parsers::parse
( const string& filename,
  Gaudi::Parsers::Catalogue& catalogue,
  std::vector<std::string>& included,
  vector<Gaudi::Parsers::Message>& msgs){
  return parse
    (Gaudi::Parsers::Parser(catalogue,included),filename,msgs);
}
// ============================================================================
StatusCode Gaudi::Parsers::Utils::getEnv
( const std::string& envName , std::string& envValue )
{
  char* envch = getenv(envName.c_str());
	if(envch==NULL) { return StatusCode::FAILURE; }
	envValue = envch;
  return StatusCode::SUCCESS;
}
// ============================================================================
std::string 
Gaudi::Parsers::Utils::removeEnvironment(const std::string& input){
  std::string result=input;// result
  
  const char* re = "\\$(([A-Za-z0-9_]+)|\\(([A-Za-z0-9_]+)\\))";
  std::string::const_iterator start, end;
  boost::regex expression(re);
  start = input.begin();
  end = input.end();   
  boost::match_results<std::string::const_iterator> what;
  boost::match_flag_type flags = boost::match_default;
  while ( boost::regex_search(start, end, what, expression, flags ) )   
  {
    std::string var,env;
    std::string matched(what[0].first,what[0].second);
    std::string v1(what[2].first,what[2].second);
    std::string v2(what[3].first,what[3].second);      
    
    if ( v1.length()>0){ var = v1; }
    else { var = v2; }
    
    StatusCode ok = getEnv(var, env);
    if(ok.isSuccess())
    { boost::algorithm::replace_first(result,matched, env); }
    start = what[0].second;
    // update flags:
    flags |= boost::match_prev_avail;
    flags |= boost::match_not_bob;
  }
  return result;
}
// ============================================================================
StatusCode Gaudi::Parsers::Utils::searchFile
( const std::string&              fileInput  , 
  bool                            addCurrent ,
  const std::vector<std::string>& dirs       , 
  std::string&                    fileOutput )
{
	std::string result;
  try {
    fs::path givenPath(removeEnvironment(fileInput),fs::native);
    //std::cout<<"Given path="<<givenPath.string()<<std::endl;
    fs::path currentPath = givenPath;
    std::vector<std::string> sdirs = dirs;
    if(addCurrent){
      sdirs.insert(sdirs.begin(), 
                   fs::initial_path().native_directory_string());
    }
    std::vector<std::string>::const_iterator current = 
      sdirs.begin(),end=sdirs.end();
    while(1) {
      if(fs::exists(currentPath)) {
        fileOutput = currentPath.native_directory_string();
        return StatusCode::SUCCESS;
      }
      if(current!=end) {
        std::string _n = *current;
        currentPath = fs::path(_n, fs::native)/givenPath;
      }else{
        return StatusCode::FAILURE;
      }
      ++current;
    }
  }catch(...) {}
  return StatusCode::FAILURE;
}
// ============================================================================
StatusCode Gaudi::Parsers::Utils::readFile
(const std::string& name ,  std::string& result)
{
  std::ifstream in(name.c_str());
  if (!in.is_open()) { return StatusCode::FAILURE; }
  char c;
  while (!in.get(c).eof()) { result += c; }
  return StatusCode::SUCCESS;  
}
// ============================================================================
bool Gaudi::Parsers::Utils::isWin()
{
#ifdef _WIN32
  return true;
#else
  return false;
#endif  
}
// ============================================================================
std::string Gaudi::Parsers::Utils::pathSeparator()
{ return Gaudi::Parsers::Utils::isWin()?";":":"; }
// ============================================================================
std::vector<std::string> Gaudi::Parsers::Utils::extractPath
( const std::string& input, bool removeEnv )
{
  std::string in = removeEnv?removeEnvironment(input):input;
  std::string separator = ","+pathSeparator();
  typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
  std::vector<std::string> result;
  Tokenizer tok(in, boost::char_separator<char>(separator.c_str()));
  Tokenizer::iterator it =tok.begin();
  while(it!=tok.end())
  {
    if(it->length()>0){ result.push_back(*it); }
    it++;
  }
  return result;
}
// ============================================================================
StatusCode Gaudi::Parsers::Utils::parseValue
( const string&             input        , 
  std::string&              stringResult ,
  std::vector<std::string>& vectorResult )
{
  
  typedef 
    boost::spirit::position_iterator<std::string::const_iterator> IteratorT;
  
  Gaudi::Parsers::ValueGrammar   grValue;
  Gaudi::Parsers::SkipperGrammar grSkipper;
  IteratorT beginpos(input.begin(), input.end(), "");
  IteratorT endpos;
  boost::tuple<std::string, std::vector<std::string> > result;
  boost::spirit::parse_info<IteratorT> info = 
    boost::spirit::parse
    (beginpos, endpos, grValue[var(result)=arg1] ,grSkipper);
  if (!info.full){ return StatusCode::FAILURE; }
  stringResult = result.get<0>();
  vectorResult = result.get<1>();
  return StatusCode::SUCCESS;
}
// ============================================================================

// ============================================================================
// The END 
// ============================================================================

