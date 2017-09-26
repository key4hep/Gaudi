#ifndef GAUDIKERNEL_DATASELECTIONAGENT_H
#define GAUDIKERNEL_DATASELECTIONAGENT_H

#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/IRegistry.h"

/** @class DataSelectionAgent DataSelectionAgent.h GaudiKernel/DataSelectionAgent.h

    DataSelectionAgent base in charge of collecting all the refereces to
    DataObjects in a transient store that passes some selection criteria. The
    default behaviour is to collect all entries.

    @author Markus Frank
*/
class DataSelectionAgent : virtual public IDataStoreAgent
{
protected:
  IDataSelector m_objects;

public:
  /// Default creator
  DataSelectionAgent() {}
  /// Destructor
  virtual ~DataSelectionAgent() {}
  /// Return the set of selected DataObjects
  IDataSelector* selectedObjects() { return &m_objects; }
  /// Analyses a given directory entry
  bool analyse( IRegistry* pRegistry, int ) override
  {
    DataObject* obj = pRegistry->object();
    if ( 0 != obj ) m_objects.push_back( obj );
    return true;
  }
};
#endif // GAUDIKERNEL_DATASELECTIONAGENT_H
