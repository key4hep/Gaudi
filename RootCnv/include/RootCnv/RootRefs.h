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
//====================================================================
// Dictionary classes for internal classes used to write ROOT files.
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================
#ifndef GAUDIROOTCNV_ROOTREFS_H
#define GAUDIROOTCNV_ROOTREFS_H

// Framework include files
#include <GaudiKernel/Kernel.h>

// C++ include files
#include <string>
#include <vector>

/*
 *   Gaudi namespace declaration
 */
namespace Gaudi {

  /** @struct RootRef RootRefs.h RootCnv/RootRefs.h
   *
   * Persistent reference object.
   *
   * @author  M.Frank
   * @version 1.0
   */
  struct RootRef {
    /// Data members to define object location in the persistent world
    int dbase, container, link, clid, svc, entry;
    /// Standard constructor
    RootRef() { this->reset(); }
    /// Copy constructor
    RootRef( const RootRef& c ) = default;
    /// Assignment operator
    RootRef& operator=( const RootRef& c ) = default;
    /// Reset data content
    void reset() {
      dbase     = -1;
      container = -1;
      link      = -1;
      entry     = -1;
      svc       = 1;
      clid      = 1;
    }
  };

  /** @struct RootObjectRefs RootRefs.h RootCnv/RootRefs.h
   *
   * Persistent reference object containing all leafs and links
   * corresponding to a Gaudi DataObject.
   *
   * @author  M.Frank
   * @version 1.0
   */
  struct RootObjectRefs {
    /// The links of the link manager
    std::vector<int> links;
    /// The references corresponding to the next layer of items in the data store
    std::vector<RootRef> refs;

    /// Default constructor
    RootObjectRefs() = default;
    /// Default destructor
    ~RootObjectRefs() = default;
  };

  /** @class RootNTupleDescriptor RootRefs.h RootCnv/RootRefs.h
   *
   * Description:
   *
   * @author  M.Frank
   * @version 1.0
   */
  struct RootNTupleDescriptor {
    /// Description string
    std::string description;
    /// Optional description
    std::string optional;
    /// Identifier of description
    std::string container;
    /// Class ID of the described object
    unsigned long clid{ 1 };
    /// Standard constructor
    RootNTupleDescriptor() = default;
    /// Standard destructor
    virtual ~RootNTupleDescriptor() = default;
  };
} // namespace Gaudi

#endif // GAUDIROOTCNV_ROOTREFS_H
