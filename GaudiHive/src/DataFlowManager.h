#ifndef GAUDIHIVE_DATAFLOWMANAGER_H
#define GAUDIHIVE_DATAFLOWMANAGER_H

// FW includes
#include "GaudiKernel/IAlgorithm.h"
#include <GaudiKernel/SmartIF.h>
#include <unordered_map>

// std includes
#include <bitset>
#include <list>

/**@class DataFlowManager DataFlowManager.h GaudiHive/src/DataFlowManager.h
 *
 *  The DataFlowManager takes care of keeping track of the dependencies of 
 *  the algorithms in terms of dataObjects. 
 *  One instance of the DataFlowManager is responsible for one  event.
 * 
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class DataFlowManager{

public:

  /// Type holding the dependencies for one single algorithm
  typedef std::bitset<1000> longBitset;

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

private:
  /// Catalog of the products in the whiteboard
  longBitset m_dataObjectsCatalog;
  /// Requirements of algos. Static since the same for all events.
  static std::vector< longBitset > m_algosRequirements; 
  /// Simple helper method
  inline long int m_productName2index(const std::string productName){return m_productName_index_map.count(productName)>0 ? m_productName_index_map[productName]: -1 ;};
  /// Track the products, assigning an index to them. Static since the same for all events.
  static std::unordered_map<std::string,long int> m_productName_index_map;
  
};

#endif
