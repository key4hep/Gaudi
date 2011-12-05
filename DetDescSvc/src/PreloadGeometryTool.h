#ifndef PRELOADGEOMETRYTOOL_H
#define PRELOADGEOMETRYTOOL_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiAlg/IGenericTool.h"            // Interface


/** @class PreloadGeometryTool PreloadGeometryTool.h
 *  Tool to traverse the full geometry tree and load it
 *
 *  @author Marco Cattaneo
 *  @date   2006-07-26
 */
class PreloadGeometryTool : public extends1<GaudiTool, IGenericTool> {
public:
  /// Standard constructor
  PreloadGeometryTool( const std::string& type,
                       const std::string& name,
                       const IInterface* parent);

  virtual ~PreloadGeometryTool( ); ///< Destructor

  void execute(); ///< Load the geometry tree
};

#endif // PRELOADGEOMETRYTOOL_H
