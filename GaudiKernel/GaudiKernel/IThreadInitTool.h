#ifndef GAUDIKERNEL_ITHREADINITTOOL_H
#define GAUDIKERNEL_ITHREADINITTOOL_H 1

#include "GaudiKernel/IAlgTool.h"

//---------------------------------------------------------------------------

/** IThreadInitTool GaudiKernel/IThreadInitTool.h
 *
 * Abstract interface for AlgTools to do thread local initialization
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class IThreadInitTool : virtual public IAlgTool {
public:

  DeclareInterfaceID(IThreadInitTool, 1, 0);

  virtual void initThread() = 0;

  virtual void terminateThread() = 0;

  virtual unsigned int nInit() const = 0;

};
#endif
