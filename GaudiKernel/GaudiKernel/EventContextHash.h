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
#ifndef __EVENT_CONTEXT_HASH_H
#define __EVENT_CONTEXT_HASH_H
#include "GaudiKernel/EventContext.h"
/**
 * @class EventContextHash Create a hash from EventContext so
 * EvenContexts can be used as keys in unordered maps. Hash is crated
 * by combining event number and event slot using the algorithm from
 * boost::hash_combine()
 *
 * @author Sami Kama
 */

class EventContextHash {
public:
  size_t hash( const EventContext& ctx ) const {
    size_t en = ctx.evt();
    size_t es = ctx.slot();
    return en ^ ( es + ( en << 6 ) + ( en >> 2 ) );
  }
  bool equal( const EventContext& a, const EventContext& b ) const { return hash( a ) == hash( b ); }

  std::size_t operator()( const EventContext& ctx ) const { return hash( ctx ); }
  bool        operator()( const EventContext& ctx, const EventContext& ctxb ) const { return equal( ctx, ctxb ); }
};

#endif
