/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDISVC_PERSISTENCYSVC_OUTPUTSTREAM_H
#define GAUDISVC_PERSISTENCYSVC_OUTPUTSTREAM_H

// Required for inheritance
#include <Gaudi/Property.h>
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/IDataSelector.h>
#include <GaudiKernel/IIncidentSvc.h>

// STL include files
#include <memory>
#include <string>
#include <vector>

// forward declarations
class IIncidentSvc;
class IRegistry;
class IConversionSvc;
struct IDataManagerSvc;
class OutputStreamAgent;
class DataStoreItem;

/** A small to stream Data I/O.
    Author:  M.Frank
    author R. Lambert
    Version: 1.0
*/
class OutputStream : public Algorithm {
public:
  typedef std::vector<DataStoreItem*>        Items;
  typedef std::vector<std::string>           ItemNames;
  typedef std::map<Gaudi::Algorithm*, Items> AlgDependentItems;
  typedef std::map<std::string, ItemNames>   AlgDependentItemNames;

public:
  /// Standard algorithm Constructor
  using Algorithm::Algorithm;

protected:
  Gaudi::Property<ItemNames> m_itemNames{ this, "ItemList", {}, "vector of item names to be saved to this stream" };
  Gaudi::Property<ItemNames> m_optItemNames{
      this, "OptItemList", {}, "vector of optional item names to be saved to this stream" };
  Gaudi::Property<AlgDependentItemNames> m_algDependentItemList{
      this,
      "AlgDependentItemList",
      {},
      "mapping between algorithm names, and a list of items for which, if the "
      "algorithm in question accepted the event, they should be also stored" };
  Gaudi::Property<bool>                     m_doPreLoad{ this, "Preload", true,
                                     "flag indicating whether data pre-loading should be performed" };
  Gaudi::Property<bool>                     m_doPreLoadOpt{ this, "PreloadOptItems", false,
                                        "flag indicating whether optional items should be preloaded" };
  Gaudi::Property<std::string>              m_output{ this, "Output", {}, "name of the output file specification" };
  Gaudi::Property<std::string>              m_outputName{ this, "OutputFile", {}, "name of the output file" };
  Gaudi::Property<std::string>              m_storeName{ this, "EvtDataSvc", "EventDataSvc",
                                            "name of the service managing the data store" };
  Gaudi::Property<std::string>              m_persName{ this, "EvtConversionSvc", "EventPersistencySvc",
                                           "name of the persistency service capable to write data from the store" };
  Gaudi::Property<std::vector<std::string>> m_acceptNames{
      this,
      "AcceptAlgs",
      {},
      [this]( auto& ) { this->decodeAlgorithms( this->m_acceptNames, this->m_acceptAlgs ); },
      "names of Algorithms that this stream accepts" };
  Gaudi::Property<std::vector<std::string>> m_requireNames{
      this,
      "RequireAlgs",
      {},
      [this]( auto& ) { this->decodeAlgorithms( this->m_requireNames, this->m_requireAlgs ); },
      "names of Algorithms that this stream requires" };
  Gaudi::Property<std::vector<std::string>> m_vetoNames{
      this,
      "VetoAlgs",
      {},
      [this]( auto& ) { this->decodeAlgorithms( this->m_vetoNames, this->m_vetoAlgs ); },
      "names of Algorithms that this stream is vetoed by" };
  Gaudi::Property<bool> m_verifyItems{ this, "VerifyItems", true,
                                       "flag to indicate that item consistency should be checked" };

  /// should I fire incidents for writing opening/closing etc?
  /// in the baseclass, always fire the incidents by default
  /// in e.g. RecordStream this will be set to false, and configurable
  bool m_fireIncidents = true;

  /// Reference to the incident service
  SmartIF<IIncidentSvc> m_incidentSvc;

  /// Output type: NEW(NEW,CREATE,WRITE,RECREATE), UPDATE)
  std::string m_outputType = "UPDATE";
  /// Keep reference to the data provider service
  SmartIF<IDataProviderSvc> m_pDataProvider;
  /// Keep reference to the data manager service
  SmartIF<IDataManagerSvc> m_pDataManager;
  /// Keep reference to the data conversion service
  SmartIF<IConversionSvc> m_pConversionSvc;
  /// Keep track of the current item
  DataStoreItem* m_currentItem;
  /// Vector of items to be saved to this stream
  Items m_itemList;
  /// Vector of optional items to be saved to this stream
  Items m_optItemList;
  /// Items to be saved for specific algorithms
  AlgDependentItems m_algDependentItems;
  /// Collection of objects being selected
  IDataSelector m_objects;
  /// Number of events written to this output stream
  int m_events;

  /// Vector of Algorithms that this stream accepts
  std::vector<Gaudi::Algorithm*> m_acceptAlgs;
  /// Vector of Algorithms that this stream requires
  std::vector<Gaudi::Algorithm*> m_requireAlgs;
  /// Vector of Algorithms that this stream is vetoed by
  std::vector<Gaudi::Algorithm*> m_vetoAlgs;

protected:
  /// Decode a single algorithm name
  Gaudi::Algorithm* decodeAlgorithm( const std::string& theName );
  /// Decode specified list of Algorithms
  void decodeAlgorithms( Gaudi::Property<std::vector<std::string>>& theNames, std::vector<Gaudi::Algorithm*>& theAlgs );
  /// Test whether this event should be output
  bool isEventAccepted() const;
  /// Find single item identified by its path (exact match)
  DataStoreItem* findItem( const std::string& path );
  /// Select the different objects and write them to file
  virtual StatusCode writeObjects();

  /// Tell if the instance has been configured with input items or not.
  virtual bool hasInput() const;

public:
  /// Start OutputStream
  StatusCode start() override;
  /// Initialize OutputStream
  StatusCode initialize() override;
  /// Terminate OutputStream
  StatusCode finalize() override;
  /// Working entry point
  StatusCode execute() override;
  // Connect to proper conversion service
  virtual StatusCode connectConversionSvc();
  /// Store agent's classback
  virtual bool collect( IRegistry* dir, int level );
  /// Collect all objects to be written to the output stream
  virtual StatusCode collectObjects();
  /// Clear list of selected objects
  void clearSelection();

private:
  /// Clear item list
  void clearItems( Items& itms );
  /// Add item to output streamer list
  void addItem( Items& itms, const std::string& descriptor );
  /// Return the list of selected objects
  IDataSelector* selectedObjects() { return &m_objects; }

  StatusCode collectFromSubTree( DataObject* );
};

#endif // GAUDISVC_PERSISTENCYSVC_OUTPUTSTREAM_H
