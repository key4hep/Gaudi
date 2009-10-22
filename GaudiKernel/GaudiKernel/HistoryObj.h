// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/HistoryObj.h,v 1.1 2004/07/14 18:13:56 leggett Exp $

#ifndef GAUDIKERNEL_HISTORYOBJ_H
#define GAUDIKERNEL_HISTORYOBJ_H

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ClassID.h"

/** @class HistoryObj HistoryObj.h GaudiKernel/HistoryObj.h

    Base class for History Objects

    @author Charles Leggett
    @date   Jul 13 2004

*/

class GAUDI_API HistoryObj: public DataObject {
public:
  HistoryObj();
  virtual ~HistoryObj(){};

  virtual const CLID& clID() const { return HistoryObj::classID(); }
  static const CLID& classID();


};

#endif
