#ifndef GAUDIKERNEL_HISTOGRAMAGENT_H
#define GAUDIKERNEL_HISTOGRAMAGENT_H

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/IRegistry.h"

/** @class HistogramAgent HistogramAgent.h GaudiKernel/HistogramAgent.h

    HistogramAgent base in charge of collecting all the references to
    DataObjects in a transient store that passes some selection criteria. The
    default behaviour is to collect all entries.

    @author Markus Frank
*/
class HistogramAgent final : virtual public IDataStoreAgent
{
  IDataSelector m_objects;

public:
  /// Return the set of selected DataObjects
  const IDataSelector& selectedObjects() const { return m_objects; }
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
