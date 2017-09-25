#ifndef GAUDIKERNEL_HISTOGRAMAGENT_H
#define GAUDIKERNEL_HISTOGRAMAGENT_H

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/IRegistry.h"

/** @class HistogramAgent HistogramAgent.h GaudiKernel/HistogramAgent.h

    HistogramAgent base in charge of collecting all the refereces to
    DataObjects in a transient store that passes some selection criteria. The
    default behaviour is to collect all entries.

    @author Markus Frank
*/
class HistogramAgent : virtual public IDataStoreAgent
{
protected:
  IDataSelector m_objects;

public:
  /// Default creator
  HistogramAgent() = default;

  /// Destructor
  ~HistogramAgent() override = default;

  /// Return the set of selected DataObjects
  IDataSelector* selectedObjects() { return &m_objects; }
  /// Analyses a given directory entry
  bool analyse( IRegistry* pRegistry, int ) override
  {
    DataObject* obj = pRegistry->object();
    if ( obj && obj->clID() != CLID_StatisticsFile ) {
      m_objects.push_back( obj );
      return true;
    }
    return false;
  }
};
#endif // GAUDIKERNEL_HISTOGRAMAGENT_H
