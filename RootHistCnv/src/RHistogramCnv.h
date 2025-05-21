/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// Include files
#include "RConverter.h"
#include "RootObjAddress.h"
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/HistogramBase.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/ObjectFactory.h>
#include <GaudiKernel/SmartIF.h>
#include <TArray.h>
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
  template <typename T, typename S, typename Q>
  class RHistogramCnv : public RConverter {
    template <typename CLASS>
    class TTH {
    public:
      template <typename INPUT>
      TTH( INPUT* i ) : m_c( dynamic_cast<CLASS*>( i ) ) {}
      template <typename INPUT>
      bool CopyH( INPUT& i ) {
        if ( m_c ) { m_c->Copy( i ); }
        return m_c != nullptr;
      }

    private:
      CLASS* m_c = nullptr;
    };

  public:
    /// Create the transient representation of an object.
    StatusCode createObj( IOpaqueAddress* pAddr, DataObject*& refpObj ) override {
      refpObj           = DataObjFactory::create( objType() ).release();
      RootObjAddress* r = dynamic_cast<RootObjAddress*>( pAddr );
      Q*              h = dynamic_cast<Q*>( refpObj );
      if ( r && h ) {
        // Need to flip representation .... clumsy for the time being, because
        // THXY constructor has no "generic" copy constructor
        auto s = dynamic_cast<S*>( r->tObj() );
        if ( s ) {
          auto a = dynamic_cast<TArray*>( s );
          if ( a ) {
            auto p  = std::make_unique<T>();
            auto ok = TTH<S>( s ).CopyH( *p );
            if ( ok ) {
              p->Set( a->GetSize() );
              p->Reset();
              p->Add( s );
              h->adoptRepresentation( p.release() );
              return StatusCode::SUCCESS;
            }
          }
        }
      }
      return error( "Cannot create histogram - invalid address." );
    }
    /// Update the transient object from the other representation.
    StatusCode updateObj( IOpaqueAddress* /* pAddr */, DataObject* /* pObj */ ) override { return StatusCode::SUCCESS; }
    /// Create the persistent representation of the histogram object.
    TObject* createPersistent( DataObject* pObj ) override {
      auto h = dynamic_cast<Q*>( pObj );
      if ( h ) {
        auto r = dynamic_cast<T*>( h->representation() );
        if ( r ) {
          auto a = dynamic_cast<TArray*>( r );
          if ( a ) {
            auto c  = std::make_unique<T>();
            auto ok = TTH<S>( r ).CopyH( *c );
            if ( ok ) {
              c->Set( a->GetSize() );
              c->Reset();
              c->Add( r );
              c->SetName( pObj->registry()->name().c_str() + 1 );
              return c.release();
            }
          }
        }
      }
      error( "Histogram object is invalid!" ).ignore();
      return nullptr;
    }
    /// Inquire class type
    static const CLID& classID();
    /// Standard constructor
    RHistogramCnv( ISvcLocator* svc ) : RConverter( classID(), svc ) {}
  };
} // namespace RootHistCnv
