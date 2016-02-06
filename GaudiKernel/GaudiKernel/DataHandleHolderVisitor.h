#ifndef GAUDIKERNEL_DATAHANDLEHOLDERVISTOR
#define GAUDIKERNEL_DATAHANDLEHOLDERVISTOR 1

#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/DataObjID.h"
#include <set>


class DHHVisitor : public IDataHandleVisitor {
public:
  DHHVisitor(DataObjIDColl& ido, DataObjIDColl& odo);
  
  virtual void visit(const IDataHandleHolder* idhh);
private:
  DataObjIDColl &m_ido, &m_odo;
  
};

#endif
