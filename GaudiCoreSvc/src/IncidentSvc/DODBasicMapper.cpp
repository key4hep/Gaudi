// Include files

#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/Parsers.h"
// Local implementation of parsers from HashMap
namespace Gaudi {
  namespace Parsers {
    /// Basic parser for the types of HashMap used in DODBasicMapper.
    /// Delegates to the parser of map<string,string>.
    template <typename K, typename V>
    StatusCode parse(GaudiUtils::HashMap<K, V> &result, const std::string &input) {
      std::map<std::string, std::string> tmp;
      StatusCode sc = parse(tmp, input);
      if (sc.isSuccess()) {
        result.clear();
        result.insert(tmp.begin(), tmp.end());
      }
      return sc;
    }
  }
}

// From Gaudi
#include "GaudiKernel/ToolFactory.h"
// local
#include "DODBasicMapper.h"

// ----------------------------------------------------------------------------
// Implementation file for class: DODBasicMapper
//
// 16/01/2012: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_COMPONENT(DODBasicMapper)

// ============================================================================
// Standard constructor, initializes variables
// ============================================================================
DODBasicMapper::DODBasicMapper(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type, name, parent)
{
  declareProperty("Nodes", m_nodeMap,
                  "Map of the type of nodes to be associated to paths (path -> data_type).");
  declareProperty("Algorithms", m_algMap,
                  "Map of algorithms to be used to produce entries (path -> alg_name).");
}

// ============================================================================
// Destructor
// ============================================================================
DODBasicMapper::~DODBasicMapper() {}


Gaudi::Utils::TypeNameString DODBasicMapper::algorithmForPath(const std::string & path)
{
  AlgMap::iterator alg = m_algMap.find(path);
  return (alg != m_algMap.end()) ? alg->second : "";
}


std::string DODBasicMapper::nodeTypeForPath(const std::string & path)
{
  NodeMap::iterator node = m_nodeMap.find(path);
  return (node != m_nodeMap.end()) ? node->second : "";
}



// ============================================================================
