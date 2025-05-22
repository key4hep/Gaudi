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
///////////////////////// -*- C++ -*- /////////////////////////////
// IIoComponentMgr.h
// Header file for class IIoComponentMgr
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////
#pragma once

#include <GaudiKernel/INamedInterface.h>
#include <GaudiKernel/StatusCode.h>
#include <string>

class IIoComponent;

class GAUDI_API IIoComponentMgr : virtual public INamedInterface {
public:
  DeclareInterfaceID( IIoComponentMgr, 1, 0 );

  struct IoMode {
    enum Type { READ = 0, WRITE, RW, INVALID };
  };

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  virtual StatusCode io_register( IIoComponent* iocomponent ) = 0;

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  virtual StatusCode io_register( IIoComponent* iocomponent, IIoComponentMgr::IoMode::Type iomode,
                                  const std::string& fname, const std::string& pfn = "" ) = 0;

  /** @brief: allow a @c IIoComponent to update the contents of the
   *          registry with a new file name
   */
  virtual StatusCode io_update( IIoComponent* iocomponent, const std::string& old_fname,
                                const std::string& new_fname ) = 0;

  /** @brief: allow a @c IIoComponent to update the contents of the
   *          registry with a new work directory
   */
  virtual StatusCode io_update( IIoComponent* iocomponent, const std::string& work_dir ) = 0;

  // VT. new method
  /** @brief: Update all @c IIoComponents with a new work directory
   */
  virtual StatusCode io_update_all( const std::string& work_dir ) = 0;

  /** @brief: check if the registry contains a given @c IIoComponent
   */
  virtual bool io_hasitem( IIoComponent* iocomponent ) const = 0;

  /** @brief: check if the registry contains a given @c IIoComponent and
   *          that component had @param `fname` as a filename
   */
  virtual bool io_contains( IIoComponent* iocomponent, const std::string& fname ) const = 0;

  /** @brief: retrieve all registered filenames for a given @c IIoComponent
   */
  virtual std::vector<std::string> io_retrieve( IIoComponent* iocomponent ) = 0;

  /** @brief: retrieve the new filename for a given @c IIoComponent and
   *          @param `fname` filename
   */
  virtual StatusCode io_retrieve( IIoComponent* iocomponent, std::string& fname ) = 0;

  /** @brief: reinitialize the I/O subsystem.
   *  This effectively calls @c IIoComponent::io_reinit on all the registered
   *  @c IIoComponent.
   */
  virtual StatusCode io_reinitialize() = 0;

  /** @brief: finalize the I/O subsystem.
   *  Hook to allow to e.g. give a chance to I/O subsystems to merge output
   *  files. Not sure how to do this correctly though...
   */
  virtual StatusCode io_finalize() = 0;
};
