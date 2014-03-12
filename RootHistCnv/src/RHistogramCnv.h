// $Id: RHistogramCnv.h,v 1.4 2007/01/08 17:16:02 mato Exp $
#ifndef ROOTHISTCNV_RHISTOGRAMCNV_H
#define ROOTHISTCNV_RHISTOGRAMCNV_H 1

// Include files
#include "RConverter.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/HistogramBase.h"
#include "GaudiKernel/ObjectFactory.h"
#include "RootObjAddress.h"
#include "TArray.h"
#include <memory>

namespace RootHistCnv {

  /** @class RHistogramCnv RHistogramCnv.h RHistogramCnv.h
    *
    * Generic converter to save/read AIDA_ROOT histograms using ROOT.
    * This converter shortcuts the AIDA binding and takes advantage
    * of the underlying implementation of transient histograms using
    * ROOT.
    *
    * Note:
    * THxY::Copy cannot be used - only THxY::Add !
    *
    * @author Markus Frank
    */
  template<typename T,typename S,typename Q> class RHistogramCnv : public RConverter {
    template <typename CLASS> struct TTH : public CLASS {
      void CopyH(TObject& o) { CLASS::Copy(o); }
    };
  public:
    /// Create the transient representation of an object.
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& refpObj) {
      refpObj = DataObjFactory::create(objType());
      RootObjAddress *r = dynamic_cast<RootObjAddress*>(pAddr);
      Q* h = dynamic_cast<Q*>(refpObj);
      if ( r && h )   {
        // Need to flip representation .... clumsy for the time being, because
        // THXY constructor has no "generic" copy constructor
        std::auto_ptr<T> p(new T());
        S *s = dynamic_cast<S*>(r->tObj());
        if ( s && p.get() )  {
          TTH<S>* casted = (TTH<S>*)s;
          TArray* a = dynamic_cast<TArray*>(s);
          casted->CopyH(*p);
          if ( 0 != a )  {
            p->Set(a->GetSize());
            p->Reset();
            p->Add(s);
            h->adoptRepresentation(p.release());
            return StatusCode::SUCCESS;
          }
        }
      }
      return error("Cannot create histogram - invalid address.");
    }
    /// Update the transient object from the other representation.
    virtual StatusCode updateObj(IOpaqueAddress* /* pAddr */, DataObject* /* pObj */)  {
      return StatusCode::SUCCESS;
    }
    /// Create the persistent representation of the histogram object.
    virtual TObject* createPersistent(DataObject* pObj)   {
      Q* h = dynamic_cast<Q*>(pObj);
      if ( 0 != h ) {
        T *r = dynamic_cast<T*>(h->representation());
        if ( r )   {
          T* c = new T();
          TArray* a = dynamic_cast<TArray*>(r);
          ((TTH<S>*)r)->CopyH(*c);
          if ( 0 != a )  {
            c->Set(a->GetSize());
            c->Reset();
            c->Add(r);
            c->SetName(pObj->registry()->name().c_str()+1);
            return c;
          }
        }
      }
      error("Histogram object is invalid!");
      return 0;
    }
    /// Inquire class type
    static const CLID& classID();
    /// Standard constructor
    RHistogramCnv(ISvcLocator* svc) : RConverter(classID(), svc) {}
    virtual ~RHistogramCnv() {}
  };
}       // namespace RootHistCnv
#endif  // ROOTHISTCNV_RHISTOGRAMCNV_H
