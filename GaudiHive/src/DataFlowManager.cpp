#include "DataFlowManager.h"

//---------------------------------------------------------------------------
// Static members
std::vector<DataFlowManager::dependency_bitset> DataFlowManager::m_algosRequirements;
std::unordered_map<DataObjID, long int, DataObjID_Hasher> DataFlowManager::m_productName_index_map;
std::vector<DataObjID> DataFlowManager::m_productName_vec;
//---------------------------------------------------------------------------
/**
 * If this is the first instance, the constructor fills the requirements of
 * all algorithms and indexes the data products.
 */
DataFlowManager::DataFlowManager( algosDependenciesCollection algosDependencies )
{

  // Count how many products are actually requested
  unsigned int nProducts( 0 );
  for ( auto& thisAlgoDependencies : algosDependencies ) {
    nProducts += thisAlgoDependencies.size();
  }
  m_dataObjectsCatalog.resize( nProducts );

  // If it's not the first instance, nothing to do here
  if ( m_algosRequirements.size() == 0 ) {
    // This is the first instance, compile the requirements
    m_algosRequirements.resize( algosDependencies.size(), dependency_bitset( nProducts ) );

    // Fill the requirements
    unsigned int algoIndex = 0;
    long int productIndex  = 0;
    for ( auto& thisAlgoDependencies : algosDependencies ) {
      // Make a local alias for better readability
      auto& dependency_bits = m_algosRequirements[algoIndex];
      for ( auto& product : thisAlgoDependencies ) {
        auto ret_val = m_productName_index_map.insert( std::pair<DataObjID, long int>( product, productIndex ) );
        // insert successful means product wasn't known before. So increment counter
        if ( ret_val.second == true ) ++productIndex;
        // in any case the return value holds the proper product index
        dependency_bits[ret_val.first->second] = true;
      } // end loop on products on which the algo depends
      algoIndex++;
    } // end loop on algorithms

    // Now the vector of products
    m_productName_vec.resize( m_productName_index_map.size() );
    for ( auto& name_idx : m_productName_index_map ) m_productName_vec[name_idx.second] = name_idx.first;
  }
}

//---------------------------------------------------------------------------

/**
 * This method is called to know if the algorithm can run according to what
 * data objects are in the event.
 */
bool DataFlowManager::canAlgorithmRun( unsigned int iAlgo )
{
  const dependency_bitset& thisAlgoRequirements = m_algosRequirements[iAlgo];
  return thisAlgoRequirements.is_subset_of( m_dataObjectsCatalog );
}

//---------------------------------------------------------------------------

/// Update the catalog of available products in the slot
void DataFlowManager::updateDataObjectsCatalog( const DataObjIDColl& newProducts )
{
  for ( const auto& new_product : newProducts ) {
    m_fc.insert( new_product );
    const int index                               = productName2index( new_product );
    if ( index >= 0 ) m_dataObjectsCatalog[index] = true;
  }
}

//---------------------------------------------------------------------------

/// Reset the slot for a new event
void DataFlowManager::reset()
{
  m_dataObjectsCatalog.reset();
  m_fc.clear();
}

//---------------------------------------------------------------------------

/// Get the content (inefficient, only for debug in case of crashes)
DataObjIDColl DataFlowManager::content() const
{
  DataObjIDColl products;
  for ( const auto& p : m_fc ) {
    products.insert( p );
  }
  return products;
}

//---------------------------------------------------------------------------
/// Get the data dependencies (inefficient, only for debug in case of crashes)
DataObjIDColl DataFlowManager::dataDependencies( unsigned int iAlgo ) const
{
  // with move semantics this is ~fine
  DataObjIDColl deps;
  for ( unsigned int i = 0; i < m_productName_vec.size(); ++i ) {
    if ( m_algosRequirements[iAlgo][i] ) deps.insert( m_productName_vec[i] );
  }
  return deps;
}

//---------------------------------------------------------------------------
