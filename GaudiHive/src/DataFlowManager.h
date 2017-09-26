#ifndef GAUDIHIVE_DATAFLOWMANAGER_H
#define GAUDIHIVE_DATAFLOWMANAGER_H

// FW includes
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/SmartIF.h"

// boost
#include "boost/dynamic_bitset.hpp"

// std includes
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**@class DataFlowManager DataFlowManager.h GaudiHive/src/DataFlowManager.h
 *
 *  The DataFlowManager takes care of keeping track of the dependencies of
 *  the algorithms in terms of dataObjects.
 *  One instance of the DataFlowManager is responsible for one  event.
 *
 *  @author  Danilo Piparo, Benedikt Hegner
 *  @version 1.0
 */
class DataFlowManager
{

public:
  /// Type holding the dependencies for one single algorithm
  typedef boost::dynamic_bitset<> dependency_bitset;

  typedef std::vector<DataObjIDColl> algosDependenciesCollection;
  /// Constructor
  DataFlowManager( const std::list<IAlgorithm*>& /*algos*/ ) : m_dataObjectsCatalog( 0 ){};

  /// Constructor (transitional, will be deprecated once handles are in place)
  DataFlowManager( algosDependenciesCollection algoDependencies );

  /// Needed data products are available
  bool canAlgorithmRun( unsigned int iAlgo );

  /// Update the catalog of available products in the slot
  void updateDataObjectsCatalog( const DataObjIDColl& newProducts );

  /// Reset to default values
  void reset();

  /// Get the content of the catalog
  DataObjIDColl content() const;

  /// Get the dependencies of a single algo;
  DataObjIDColl dataDependencies( unsigned int iAlgo ) const;

private:
  DataObjIDColl m_fc;

  /// Catalog of the products in the whiteboard
  dependency_bitset m_dataObjectsCatalog;
  /// Requirements of algos. Static since the same for all events.
  static std::vector<dependency_bitset> m_algosRequirements;
  /// Track the products, assigning an index to them. Static since the same for all events.
  static std::vector<DataObjID> m_productName_vec;
  /// Track the products, assigning an index to them. Static since the same for all events.
  typedef DataObjID productName_t;

  static std::unordered_map<productName_t, long int, DataObjID_Hasher> m_productName_index_map;
  /// Simple helper method to convert the product name into an index
  inline long int productName2index( const productName_t& productName )
  {
    return m_productName_index_map.count( productName ) > 0 ? m_productName_index_map[productName] : -1;
  };
  /// Simple helper method to convert an index to a product name
  inline DataObjID& index2productName( const unsigned int i ) { return m_productName_vec[i]; };
};

#endif
