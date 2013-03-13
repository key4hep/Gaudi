#ifndef GAUDIHIVE_DATAFLOWMANAGER_H
#define GAUDIHIVE_DATAFLOWMANAGER_H

// FW includes
#include "GaudiKernel/IAlgorithm.h"
#include <GaudiKernel/SmartIF.h>

// boost
#include "boost/dynamic_bitset.hpp"

// std includes
#include <list>
#include <vector>
#include <string>
#include <unordered_map>

/**@class DataFlowManager DataFlowManager.h GaudiHive/src/DataFlowManager.h
 *
 *  The DataFlowManager takes care of keeping track of the dependencies of 
 *  the algorithms in terms of dataObjects. 
 *  One instance of the DataFlowManager is responsible for one  event.
 * 
 *  @author  Danilo Piparo, Benedikt Hegner
 *  @version 1.0
 */
class DataFlowManager{

public:

  /// Type holding the dependencies for one single algorithm
  typedef boost::dynamic_bitset<> dependency_bitset;

  typedef std::vector<std::vector<std::string>> algosDependenciesCollection;  
  /// Constructor
  DataFlowManager(const std::list<IAlgorithm*>& /*algos*/):
                  m_dataObjectsCatalog(0){};

  /// Constructor (transitional, will be deprecated once handles are in place)
  DataFlowManager(algosDependenciesCollection algoDependencies);

  /// Needed data products are available
  bool canAlgorithmRun(unsigned int iAlgo);

  /// Update the catalog of available products in the slot
  void updateDataObjectsCatalog(const std::vector<std::string>& newProducts);

  /// Reset to default values
  void reset();
  
  /// Get the content of the catalog
  std::vector<std::string> content() const;

  /// Get the dependencies of a single algo;
  std::vector<std::string> dataDependencies(unsigned int iAlgo) const;

private:
  /// Catalog of the products in the whiteboard
  dependency_bitset m_dataObjectsCatalog;
  /// Requirements of algos. Static since the same for all events.
  static std::vector< dependency_bitset > m_algosRequirements; 
  /// Simple helper method
  inline long int m_productName2index(const std::string productName){return m_productName_index_map.count(productName)>0 ? m_productName_index_map[productName]: -1 ;};
  /// Track the products, assigning an index to them. Static since the same for all events.
  static std::unordered_map<std::string,long int> m_productName_index_map;
  /// Simple helper method
  inline std::string& m_index2productName(const unsigned int i){return m_productName_vec[i];};
  /// Track the products, assigning an index to them. Static since the same for all events.
  static std::vector<std::string> m_productName_vec;
};

#endif
