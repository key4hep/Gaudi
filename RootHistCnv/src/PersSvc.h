#ifndef ROOTHISTCNV_PERSSVC_H
#define ROOTHISTCNV_PERSSVC_H 1

// Include files
#include "GaudiKernel/ConversionSvc.h"

// Forward declarations
class TFile;

namespace RootHistCnv
{

  /** @class RootHistCnv::PersSvc PersSvc.h

      Persistency service - to store histograms in ROOT format
      @author Charles Leggett
  */

  class PersSvc : public ConversionSvc
  {
  public:
    /// Initialise the service
    StatusCode initialize() override;

    /// Finalise the service
    StatusCode finalize() override;

    /// Convert a collection of transient data objects into another representation
    StatusCode createRep( DataObject* pObject, IOpaqueAddress*& refpAddress ) override;

    /// Standard constructor
    PersSvc( const std::string& name, ISvcLocator* svc ) : ConversionSvc( name, svc, ROOT_StorageType ) {}

  private:
    // undefFileName is defined in PersSvc.cpp
    Gaudi::Property<std::string> m_defFileName{this, "OutputFile", undefFileName, "default file name"};
    Gaudi::Property<bool>        m_alphaIds{this, "ForceAlphaIds", false, "force alphabetic histograms/ntuple IDs"};
    Gaudi::Property<bool> m_outputEnabled{this, "OutputEnabled", true, "Flag to enable/disable the output to file."};

    std::unique_ptr<TFile> m_hfile;          ///< Pointer to the ROOT file
    bool                   m_prtWar = false; ///< Already printed a Warning
  };

} // namespace RootHistCnv

#endif // ROOTHISTCNV_PERSSVC_H
