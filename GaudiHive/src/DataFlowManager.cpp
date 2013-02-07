#include "DataFlowManager.h"

//---------------------------------------------------------------------------
// Static members
std::vector< DataFlowManager::longBitset > DataFlowManager::m_algosRequirements;
std::unordered_map<std::string,long int> DataFlowManager::m_productName_index_map;
std::vector<std::string> DataFlowManager::m_productName_vec;
//---------------------------------------------------------------------------
/**
 * If this is the first instance, the constructor fills the requirements of 
 * all algorithms and indexes the data products.
 */
DataFlowManager::DataFlowManager(algosDependenciesCollection algosDependencies){

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
  
  // Now the vector of products    
  m_productName_vec.resize(m_algosRequirements.size());
  for (auto& name_idx : m_productName_index_map)
    m_productName_vec[name_idx.second]=name_idx.first;    
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
  return dependencies_missing == 0 ? true : false;
}

//---------------------------------------------------------------------------

/// Update the catalog of available products in the slot
void DataFlowManager::updateDataObjectsCatalog(const std::vector<std::string>& newProducts){
  // DP: performance of interrogating the WB to be checked.
  for (const auto& new_product : newProducts){
    const int index = m_productName2index(new_product);
//    std::cout << "New product on slot "<< m_evtSlotNumber << " ***" << new_product<< " indexed as "<< index <<"*** " << std::endl;
    if (index>=0)
      m_dataObjectsCatalog[index]=true;
  }
  
}

//---------------------------------------------------------------------------

/// Reset the slot for a new event
void DataFlowManager::reset(){
  m_dataObjectsCatalog.reset();
}

//---------------------------------------------------------------------------

/// Get the content (inefficient, only for debug in case of crashes)
std::vector<std::string> DataFlowManager::content() const{  
  // with move semantics this is ~fine
  std::vector<std::string> products;
  for (unsigned int i=0;i<m_productName_vec.size();++i){
    if (m_dataObjectsCatalog[i])
      products.push_back(m_productName_vec[i]);
  }
  return products;
}

//---------------------------------------------------------------------------
/// Get the data dependencies (inefficient, only for debug in case of crashes)
std::vector<std::string> DataFlowManager::dataDependencies(unsigned int iAlgo) const{
  // with move semantics this is ~fine
  std::vector<std::string> deps;
  for (unsigned int i=0;i<m_productName_vec.size();++i){
    if (m_algosRequirements[iAlgo][i])
      deps.push_back(m_productName_vec[i]);
  }
  return deps;
}

//---------------------------------------------------------------------------
