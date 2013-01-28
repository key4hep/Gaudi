#ifndef GAUDIHIVE_DATAFLOWMANAGER_H
#define GAUDIHIVE_DATAFLOWMANAGER_H

// FW includes
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IAlgorithm.h"
#include <GaudiKernel/SmartIF.h>
#include <unordered_map>

// std includes
#include <bitset>
#include <list>


/**@class DataFlowManager DataFlowManager.h GaudiHive/src/DataFlowManager.h
 *
 *  The DataFlowManager takes care of keeping track of the dependencies of 
 *  the algorithms in terms of dataObjects. It communicates with 
 *  the whiteboard and compiles all the dependencies looping through the 
 *  algorithms. One instance of the DataFlowManager is responsible for one 
 *  event.
 * 
 *  @author  Danilo Piparo
 *  @version 1.0
 */
typedef std::bitset<1000> longBitset;  

class DataFlowManager{
// typedef for the event and algo state
  
public:
  typedef std::vector<std::vector<std::string>> algosDependenciesCollection;  
  DataFlowManager(SmartIF<IHiveWhiteBoard> whiteboard, 
                  unsigned int eventSlotNumber,
                  const std::list<IAlgorithm*>& /*algos*/):
                  m_whiteboard(whiteboard),
                  m_evtSlotNumber(eventSlotNumber),
                  m_dataObjectsCatalog(0){};// DP ctor to be implemented when deps will be in the algorithms

  DataFlowManager(SmartIF<IHiveWhiteBoard> whiteboard,
                  unsigned int eventSlotNumber,
                  algosDependenciesCollection algoDependencies);

  /// Needed data products are available
  bool canAlgorithmRun(unsigned int iAlgo);
  
  /// Update the catalog of available products in the slot
  void updateDataObjectsCatalog();
  
  /// Reset to default values
  void reset(unsigned int newEventSlotNumber);
                  
private:
  SmartIF<IHiveWhiteBoard> m_whiteboard; 
  unsigned int m_evtSlotNumber;
  longBitset m_dataObjectsCatalog;
  /// Requirements of algos. Static since the same for all events.
  static std::vector< longBitset > m_algosRequirements; 
  /// Simple helper method
  unsigned int m_productName2index(const std::string productName){return m_productName_index_map[productName];};
  /// Track the products, assigning an index to them. Static since the same for all events.
  static std::unordered_map<std::string,unsigned int> m_productName_index_map;
  
};

#endif