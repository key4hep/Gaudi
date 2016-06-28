#ifndef GAUDISVC_PERSISTENCYSVC_OUTPUTSTREAM_H
#define GAUDISVC_PERSISTENCYSVC_OUTPUTSTREAM_H

// Required for inheritance
#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "OutputStreamAgent.h"

// STL include files
#include <memory>
#include <vector>
#include <string>

// forward declarations
class IIncidentSvc;
class IRegistry;
class IConversionSvc;
class IDataManagerSvc;
class OutputStreamAgent;
class DataStoreItem;

/** A small to stream Data I/O.
    Author:  M.Frank
    author R. Lambert
    Version: 1.0
*/
class OutputStream : public Algorithm     {
public:
  typedef std::vector<DataStoreItem*> Items;
  typedef std::vector<std::string>    ItemNames;
  typedef std::map< Algorithm*,  Items     > AlgDependentItems;
  typedef std::map< std::string, ItemNames > AlgDependentItemNames;
protected:
  PropertyWithValue<ItemNames> m_itemNames{this, "ItemList", {}, "vector of item names to be saved to this stream"};
  PropertyWithValue<ItemNames> m_optItemNames{this, "OptItemList", {}, "vector of optional item names to be saved to this stream"};
  PropertyWithValue<AlgDependentItemNames> m_algDependentItemList{this, "AlgDependentItemList", {},
    "mapping between algorithm names, and a list of items for which, if the "
    "algorithm in question accepted the event, they should be also stored"};
  BooleanProperty m_doPreLoad{this, "Preload",  true, "flag indicating whether data pre-loading should be performed"};
  BooleanProperty m_doPreLoadOpt{this, "PreloadOptItems",  false, "flag indicating whether optional items should be preloaded"};
  StringProperty m_output{this, "Output", {}, "name of the output file specification"};
  StringProperty m_outputName{this, "OutputFile", {}, "name of the output file"};
  StringProperty m_storeName{this, "EvtDataSvc",  "EventDataSvc", "name of the service managing the data store"};
  StringProperty m_persName{this, "EvtConversionSvc",  "EventPersistencySvc", "name of the persistency service capable to write data from the store"};
  StringArrayProperty m_acceptNames{this, "AcceptAlgs", {}, "names of Algorithms that this stream accepts"};
  StringArrayProperty m_requireNames{this, "RequireAlgs", {}, "names of Algorithms that this stream requires"};
  StringArrayProperty m_vetoNames{this, "VetoAlgs", {}, "names of Algorithms that this stream is vetoed by"};
  BooleanProperty m_verifyItems{this, "VerifyItems",  true, "flag to indicate that item consistency should be checked"};

  /// should I fire incidents for writing opening/closing etc?
  /// in the baseclass, always fire the incidents by default
  /// in e.g. RecordStream this will be set to false, and configurable
  bool m_fireIncidents = true;

  /// Reference to the incident service
  SmartIF<IIncidentSvc> m_incidentSvc;

  /// Output type: NEW(NEW,CREATE,WRITE,RECREATE), UPDATE)
  std::string              m_outputType = "UPDATE";
  /// Keep reference of agent
  std::unique_ptr<OutputStreamAgent> m_agent;
  /// Keep reference to the data provider service
  SmartIF<IDataProviderSvc>        m_pDataProvider;
  /// Keep reference to the data manager service
  SmartIF<IDataManagerSvc>         m_pDataManager;
  /// Keep reference to the data conversion service
  SmartIF<IConversionSvc>          m_pConversionSvc;
  /// Keep track of the current item
  DataStoreItem*           m_currentItem;
  /// Vector of items to be saved to this stream
  Items                    m_itemList;
  /// Vector of optional items to be saved to this stream
  Items                    m_optItemList;
  /// Items to be saved for specific algorithms
  AlgDependentItems        m_algDependentItems;
  /// Collection of objects being selected
  IDataSelector            m_objects;
  /// Number of events written to this output stream
  int                      m_events;

  /// Vector of Algorithms that this stream accepts
  std::vector<Algorithm*>  m_acceptAlgs;
  /// Vector of Algorithms that this stream requires
  std::vector<Algorithm*>  m_requireAlgs;
  /// Vector of Algorithms that this stream is vetoed by
  std::vector<Algorithm*>  m_vetoAlgs;

public:
  /// Standard algorithm Constructor
  OutputStream(const std::string& name, ISvcLocator* pSvcLocator);
  /// Standard Destructor
  virtual ~OutputStream() = default;
protected:
  /// Decode list of Algorithms that this stream accepts
  StatusCode decodeAcceptAlgs();
  /// Handler for AcceptAlgs Property
  void acceptAlgsHandler( Property& theProp );
  /// Decode list of Algorithms that this stream requires
  StatusCode decodeRequireAlgs();
  /// Handler for RequireAlgs Property
  void requireAlgsHandler( Property& theProp );
  /// Decode list of Algorithms that this stream is vetoed by
  StatusCode decodeVetoAlgs();
  /// Handler for VetoAlgs Property
  void vetoAlgsHandler( Property& theProp );
  /// Decode a single algorithm name
  Algorithm* decodeAlgorithm( const std::string& theName );
  /// Decode specified list of Algorithms
  StatusCode decodeAlgorithms( StringArrayProperty& theNames,
                               std::vector<Algorithm*>& theAlgs );
  /// Test whether this event should be output
  bool isEventAccepted() const;
  /// Find single item identified by its path (exact match)
  DataStoreItem* findItem(const std::string& path);
  /// Select the different objects and write them to file
  virtual StatusCode writeObjects();

  /// Tell if the instance has been configured with input items or not.
  virtual bool hasInput() const;

public:
  /// Initialize OutputStream
  StatusCode initialize() override;
  /// Terminate OutputStream
  StatusCode finalize() override;
  /// Working entry point
  StatusCode execute() override;
  // Connect to proper conversion service
  virtual StatusCode connectConversionSvc();
  /// Store agent's classback
  virtual bool collect(IRegistry* dir, int level);
  /// Collect all objects to be written to the output stream
  virtual StatusCode collectObjects();
  /// Clear list of selected objects
  void clearSelection();

private:
  /// Clear item list
  void clearItems(Items& itms);
  /// Add item to output streamer list
  void addItem(Items& itms, const std::string& descriptor);
  /// Return the list of selected objects
  IDataSelector* selectedObjects() { return &m_objects; }
};

#endif // GAUDISVC_PERSISTENCYSVC_OUTPUTSTREAM_H
