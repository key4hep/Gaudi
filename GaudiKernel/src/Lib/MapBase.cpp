/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/MapBase.h>
#include <GaudiKernel/System.h>
#include <stdexcept>

/** @file
 *  Implementation file for class Gaudi::Utils::MapBase
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date 2010-02-20
 */

Gaudi::Utils::MapBase::~MapBase() {}
void Gaudi::Utils::MapBase::throw_out_of_range_exception() const {
  std::string _nam = System::typeinfoName( typeid( *this ) );
  throw std::out_of_range( _nam.c_str() );
}
