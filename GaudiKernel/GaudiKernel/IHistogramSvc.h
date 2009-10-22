// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IHistogramSvc.h,v 1.13 2008/10/09 09:59:14 marcocle Exp $
#ifndef GAUDIKERNEL_IHISTOGRAMSVC_H
#define GAUDIKERNEL_IHISTOGRAMSVC_H


// Include files
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IService.h"
#include <string>
#include <vector>
#include <iostream>

// Forward declarations
class DataObject;

// added to allow use of AIDA in applications
// without prefix AIDA:: (as before)
namespace AIDA {
  class IBaseHistogram;
  class IHistogram;
  class IHistogram1D;
  class IProfile1D;
  class IHistogram2D;
  class IProfile2D;
  class IHistogram3D;
  class IHistogramFactory;
  class IAnnotation;
  class IAxis;
}

using AIDA::IHistogramFactory;
using AIDA::IBaseHistogram;
using AIDA::IHistogram;
using AIDA::IHistogram1D;
using AIDA::IProfile1D;
using AIDA::IHistogram2D;
using AIDA::IProfile2D;
using AIDA::IHistogram3D;
using AIDA::IAxis;
using AIDA::IAnnotation;

/** @class IHistogramSvc IHistogramSvc.h GaudiKernel/IHistogramSvc.h

    Definition of the IHistogramSvc interface class

    @author Pavel Binko and Anatael Cabrera
*/
class GAUDI_API IHistogramSvc: virtual public IDataProviderSvc
{
public:
  /// InterfaceID
  DeclareInterfaceID(IHistogramSvc,5,0);

  /// Retrieve the AIDA HistogramFactory interface
  virtual AIDA::IHistogramFactory* histogramFactory() = 0;

  /** Book histogram and register it with the histogram data store
      @param fullPath    Full path to the node of the object.
                         The basename (last part of the fullPath)
                         has to be an integer number
                        (otherwise conversion to HBOOK is not possible)
      @param dirPath     Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pParent     Pointer to parent node
      @param relPath     Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param binsX/Y     Number of bins on the axis X/Y
      @param lowX/Y      Lower histogram edge on the axis X/Y
      @param highX/Y     Upper histogram edge on the axis X/Y
  */
  // ==========================================================================
  // Book 1D histogram with fixed binning
  // ==========================================================================

  virtual AIDA::IHistogram1D* book( const std::string& fullPath,
                              const std::string& title,
                              int binsX, double lowX, double highX )          = 0;
  virtual AIDA::IHistogram1D* book( const std::string& dirPath,
                              const std::string& relPath,
                              const std::string& title,
                              int binsX, double lowX, double highX )          = 0;
  virtual AIDA::IHistogram1D* book( const std::string& dirPath,
                              int hID,
                              const std::string& title,
                              int binsX, double lowX, double highX )          = 0;
  virtual AIDA::IHistogram1D* book( DataObject* pParent,
                              const std::string& relPath,
                              const std::string& title,
                              int binsX, double lowX, double highX )          = 0;
  virtual AIDA::IHistogram1D* book( DataObject* pParent,
                              int hID,
                              const std::string& title,
                              int binsX, double lowX, double highX )          = 0;

  // ==========================================================================
  // Book 1D profile histogram with fixed binning
  // ==========================================================================

  virtual AIDA::IProfile1D* bookProf
  ( const std::string& fullPath,
    const std::string& title,
    int binsX, double lowX, double highX ,
    const std::string& opt = "" )          = 0;
  virtual AIDA::IProfile1D* bookProf
  ( const std::string& dirPath,
    const std::string& relPath,
    const std::string& title,
    int binsX, double lowX, double highX ,
    const std::string& opt = "" )          = 0;
  virtual AIDA::IProfile1D* bookProf
  ( const std::string& dirPath,
    int hID,
    const std::string& title,
    int binsX, double lowX, double highX ,
    const std::string& opt = "" )          = 0;
  virtual AIDA::IProfile1D* bookProf
  ( DataObject* pParent,
    const std::string& relPath,
    const std::string& title,
    int binsX, double lowX, double highX ,
    const std::string& opt = "" )          = 0;
  virtual AIDA::IProfile1D* bookProf
  ( DataObject* pParent,
    int hID,
    const std::string& title,
    int binsX, double lowX, double highX ,
    const std::string& opt = "" )          = 0;

  // ==========================================================================
  // Book 1D profile histogram with fixed binning & Y-limits
  // ==========================================================================

  virtual AIDA::IProfile1D* bookProf
  ( const std::string& fullPath ,
    const std::string& title    ,
    int                binsX    ,
    double lowX , double highX  ,
    double lowY , double highY  ,
    const std::string& opt = "s" )          = 0;
  virtual AIDA::IProfile1D* bookProf
  ( const std::string& dirPath  ,
    const std::string& relPath  ,
    const std::string& title    ,
    int binsX                   ,
    double lowX , double highX  ,
    double lowY , double highY  ,
    const std::string& opt = "s" )          = 0;
  virtual AIDA::IProfile1D* bookProf
  ( const std::string& dirPath  ,
    int hID                     ,
    const std::string& title    ,
    int binsX                   ,
    double lowX , double highX  ,
    double lowY , double highY  ,
    const std::string& opt = "s" )          = 0;
  virtual AIDA::IProfile1D* bookProf
  ( DataObject*        pParent  ,
    const std::string& relPath  ,
    const std::string& title    ,
    int binsX                   ,
    double lowX , double highX  ,
    double lowY , double highY  ,
    const std::string& opt = "s" )          = 0;
  virtual AIDA::IProfile1D* bookProf
  ( DataObject*         pParent ,
    int                 hID     ,
    const std::string&  title   ,
    int binsX                   ,
    double lowX , double highX  ,
    double lowY , double highY  ,
    const std::string& opt = "s" )          = 0;

  // ==========================================================================
  // Book 1D histogram with variable binning
  // ==========================================================================

  virtual AIDA::IHistogram1D* book( const std::string& fullPath,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;
  virtual AIDA::IHistogram1D* book( const std::string& dirPath,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;
  virtual AIDA::IHistogram1D* book( const std::string& dirPath,
                              int hID,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;
  virtual AIDA::IHistogram1D* book( DataObject* pParent,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;
  virtual AIDA::IHistogram1D* book( DataObject* pParent,
                              int hID,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;

  // ==========================================================================
  // Book 1D profile histogram with variable binning
  // ==========================================================================

  virtual AIDA::IProfile1D* bookProf( const std::string& fullPath,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;
  virtual AIDA::IProfile1D* bookProf( const std::string& dirPath,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;
  virtual AIDA::IProfile1D* bookProf( const std::string& dirPath,
                              int hID,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;
  virtual AIDA::IProfile1D* bookProf( DataObject* pParent,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;
  virtual AIDA::IProfile1D* bookProf( DataObject* pParent,
                              int hID,
                              const std::string& title,
                              std::vector<double> edges )                   = 0;

  // ==========================================================================
  // Book 2D histogram with fixed binning
  // ==========================================================================

  virtual AIDA::IHistogram2D* book( const std::string& fullPath,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;
  virtual AIDA::IHistogram2D* book( const std::string& dirPath,
                              const std::string& relPath,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;
  virtual AIDA::IHistogram2D* book( const std::string& dirPath,
                              int hID,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;
  virtual AIDA::IHistogram2D* book( DataObject* pParent,
                              const std::string& relPath,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;
  virtual AIDA::IHistogram2D* book( DataObject* pParent,
                              int hID,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;

  // ==========================================================================
  // Book 2D profile histogram with fixed binning
  // ==========================================================================

  virtual AIDA::IProfile2D* bookProf( const std::string& fullPath,
			      const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;
  virtual AIDA::IProfile2D* bookProf( const std::string& dirPath,
                              const std::string& relPath,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;
  virtual AIDA::IProfile2D* bookProf( const std::string& dirPath,
                              int hID,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;
  virtual AIDA::IProfile2D* bookProf( DataObject* pParent,
                              const std::string& relPath,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;
  virtual AIDA::IProfile2D* bookProf( DataObject* pParent,
                              int hID,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY )          = 0;

  // ==========================================================================
  // Book 2D histogram with variable binning
  // ==========================================================================

  virtual AIDA::IHistogram2D* book( const std::string& fullPath,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;
  virtual AIDA::IHistogram2D* book( const std::string& dirPath,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;
  virtual AIDA::IHistogram2D* book( const std::string& dirPath,
                              int hID,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;
  virtual AIDA::IHistogram2D* book( DataObject* pParent,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;
  virtual AIDA::IHistogram2D* book( DataObject* pParent,
                              int hID,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;


  // ==========================================================================
  // Book 2D profile histogram with variable binning
  // ==========================================================================

  virtual AIDA::IProfile2D* bookProf( const std::string& fullPath,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;
  virtual AIDA::IProfile2D* bookProf( const std::string& dirPath,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;
  virtual AIDA::IProfile2D* bookProf( const std::string& dirPath,
                              int hID,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;
  virtual AIDA::IProfile2D* bookProf( DataObject* pParent,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;
  virtual AIDA::IProfile2D* bookProf( DataObject* pParent,
                              int hID,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY )                    = 0;


  // ==========================================================================
  // Book 3D histogram with fixed binning
  // ==========================================================================

  virtual AIDA::IHistogram3D* book( const std::string& fullPath,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY,
                              int binsZ, double lowZ, double highZ )          = 0;
  virtual AIDA::IHistogram3D* book( const std::string& dirPath,
                              const std::string& relPath,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY,
                              int binsZ, double lowZ, double highZ )          = 0;
  virtual AIDA::IHistogram3D* book( const std::string& dirPath,
                              int hID,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY,
                              int binsZ, double lowZ, double highZ )          = 0;
  virtual AIDA::IHistogram3D* book( DataObject* pParent,
                              const std::string& relPath,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY,
                              int binsZ, double lowZ, double highZ )          = 0;
  virtual AIDA::IHistogram3D* book( DataObject* pParent,
                              int hID,
                              const std::string& title,
                              int binsX, double lowX, double highX,
                              int binsY, double lowY, double highY,
                              int binsZ, double lowZ, double highZ )          = 0;

  // ==========================================================================
  // Book 3D histogram with variable binning
  // ==========================================================================

  virtual AIDA::IHistogram3D* book( const std::string& fullPath,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY,
                              std::vector<double> edgesZ )                    = 0;
  virtual AIDA::IHistogram3D* book( const std::string& dirPath,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY,
                              std::vector<double> edgesZ )                    = 0;
  virtual AIDA::IHistogram3D* book( const std::string& dirPath,
                              int hID,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY,
                              std::vector<double> edgesZ )                    = 0;
  virtual AIDA::IHistogram3D* book( DataObject* pParent,
                              const std::string& relPath,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY,
                              std::vector<double> edgesZ )                    = 0;
  virtual AIDA::IHistogram3D* book( DataObject* pParent,
                              int hID,
                              const std::string& title,
                              std::vector<double> edgesX,
                              std::vector<double> edgesY,
                              std::vector<double> edgesZ )                    = 0;

  // ==========================================================================
  // Register histogram with the data store
  // ==========================================================================

  virtual StatusCode registerObject( const std::string& fullPath,
                                     AIDA::IBaseHistogram* hObj )                     = 0;
  // ---------------------------
  virtual StatusCode registerObject( const std::string& parentPath,
                                     const std::string& objPath,
                                     AIDA::IBaseHistogram* hObj )                     = 0;
  // ---------------------------
  virtual StatusCode registerObject( const std::string& parentPath,
                                     int item, AIDA::IBaseHistogram* hObj )           = 0;
  // ---------------------------
  virtual StatusCode registerObject( DataObject* parentObj,
                                     const std::string& objPath,
                                     AIDA::IBaseHistogram* hObj )                     = 0;
  virtual StatusCode registerObject( AIDA::IBaseHistogram* parentObj,
                                     const std::string& objPath,
                                     AIDA::IBaseHistogram* hObj )                     = 0;
  // ---------------------------
  virtual StatusCode registerObject( DataObject* parentObj,
                                     int item,
                                     AIDA::IBaseHistogram* hObj )                     = 0;
  virtual StatusCode registerObject( AIDA::IBaseHistogram* parentObj,
                                     int item,
                                     AIDA::IBaseHistogram* hObj )                     = 0;


  // ==========================================================================
  // Unregister histogram from the data store
  // ==========================================================================

  virtual StatusCode unregisterObject( AIDA::IBaseHistogram* hObj )                   = 0;
  // ---------------------------
  virtual StatusCode unregisterObject( AIDA::IBaseHistogram* hObj,
                                       const std::string& objectPath )      = 0;
  // ---------------------------
  virtual StatusCode unregisterObject( AIDA::IBaseHistogram* hObj,
                                       int item )                           = 0;


  // ==========================================================================
  // Retrieve histogram from data store
  // ==========================================================================

  virtual StatusCode retrieveObject( IRegistry* pDirectory,
                                     const std::string& path,
                                     AIDA::IHistogram1D*& h1dObj )                = 0;
  virtual StatusCode retrieveObject( IRegistry* pDirectory,
                                     const std::string& path,
                                     AIDA::IProfile1D*& h1dObj )                  = 0;
  virtual StatusCode retrieveObject( IRegistry* pDirectory,
                                     const std::string& path,
                                     AIDA::IHistogram2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( IRegistry* pDirectory,
                                     const std::string& path,
                                     AIDA::IProfile2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( IRegistry* pDirectory,
                                     const std::string& path,
                                     AIDA::IHistogram3D*& h3dObj )                = 0;
  // ---------------------------
  virtual StatusCode retrieveObject( const std::string& fullPath,
                                     AIDA::IHistogram1D*& h1dObj )                = 0;
  virtual StatusCode retrieveObject( const std::string& fullPath,
                                     AIDA::IProfile1D*& h1dObj )                  = 0;
  virtual StatusCode retrieveObject( const std::string& fullPath,
                                     AIDA::IHistogram2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( const std::string& fullPath,
                                     AIDA::IProfile2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( const std::string& fullPath,
                                     AIDA::IHistogram3D*& h3dObj )                = 0;
  // ---------------------------
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     const std::string& objPath,
                                     AIDA::IHistogram1D*& h1dObj )                = 0;
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     const std::string& objPath,
                                     AIDA::IProfile1D*& h1dObj )                  = 0;
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     const std::string& objPath,
                                     AIDA::IHistogram2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     const std::string& objPath,
                                     AIDA::IProfile2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     const std::string& objPath,
                                     AIDA::IHistogram3D*& h3dObj )                = 0;
  // ---------------------------
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     int item,
                                     AIDA::IHistogram1D*& h1dObj )                = 0;
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     int item,
                                     AIDA::IProfile1D*& h1dObj )                  = 0;
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     int item,
                                     AIDA::IHistogram2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     int item,
                                     AIDA::IProfile2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( const std::string& parentPath,
                                     int item,
                                     AIDA::IHistogram3D*& h3dObj )                = 0;
  // ---------------------------
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     const std::string& objPath,
                                     AIDA::IHistogram1D*& h1dObj )                = 0;
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     const std::string& objPath,
                                     AIDA::IProfile1D*& h1dObj )                  = 0;
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     const std::string& objPath,
                                     AIDA::IHistogram2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     const std::string& objPath,
                                     AIDA::IProfile2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     const std::string& objPath,
                                     AIDA::IHistogram3D*& h3dObj )                = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     const std::string& objPath,
                                     AIDA::IHistogram1D*& h1dObj )                = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     const std::string& objPath,
                                     AIDA::IProfile1D*& h1dObj )                  = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     const std::string& objPath,
                                     AIDA::IHistogram2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     const std::string& objPath,
                                     AIDA::IProfile2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     const std::string& objPath,
                                     AIDA::IHistogram3D*& h3dObj )                = 0;
  // ---------------------------
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     int item,
                                     AIDA::IHistogram1D*& h1dObj )                = 0;
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     int item,
                                     AIDA::IProfile1D*& h1dObj )                  = 0;
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     int item,
                                     AIDA::IHistogram2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     int item,
                                     AIDA::IProfile2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( DataObject* parentObj,
                                     int item,
                                     AIDA::IHistogram3D*& h3dObj )                = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     int item,
                                     AIDA::IHistogram1D*& h1dObj )                = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     int item,
                                     AIDA::IProfile1D*& h1dObj )                  = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     int item,
                                     AIDA::IHistogram2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     int item,
                                     AIDA::IProfile2D*& h2dObj )                = 0;
  virtual StatusCode retrieveObject( AIDA::IBaseHistogram* parentObj,
                                     int item,
                                     AIDA::IHistogram3D*& h3dObj )                = 0;


  // ==========================================================================
  // Find histogram identified by its full path in the data store
  // ==========================================================================

  virtual StatusCode findObject( const std::string& fullPath,
                                 AIDA::IHistogram1D*& h1dObj )                    = 0;
  virtual StatusCode findObject( const std::string& fullPath,
                                 AIDA::IProfile1D*& h1dObj )                      = 0;
  virtual StatusCode findObject( const std::string& fullPath,
                                 AIDA::IHistogram2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( const std::string& fullPath,
                                 AIDA::IProfile2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( const std::string& fullPath,
                                 AIDA::IHistogram3D*& h3dObj )                    = 0;
  // ---------------------------
  virtual StatusCode findObject( IRegistry* pDirectory,
                                 const std::string& path,
                                 AIDA::IHistogram1D*& h1dObj )                    = 0;
  virtual StatusCode findObject( IRegistry* pDirectory,
                                 const std::string& path,
                                 AIDA::IProfile1D*& h1dObj )                      = 0;
  virtual StatusCode findObject( IRegistry* pDirectory,
                                 const std::string& path,
                                 AIDA::IHistogram2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( IRegistry* pDirectory,
                                 const std::string& path,
                                 AIDA::IProfile2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( IRegistry* pDirectory,
                                 const std::string& path,
                                 AIDA::IHistogram3D*& h2dObj )                    = 0;
  // ---------------------------
  virtual StatusCode findObject( const std::string& parentPath,
                                 const std::string& objPath,
                                 AIDA::IHistogram1D*& h1dObj )                    = 0;
  virtual StatusCode findObject( const std::string& parentPath,
                                 const std::string& objPath,
                                 AIDA::IProfile1D*& h1dObj )                      = 0;
  virtual StatusCode findObject( const std::string& parentPath,
                                 const std::string& objPath,
                                 AIDA::IHistogram2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( const std::string& parentPath,
                                 const std::string& objPath,
                                 AIDA::IProfile2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( const std::string& parentPath,
                                 const std::string& objPath,
                                 AIDA::IHistogram3D*& h3dObj )                    = 0;
  // ---------------------------
  virtual StatusCode findObject( const std::string& parentPath,
                                 int item,
                                 AIDA::IHistogram1D*& h1dObj )                    = 0;
  virtual StatusCode findObject( const std::string& parentPath,
                                 int item,
                                 AIDA::IProfile1D*& h1dObj )                      = 0;
  virtual StatusCode findObject( const std::string& parentPath,
                                 int item,
                                 AIDA::IHistogram2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( const std::string& parentPath,
                                 int item,
                                 AIDA::IProfile2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( const std::string& parentPath,
                                 int item,
                                 AIDA::IHistogram3D*& h3dObj )                    = 0;
  // ---------------------------
  virtual StatusCode findObject( DataObject* parentObj,
                                 const std::string& objPath,
                                 AIDA::IHistogram1D*& h1dObj )                    = 0;
  virtual StatusCode findObject( DataObject* parentObj,
                                 const std::string& objPath,
                                 AIDA::IProfile1D*& h1dObj )                      = 0;
  virtual StatusCode findObject( DataObject* parentObj,
                                 const std::string& objPath,
                                 AIDA::IHistogram2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( DataObject* parentObj,
                                 const std::string& objPath,
                                 AIDA::IProfile2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( DataObject* parentObj,
                                 const std::string& objPath,
                                 AIDA::IHistogram3D*& h3dObj )                    = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 const std::string& objPath,
                                 AIDA::IHistogram1D*& h1dObj )                    = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 const std::string& objPath,
                                 AIDA::IProfile1D*& h1dObj )                      = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 const std::string& objPath,
                                 AIDA::IHistogram2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 const std::string& objPath,
                                 AIDA::IProfile2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 const std::string& objPath,
                                 AIDA::IHistogram3D*& h3dObj )                    = 0;
  // ---------------------------
  virtual StatusCode findObject( DataObject* parentObj,
                                 int item,
                                 AIDA::IHistogram1D*& h1dObj )                    = 0;
  virtual StatusCode findObject( DataObject* parentObj,
                                 int item,
                                 AIDA::IProfile1D*& h1dObj )                      = 0;
  virtual StatusCode findObject( DataObject* parentObj,
                                 int item,
                                 AIDA::IHistogram2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( DataObject* parentObj,
                                 int item,
                                 AIDA::IProfile2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( DataObject* parentObj,
                                 int item,
                                 AIDA::IHistogram3D*& h3dObj )                    = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 int item,
                                 AIDA::IHistogram1D*& h1dObj )                    = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 int item,
                                 AIDA::IProfile1D*& h1dObj )                      = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 int item,
                                 AIDA::IHistogram2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 int item,
                                 AIDA::IProfile2D*& h2dObj )                    = 0;
  virtual StatusCode findObject( AIDA::IBaseHistogram* parentObj,
                                 int item,
                                 AIDA::IHistogram3D*& h3dObj )                    = 0;


  // ==========================================================================
  // ASCII output
  // ==========================================================================
  // --------------------------------------------------------------------------
  // Print functions (ASCII graphical representation)

  // Print (ASCII) the histogram into the output stream
  virtual std::ostream& print( AIDA::IBaseHistogram* h,
                               std::ostream& s = std::cout) const           = 0;

  // --------------------------------------------------------------------------
  // Write functions (ASCII table containing numbers e.g. bin height)

  /// Write (ASCII) the histogram table into the output stream
  virtual std::ostream& write( AIDA::IBaseHistogram* h,
                               std::ostream& s = std::cout) const           = 0;

  /// Write (ASCII) the histogram table into a file
  virtual int write( AIDA::IBaseHistogram* h, const char* file_name ) const       = 0;

  // ==========================================================================
  // Create all directories in a given full path
  // ==========================================================================

  virtual DataObject* createPath( const std::string& newPath )              = 0;

  /** Create a sub-directory in a directory.
      @param parentDir name of the parent directory
      @param subDir to identify the histogram object in the store
                    returns pointer to DataObject (NULL on failure)
  */
  virtual DataObject* createDirectory	(
    const std::string& parentDir,
    const std::string& subDir  ) = 0;

  /// Avoids a compiler warning about hidden functions.
  using IDataProviderSvc::registerObject;
  using IDataProviderSvc::unregisterObject;
  using IDataProviderSvc::retrieveObject;
  using IDataProviderSvc::findObject;

};


#endif	// INTERFACES_IHISTOGRAMSVC_H
