#ifndef HISTOGRAMSVC_BASE_H
#define HISTOGRAMSVC_BASE_H

#include <iostream>
#include "GaudiKernel/Kernel.h"

class TObject;

/*
 *    Gaudi namespace declaration
 */
namespace Gaudi {

  /**@class HistogramBase HistogramBase.h HistogramSvc/HistogramBase.h
    *
    *   Common base class for all histograms
    *   Use is solely functional to minimize dynamic_casts inside HistogramSvc
    *
    *   @author  M.Frank
    *   @version 1.0
    */
  class GAUDI_API HistogramBase {
  public:
    /// ROOT object implementation
    virtual TObject* representation() const = 0;
    /// Adopt ROOT histogram representation
    virtual void adoptRepresentation(TObject*rep) = 0;
    /// Print histogram to output stream
    virtual std::ostream& print(std::ostream& s) const = 0;
    /// Write (binary) histogram to output stream
    virtual std::ostream& write(std::ostream& s) const = 0;
    /// Write (binary) histogram to file
    virtual int           write(const char* file_name) const = 0;
    virtual ~HistogramBase();
  }; // class

} // namespace Gaudi

#endif // HISTOGRAMSVC_BASE_H

