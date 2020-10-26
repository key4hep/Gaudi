/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
