#ifndef GAUDICOMMONSVC_HISTOGRAMUTILITY_H
#define GAUDICOMMONSVC_HISTOGRAMUTILITY_H 1

#include "GaudiKernel/HistogramBase.h"

namespace Gaudi {
  template <class Q, class T>
  T* getRepresentation( const Q& hist ) {
    auto p = dynamic_cast<const HistogramBase*>( &hist );
    return p ? dynamic_cast<T*>( p->representation() ) : nullptr;
  }
} // namespace Gaudi

#endif
