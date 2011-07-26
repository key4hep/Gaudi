// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/HistoryObj.h,v 1.1 2004/07/14 18:13:56 leggett Exp $

#ifndef GAUDIKERNEL_HISTORYOBJ_H
#define GAUDIKERNEL_HISTORYOBJ_H

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ClassID.h"
#include <string>
#include <ostream>
#include <vector>

class Property;
class IVersHistoryObj;

/** @class HistoryObj HistoryObj.h GaudiKernel/HistoryObj.h

    Base class for History Objects

    @author Charles Leggett
    @date   Jul 13 2004

*/

class GAUDI_API HistoryObj: public DataObject {
public:
  HistoryObj();
  virtual ~HistoryObj();

  virtual const CLID& clID() const { return classID(); }
  static const CLID& classID();
  static std::string convert_string(const std::string&);

  virtual void dump(std::ostream&, const bool isXML=false,
		    int indent=0) const = 0;

 protected:
  virtual void indent(std::ostream&, int) const;

};

// Output stream.
GAUDI_API std::ostream& operator<<(std::ostream& lhs, const HistoryObj& rhs);


#endif
