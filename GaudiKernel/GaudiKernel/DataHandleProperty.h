/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include "Gaudi/Property.h"
#include "GaudiKernel/StatusCode.h"

#include <iostream>

namespace Gaudi {
  class DataHandle;
}

//---------------------------------------------------------------------------

/** DataHandleProperty.h GaudiKernel/DataHandleProperty.h
 *
 * Helper class to manage DataHandles and DataHandleColls as properties
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class GAUDI_API DataHandleProperty : public PropertyWithHandlers<> {
public:
  DataHandleProperty( const std::string& name, Gaudi::DataHandle& ref );
  DataHandleProperty& operator=( const Gaudi::DataHandle& value );

  DataHandleProperty*      clone() const override;
  bool                     load( Gaudi::Details::PropertyBase& destination ) const override;
  bool                     assign( const Gaudi::Details::PropertyBase& source ) override;
  std::string              toString() const override;
  void                     toStream( std::ostream& out ) const override;
  StatusCode               fromString( const std::string& s ) override;
  const Gaudi::DataHandle& value() const;
  bool                     setValue( const Gaudi::DataHandle& value );

private:
  /** Pointer to the real property. Reference would be better,
   *  but Reflex does not support references yet
   */
  Gaudi::DataHandle* m_pValue = nullptr;
};
