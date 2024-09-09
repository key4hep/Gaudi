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
#include "ParsersStandardSet.h"

#include <string>

PARSERS_DEF_FOR_SET( bool )
PARSERS_DEF_FOR_SET( char )
PARSERS_DEF_FOR_SET( unsigned char )
PARSERS_DEF_FOR_SET( signed char )

PARSERS_DEF_FOR_SET( int )
PARSERS_DEF_FOR_SET( short )
PARSERS_DEF_FOR_SET( unsigned short )
PARSERS_DEF_FOR_SET( unsigned int )
PARSERS_DEF_FOR_SET( long )
PARSERS_DEF_FOR_SET( unsigned long )
PARSERS_DEF_FOR_SET( long long )
PARSERS_DEF_FOR_SET( unsigned long long )

PARSERS_DEF_FOR_SET( double )
PARSERS_DEF_FOR_SET( float )
PARSERS_DEF_FOR_SET( long double )

PARSERS_DEF_FOR_SET( std::string )
