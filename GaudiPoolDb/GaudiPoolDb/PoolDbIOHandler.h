// $Id: PoolDbIOHandler.h,v 1.4 2008/05/05 19:48:14 marcocle Exp $
//====================================================================
//	PoolDbIOHandler.h
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/GaudiPoolDb/PoolDbIOHandler.h,v 1.4 2008/05/05 19:48:14 marcocle Exp $
#ifndef POOLDB_POOLDBIOHANDLER_H
#define POOLDB_POOLDBIOHANDLER_H
#include "TClassStreamer.h"
#include "Reflex/Reflex.h"

template <class T> class PoolDbIOHandler : public TClassStreamer {
protected:
  typedef ROOT::Reflex::Type TypeH;
  /// LCG Reflection type
  TypeH     m_type;
  /// ROOT persistent class description
  TClass*   m_root;
public:
  /// Initializing constructor
  PoolDbIOHandler(const TypeH& typ, TClass* c) : m_type(typ), m_root(c) {  }

  /// Standard destructor
  virtual ~PoolDbIOHandler() {  }

  /// ROOT I/O callback
  virtual void operator()(TBuffer &b, void *obj);

  /// Callback for reading the object
  /** @param obj  [IN]   Pointer to user object.
    */
  virtual void get(TBuffer &b, void* obj);

  /// Callback for writing the object
  /** @param obj  [IN]   Pointer to user object.
    */
  virtual void put(TBuffer &b, void* obj);
};

#endif // POOLDB_POOLDBIOHANDLER_H
