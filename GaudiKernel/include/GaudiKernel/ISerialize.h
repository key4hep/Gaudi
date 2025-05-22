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

#include <GaudiKernel/IInterface.h>

class StreamBuffer;

/** @class ISerialize ISerialize.h GaudiKernel/ISerialize.h

    Object serialization interface definition.
    Definition of a interface to save and restore objects.

    @author M.Frank
*/
class GAUDI_API ISerialize : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( ISerialize, 2, 0 );

  /** Input serialization from stream buffer. Restores the status of the object.
      @param   str    Reference to Streambuffer to be filled
      @return  Reference to filled stream buffer
  */
  virtual StreamBuffer& serialize( StreamBuffer& str ) = 0;
  /** Output serialization to stream buffer. Saves the status of the object.
      @param   str    Reference to Streambuffer to be read
      @return  Reference to read stream buffer
  */
  virtual StreamBuffer& serialize( StreamBuffer& str ) const = 0;
};
