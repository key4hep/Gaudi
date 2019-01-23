// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "RConverter.h"
#include "RootObjAddress.h"
#include "TFile.h"

namespace RootHistCnv
{
  static std::string stat_dir      = "/stat";
  static std::string undefFileName = "UndefinedROOTOutputFileName";
}

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

  // Get my properties from the JobOptionsSvc
  if ( setProperties().isFailure() ) {
    error() << "Could not set my properties" << endmsg;
    return StatusCode::FAILURE;
  }
  if ( m_outputEnabled ) {
    // Initialize ROOT if output file name is defined
    if ( undefFileName != m_defFileName ) {
      m_hfile.reset( TFile::Open( m_defFileName.value().c_str(), "RECREATE", "GAUDI Histograms" ) );
    } else {
      m_hfile.reset();
    }
    info() << "Writing ROOT histograms to: " << m_defFileName.value() << endmsg;
  } else {
    info() << "Writing ROOT histograms disabled." << endmsg;
  }
  return StatusCode( StatusCode::SUCCESS, true );
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::PersSvc::finalize()
//-----------------------------------------------------------------------------
{
  // Close ROOT only if the output file name is defined
  debug() << "RootHistCnv::PersSvc::finalize()" << endmsg;
  if ( m_outputEnabled ) {
    if ( undefFileName != m_defFileName ) {
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
      warning() << "no ROOT output file name, "
                << "Histograms cannot be persistified" << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}
