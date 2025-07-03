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
#pragma once
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <sstream>
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include <GaudiKernel/ContainedObject.h>
#include <GaudiKernel/DataObject.h>
// ============================================================================

namespace GaudiPython {
  /** @struct Printer Printer.h GaudiPython/Printer.h
   *
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2005-08-05
   */
  template <class TYPE>
  struct Printer {
    static std::string print( const TYPE& object ) {
      std::stringstream stream;
      stream << object << std::endl;
      return stream.str();
    };
  };
  template <>
  struct Printer<ContainedObject> {
    static std::string print( const ContainedObject& object ) {
      std::ostringstream stream;
      object.fillStream( stream );
      return stream.str();
    };
  };
  template <>
  struct Printer<DataObject> {
    static std::string print( const DataObject& type ) {
      std::ostringstream stream;
      type.fillStream( stream );
      return stream.str();
    };
  };

} // end of namespace GaudiPython
