#include "DataFlowManager.h"

//---------------------------------------------------------------------------
// Static members
std::vector< longBitset > DataFlowManager::m_algosRequirements;
std::unordered_map<std::string,unsigned int> DataFlowManager::m_productName_index_map;

//---------------------------------------------------------------------------
/**
 * If this is the first instance, the constructor fills the requirements of 
 * all algorithms and indexes the data products.
 */
DataFlowManager::DataFlowManager(SmartIF<IHiveWhiteBoard> whiteboard, 
                                 unsigned int eventSlotNumber,           
                                 algosDependenciesCollection algosDependencies):
    m_whiteboard(whiteboard),
    m_evtSlotNumber(eventSlotNumber),
    m_dataObjectsCatalog(0){

  // If it's not the first instance, nothing to do here
  if (m_algosRequirements.size()==0){      
    // This is the first instance, compile the requirements
    m_algosRequirements.resize(algosDependencies.size());
                        
    // Fill the requirements
    unsigned int algoIndex=0;    
    unsigned int productIndex=0;    
    for (auto& thisAlgoDependencies : algosDependencies){
      // Make a local alias for better readability
      auto& depenency_bits = m_algosRequirements[algoIndex];    
      for (auto& product : thisAlgoDependencies){
        auto ret_val = m_productName_index_map.insert(std::pair<std::string, unsigned int>("/Event/"+product,productIndex));
        // insert successful means product wasn't known before. So increment counter
        if (ret_val.second==true) ++productIndex;
        // in any case the return value holds the proper product index
        depenency_bits[ret_val.first->second] = true;                  
      }// end loop on products on which the algo depends
      algoIndex++;
    }// end loop on algorithms
  }
                    
}

//---------------------------------------------------------------------------                  

/**
 * This method is called to know if the algorithm can run according to what 
 * data objects are in the event.
 */
bool DataFlowManager::canAlgorithmRun(unsigned int iAlgo){
  const longBitset& thisAlgoRequirements = m_algosRequirements[iAlgo];
  longBitset dependencies_missing = (m_dataObjectsCatalog & thisAlgoRequirements) ^ thisAlgoRequirements;
  if (dependencies_missing == 0) 
    return true;
  else 
    return false;
}

//---------------------------------------------------------------------------  

/// Update the catalog of available products in the slot
void DataFlowManager::updateDataObjectsCatalog(){
  // DP: performance of interrogating the WB to be checked.
  std::vector<std::string> new_products;  
  m_whiteboard->selectStore(m_evtSlotNumber).ignore();
  m_whiteboard->getNewDataObjects(new_products).ignore();  
  for (const auto& new_product : new_products)
    m_dataObjectsCatalog[m_productName2index(new_product)]=true;
  
}

//---------------------------------------------------------------------------  
  

void DataFlowManager::reset(unsigned int newEventSlotNumber){
  m_dataObjectsCatalog = 0;
  m_evtSlotNumber = newEventSlotNumber;
}

//---------------------------------------------------------------------------                  

