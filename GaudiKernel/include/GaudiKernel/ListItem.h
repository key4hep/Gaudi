/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_LISTITEM_H
#define GAUDIKERNEL_LISTITEM_H

// Include files
#if !defined( GAUDI_V22_API ) || defined( G22_NO_DEPRECATED )
#  include <GaudiKernel/TypeNameString.h>
typedef Gaudi::Utils::TypeNameString ListItem;
#else
#  error "Deprecated header file, use TypeNameString.h instead"
#endif

#endif /* GAUDIKERNEL_LISTITEM_H */
