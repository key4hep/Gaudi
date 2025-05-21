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
//====================================================================
//  Tag collection service definition
//--------------------------------------------------------------------
//
//  Package    : GaudiSvc ( The LHCb Offline System)
//  Author     : M.Frank
//====================================================================
#pragma once

// Framework include files
#include "NTupleSvc.h"

/** Gaudi tag collection service definition

    Description:
    See the base class definition files for more detailed documentation
    of the implementing class.
    The basic functionality of the implementation is enhanced to
    allow for more general N-tuples, which can be used also
    as event tag collections.

    @Author  M.Frank
    @Version 1.0
*/
class TagCollectionSvc : public NTupleSvc {
public:
  /// unhides some NTupleSvc methods
  using NTupleSvc::connect;

  /// Standard Constructor
  TagCollectionSvc( const std::string& name, ISvcLocator* svc );
  /// Add file to list I/O list
protected:
  StatusCode connect( const std::string& ident, std::string& logname ) override;
  /** Create conversion service.
      @param nam     Name of the service to be created
      @param svc     Service type if sepecified in the options
      @param props
  */
  virtual StatusCode createService( const std::string& nam, const std::string& typ, const std::vector<Prop>& props,
                                    IConversionSvc*& pSvc );
};
