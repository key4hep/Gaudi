/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Include files
#include "RConverter.h"
#include "RootObjAddress.h"
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/SmartDataPtr.h>
#include <TFile.h>

namespace RootHistCnv {
  static std::string stat_dir      = "/stat";
  static std::string undefFileName = "UndefinedROOTOutputFileName";
} // namespace RootHistCnv

#include "PersSvc.h"

//-----------------------------------------------------------------------------
//
// Implementation of class :  RootHistCnv::PersSvc
//
// Author :                   Charles Leggett
//
//-----------------------------------------------------------------------------

// Instantiation of a static factory class used by clients to create
// instances of this service
using RootHistCnv::PersSvc;
DECLARE_COMPONENT( PersSvc )

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::PersSvc::initialize()
//-----------------------------------------------------------------------------
{

  StatusCode status = ConversionSvc::initialize();
  if ( status.isFailure() ) return status;

  if ( m_outputEnabled ) {
    // Initialize ROOT if output file name is defined
    if ( undefFileName != m_defFileName ) {
      m_hfile.reset( TFile::Open( m_defFileName.value().c_str(), "RECREATE", "GAUDI Histograms" ) );
      info() << "Writing ROOT histograms to: " << m_defFileName.value() << endmsg;
    } else {
      m_hfile.reset();
      // assume using another output system (Gaudi::Histograming::Sink::Base) to write hists
      // if default file name still set
    }
  } else {
    info() << "Writing ROOT histograms disabled." << endmsg;
  }
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::PersSvc::finalize()
//-----------------------------------------------------------------------------
{
  // Close ROOT only if the output file name is defined
  debug() << "RootHistCnv::PersSvc::finalize()" << endmsg;
  if ( m_outputEnabled ) {
    if ( m_hfile.get() && undefFileName != m_defFileName ) {
      m_hfile->Write( "", TObject::kOverwrite );
      m_hfile->Close();
    }
  }
  return ConversionSvc::finalize();
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::PersSvc::createRep( DataObject* pObject, IOpaqueAddress*& refpAddress )
//-----------------------------------------------------------------------------
{
  // There are objects in the HDS to be stored
  if ( m_outputEnabled && undefFileName != m_defFileName ) {
    SmartDataPtr<DataObject> top( dataProvider(), stat_dir );
    if ( top ) {
      IRegistry* pReg = top->registry();
      if ( pReg ) {
        if ( top.ptr() == pObject ) {
          TDirectory* pDir = m_hfile.get();
          refpAddress      = new RootObjAddress( repSvcType(), CLID_DataObject, stat_dir, m_defFileName.value(),
                                                 long( pDir ), long( 0 ) );
          return StatusCode::SUCCESS;
        } else {
          StatusCode sc = ConversionSvc::createRep( pObject, refpAddress );
          if ( sc.isFailure() ) {
            error() << "Error while creating persistent Histogram:" << pReg->identifier() << endmsg;
          }
          return sc;
        }
      }
    }
    error() << "Internal error while creating Histogram persistent representations" << endmsg;
    return StatusCode::FAILURE;
  } else {
    if ( m_outputEnabled && !m_prtWar ) {
      m_prtWar = true;
      // if undefined this may be due to using RootHistSink and not a warning
      if ( undefFileName != m_defFileName ) {
        warning() << "no ROOT output file name, "
                  << "Histograms cannot be persistified" << endmsg;
      }
    }
  }
  return StatusCode::SUCCESS;
}
