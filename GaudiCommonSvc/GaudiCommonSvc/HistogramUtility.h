#ifndef GAUDICOMMONSVC_HISTOGRAMUTILITY_H
#define GAUDICOMMONSVC_HISTOGRAMUTILITY_H 1

#include "GaudiKernel/HistogramBase.h"

namespace Gaudi {
  template <class Q, class T> T* getRepresentation(const Q& hist)  {
    T* result = nullptr;
    const HistogramBase *p = dynamic_cast<const HistogramBase*>(&hist);
    if ( p )  {
      result = dynamic_cast<T*>(p->representation());
    }
    return result;
  }
}

#endif
