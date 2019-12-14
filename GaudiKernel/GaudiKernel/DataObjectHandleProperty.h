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
#ifndef GAUDIKERNEL_DATAOBJECTHANDLEPROPERTY_H
#define GAUDIKERNEL_DATAOBJECTHANDLEPROPERTY_H 1

#include "GaudiKernel/Property.h"
#include "GaudiKernel/StatusCode.h"

#include <iostream>

class DataObjectHandleBase;

//---------------------------------------------------------------------------

/** DataObjectHandleProperty.h GaudiKernel/DataObjectHandleProperty.h
 *
 * Helper class to manage DataObjectHandles and DataObjectHandleColls as properties
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class GAUDI_API DataObjectHandleProperty : public PropertyWithHandlers<> {
public:
  DataObjectHandleProperty( const std::string& name, DataObjectHandleBase& ref );
  DataObjectHandleProperty& operator=( const DataObjectHandleBase& value );

  DataObjectHandleProperty*   clone() const override;
  bool                        load( Property& destination ) const override;
  bool                        assign( const Property& source ) override;
  std::string                 toString() const override;
  void                        toStream( std::ostream& out ) const override;
  StatusCode                  fromString( const std::string& s ) override;
  const DataObjectHandleBase& value() const;
  bool                        setValue( const DataObjectHandleBase& value );

private:
  /** Pointer to the real property. Reference would be better,
   *  but Reflex does not support references yet
   */
  DataObjectHandleBase* m_pValue;
};

#endif
