// $Id:$
// ============================================================================
// STD:
// ============================================================================
#include <fstream>
// ============================================================================
// BOOST:
// ============================================================================
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
// ============================================================================
#include "Parser.h"
#include "Messages.h"
#include "Node.h"
#include "Grammar.h"
#include "Iterator.h"
#include "IncludedFiles.h"
#include "Utils.h"
// ============================================================================
// Gaudi:
// ============================================================================
#include "GaudiKernel/PathResolver.h"
// ============================================================================
namespace classic = boost::spirit::classic;
namespace bf = boost::filesystem;
namespace gp= Gaudi::Parsers;
namespace gpu= Gaudi::Parsers::Utils;
namespace qi = boost::spirit::qi;
// ============================================================================
namespace {
// ============================================================================
void GetLastLineAndColumn(std::ifstream& ifs, int& line, int& column) {
    int n = 0;
    std::string str;
    while(!ifs.eof()) {
        getline(ifs, str);
        n++;
    }
    line = n;
    column = str.length()+1;
    ifs.clear();
    ifs.seekg(0, ifs.beg);
}

template<typename Grammar>
bool ParseStream(std::ifstream& stream,
        const std::string& stream_name, gp::Messages* messages,
               gp::Node* root) {
    
    int last_line, last_column;
    

    GetLastLineAndColumn(stream, last_line, last_column);

    std::string input((std::istreambuf_iterator<char>(stream)),
             std::istreambuf_iterator<char>());

    BaseIterator in_begin(input.begin());
    // convert input iterator to forward iterator, usable by spirit parser
    ForwardIterator fwd_begin =
            boost::spirit::make_default_multi_pass(in_begin);
    ForwardIterator fwd_end;

    // wrap forward iterator with position iterator, to record the position

    Iterator position_begin(fwd_begin, fwd_end, stream_name);
    Iterator position_end;

    Grammar gr;
    gp::SkipperGrammar<Iterator> skipper;

    root->value = stream_name;
    bool result = qi::phrase_parse(position_begin,
            position_end, gr, skipper, *root);

    const IteratorPosition& pos = position_begin.get_position();    
    if (result && (pos.line == last_line) && (pos.column == last_column)) {
        return true;
    }

    messages->AddError(gp::Position(stream_name, pos.line,
            pos.column),"parse error");
    return false;
}

// ============================================================================
template<typename Grammar>
bool ParseFile(const gp::Position& from, const std::string& filename,
        const std::string& search_path,
        gp::IncludedFiles* included, gp::Messages* messages,gp::Node* root) {
  std::string search_path_with_current_dir =
      gpu::replaceEnvironments(search_path);
  if (!from.filename().empty()) { // Add current file directory to search_path
    bf::path file_path(from.filename());
    search_path_with_current_dir = file_path.parent_path().string()
        + (search_path_with_current_dir.empty()?""
                                        :("," + search_path_with_current_dir));
  }
  std::string absolute_path = System::PathResolver::find_file_from_list(
      gpu::replaceEnvironments(filename), search_path_with_current_dir);

  if (absolute_path.empty()) {
    messages->AddError(from, "Couldn't find a file " + filename+
        " in search path '"+ search_path_with_current_dir+"'");
    return false;
  }
  const gp::Position* included_from;
  if (!included->GetPosition(absolute_path, &included_from)){
    included->AddFile(absolute_path, from);
    std::ifstream file(absolute_path.c_str());
    if (!file.is_open()) {
      messages->AddError(from, "Couldn't open a file "+filename);
      return false;
    }
    return ParseStream<Grammar>(file, absolute_path, messages, root);
  } else {
    assert(included_from != NULL);
    messages->AddWarning(from,
        str(boost::format("File %1% already included from %2%")
    % absolute_path %  included_from->ToString()));
    return true;
  }
}
// ============================================================================
}
// ============================================================================
bool gp::Parse(const std::string& filename,
        const std::string& search_path, IncludedFiles* included,
        Messages* messages,  Node* root){
    return Parse(Position(),  filename, search_path, included, messages,
            root);
}
// ============================================================================
bool gp::Parse(const Position& from,
        const std::string& filename,
        const std::string& search_path, IncludedFiles* included,
        Messages* messages, Node* root) {
    typedef FileGrammar<Iterator,SkipperGrammar<Iterator> > Grammar;
    return ParseFile<Grammar>(from, filename, search_path, included,
            messages, root);
}

// ============================================================================
bool gp::ParseUnits(const Position& from,
        const std::string& filename,
        const std::string& search_path, IncludedFiles* included,
        Messages* messages, Node* root) {
    typedef UnitsGrammar<Iterator,SkipperGrammar<Iterator> > Grammar;
    return ParseFile<Grammar>(from, filename, search_path, included,
            messages, root);
}
// ============================================================================
