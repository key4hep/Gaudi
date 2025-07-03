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

#include <GaudiKernel/DataHandle.h>
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/INamedInterface.h>

#include <set>
#include <vector>

class IDataHandleVisitor;

class GAUDI_API IDataHandleHolder : virtual public INamedInterface {
public:
  virtual std::vector<Gaudi::DataHandle*> inputHandles() const  = 0;
  virtual std::vector<Gaudi::DataHandle*> outputHandles() const = 0;

  virtual const DataObjIDColl& extraInputDeps() const  = 0;
  virtual const DataObjIDColl& extraOutputDeps() const = 0;

  virtual void acceptDHVisitor( IDataHandleVisitor* ) const = 0;

  virtual const DataObjIDColl& inputDataObjs() const  = 0;
  virtual const DataObjIDColl& outputDataObjs() const = 0;

  virtual void addDependency( const DataObjID&, const Gaudi::DataHandle::Mode& ) = 0;

  DeclareInterfaceID( IDataHandleHolder, 1, 0 );

  virtual void declare( Gaudi::DataHandle& )     = 0;
  virtual void renounce( Gaudi::DataHandle& )    = 0;
  virtual bool renounceInput( const DataObjID& ) = 0;
};

class IDataHandleVisitor {
public:
  virtual ~IDataHandleVisitor()                  = default;
  virtual void visit( const IDataHandleHolder* ) = 0;
};
