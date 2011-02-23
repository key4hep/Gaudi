// $Id: PersSvc.h,v 1.5 2006/11/30 15:04:07 mato Exp $
#ifndef ROOTHISTCNV_PERSSVC_H
#define ROOTHISTCNV_PERSSVC_H 1

// Include files
#include "GaudiKernel/ConversionSvc.h"

// Forward declarations
template <class TYPE> class SvcFactory;
class TFile;


namespace RootHistCnv {

/** @class RootHistCnv::PersSvc PersSvc.h

    Persistency service - to store histograms in ROOT format
    @author Charles Leggett
*/

  class PersSvc : public ConversionSvc {

    friend class SvcFactory<PersSvc>;

  public:
    /// Initialise the service
    virtual StatusCode initialize();

    /// Finalise the service
    virtual StatusCode finalize();

    /// Convert a collection of transient data objects into another representation
    virtual StatusCode createRep(DataObject* pObject, IOpaqueAddress*& refpAddress);

    /// Standard constructor
    PersSvc( const std::string& name, ISvcLocator* svc );

    /// Standard destructor
    virtual ~PersSvc();

  private:
    std::string m_defFileName;  ///< Default file name
    TFile *m_hfile;             ///< Pointer to the ROOT file
    bool m_prtWar;              ///< Already printed a Warning
    bool m_alphaIds;            ///< Force alphabetic histograms/ntuple IDs
    bool m_outputEnabled;       ///< Flag to enable/disable the output to file
  };

}    // namespace RootHistCnv

#endif    // ROOTHISTCNV_PERSSVC_H
