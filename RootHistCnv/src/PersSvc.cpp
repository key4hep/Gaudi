// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "RootObjAddress.h"
#include "RConverter.h"
#include "PersSvc.h"
#include "TFile.h"

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
DECLARE_COMPONENT(PersSvc)

namespace RootHistCnv {
  static std::string stat_dir = "/stat";
  static std::string undefFileName = "UndefinedROOTOutputFileName";
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::PersSvc::initialize()
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );

  StatusCode status = ConversionSvc::initialize();
  if( status.isFailure() ) return status;

  // Get my properties from the JobOptionsSvc
  if (setProperties().isFailure()) {
    log << MSG::ERROR << "Could not set my properties" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_outputEnabled) {
    // Initialize ROOT if output file name is defined
    if( undefFileName != m_defFileName ) {
      m_hfile = TFile::Open(m_defFileName.c_str(),"RECREATE","GAUDI Histograms");
    } else {
      m_hfile = 0;
    }
    log << MSG::INFO << "Writing ROOT histograms to: " << m_defFileName
        << endmsg;
  }
  else {
    log << MSG::INFO << "Writing ROOT histograms disabled." << endmsg;
  }
  return StatusCode(StatusCode::SUCCESS,true);
}


//-----------------------------------------------------------------------------
StatusCode RootHistCnv::PersSvc::finalize()
//-----------------------------------------------------------------------------
{
  // Close ROOT only if the output file name is defined
  MsgStream log( msgSvc(), name() );
  log << MSG::DEBUG << "RootHistCnv::PersSvc::finalize()" << endmsg;
  if( undefFileName != m_defFileName ) {
    m_hfile->Write("",TObject::kOverwrite);
    m_hfile->Close();
  }
  return ConversionSvc::finalize();
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::PersSvc::createRep(DataObject* pObject,
                                           IOpaqueAddress*& refpAddress)
//-----------------------------------------------------------------------------
{
  // There are objects in the HDS to be stored
  if( m_outputEnabled && undefFileName != m_defFileName )  {
    SmartDataPtr<DataObject> top(dataProvider(), stat_dir);
    if ( 0 != top )    {
      IRegistry* pReg = top->registry();
      if ( pReg )   {
        if ( top.ptr() == pObject )   {
          TDirectory* pDir = m_hfile;
          refpAddress = new RootObjAddress( repSvcType(),
                                            CLID_DataObject,
                                            stat_dir,
                                            m_defFileName,
                                            long(pDir),
                                            long(0));
          return StatusCode::SUCCESS;
        }
        else    {
          StatusCode sc = ConversionSvc::createRep(pObject, refpAddress);
          if( sc.isFailure() )   {
            MsgStream log( msgSvc(), name() );
            log << MSG::ERROR
                << "Error while creating persistent Histogram:"
                << pReg->identifier()
                << endmsg;
          }
          return sc;
        }
      }
    }
    MsgStream err( msgSvc(), name() );
    err << MSG::ERROR
        << "Internal error while creating Histogram persistent representations"
        << endmsg;
    return StatusCode::FAILURE;
  } else {
    if (m_outputEnabled && !m_prtWar) {
      m_prtWar = true;
      MsgStream log( msgSvc(), name() );
      log << MSG::WARNING
	  << "no ROOT output file name, "
	  << "Histograms cannot be persistified" << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
RootHistCnv::PersSvc::PersSvc(const std::string& name, ISvcLocator* svc)
//-----------------------------------------------------------------------------
: ConversionSvc(name, svc, ROOT_StorageType), m_hfile(0), m_prtWar(false) {
  declareProperty("OutputFile", m_defFileName = undefFileName);
  declareProperty("ForceAlphaIds", m_alphaIds = false);
  declareProperty("OutputEnabled", m_outputEnabled = true,
                  "Flag to enable/disable the output to file.");
}

//-----------------------------------------------------------------------------
RootHistCnv::PersSvc::~PersSvc()
//-----------------------------------------------------------------------------
{
  if ( m_hfile != 0 ) {
    delete m_hfile;
    m_hfile = 0;
  }
}
