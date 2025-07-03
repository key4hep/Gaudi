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
// IoComponentMgr.h
// Header file for class IoComponentMgr
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////
#pragma once

// Python includes
#include <Python.h>

// STL includes
#include <list>
#include <map>
#include <string>
#include <vector>

// FrameWork includes
#include <GaudiKernel/Service.h>

// GaudiKernel
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIoComponent.h>
#include <GaudiKernel/IIoComponentMgr.h>

// Forward declaration
class ISvcLocator;

class IoComponentMgr : public extends<Service, IIoComponentMgr, IIncidentListener> {
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:
  /// Inherited constructor:
  using extends::extends;

  /// Gaudi Service Implementation
  //@{
  StatusCode initialize() override;
  StatusCode finalize() override;
  //@}

  void handle( const Incident& ) override;

  ///////////////////////////////////////////////////////////////////
  // Const methods:
  ///////////////////////////////////////////////////////////////////

  /** @brief: check if the registry contains a given @c IIoComponent
   */
  bool io_hasitem( IIoComponent* iocomponent ) const override;

  /** @brief: check if the registry contains a given @c IIoComponent and
   *          that component had @param `fname` as a filename
   */
  bool io_contains( IIoComponent* iocomponent, const std::string& fname ) const override;

  ///////////////////////////////////////////////////////////////////
  // Non-const methods:
  ///////////////////////////////////////////////////////////////////

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  StatusCode io_register( IIoComponent* iocomponent ) override;

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  StatusCode io_register( IIoComponent* iocomponent, IIoComponentMgr::IoMode::Type iomode, const std::string& fname,
                          const std::string& pfn ) override;

  /** @brief: allow a @c IIoComponent to update the contents of the
   *          registry with a new file name
   */
  StatusCode io_update( IIoComponent* iocomponent, const std::string& old_fname,
                        const std::string& new_fname ) override;

  /** @brief: allow a @c IIoComponent to update the contents of the
   *          registry with a new work directory
   */
  StatusCode io_update( IIoComponent* iocomponent, const std::string& work_dir ) override;

  /** @brief: Update all @c IIoComponents with a new work directory
   */
  StatusCode io_update_all( const std::string& work_dir ) override;

  /** @brief: retrieve all registered filenames for a given @c IIoComponent
   */
  std::vector<std::string> io_retrieve( IIoComponent* iocomponent ) override;

  /** @brief: retrieve the new filename for a given @c IIoComponent and
   *          @param `fname` filename
   */
  StatusCode io_retrieve( IIoComponent* iocomponent, std::string& fname ) override;

  /** @brief: reinitialize the I/O subsystem.
   *  This effectively calls @c IIoComponent::io_reinit on all the registered
   *  @c IIoComponent.
   */
  StatusCode io_reinitialize() override;

  /** @brief: finalize the I/O subsystem.
   *  Hook to allow to e.g. give a chance to I/O subsystems to merge output
   *  files. Not sure how to do this correctly though...
   */
  StatusCode io_finalize() override;

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
private:
  struct IoComponentEntry final {
    std::string                   m_oldfname;
    std::string                   m_oldabspath;
    std::string                   m_newfname;
    IIoComponentMgr::IoMode::Type m_iomode{ IIoComponentMgr::IoMode::INVALID };

    IoComponentEntry() = default;
    IoComponentEntry( const std::string& f, const std::string& p, const IIoComponentMgr::IoMode::Type& t )
        : m_oldfname( f ), m_oldabspath( p ), m_iomode( t ) {}

    friend bool operator<( const IoComponentEntry& lhs, const IoComponentEntry& rhs ) {
      return std::tie( lhs.m_oldfname, lhs.m_iomode ) < std::tie( rhs.m_oldfname, rhs.m_iomode );
    }

    friend std::ostream& operator<<( std::ostream& os, const IoComponentEntry& c ) {
      return os << "old: \"" << c.m_oldfname << "\"  absolute path: \"" << c.m_oldabspath << "\"  new: \""
                << c.m_newfname << "\"  m: " << ( ( c.m_iomode == IIoComponentMgr::IoMode::READ ) ? "R" : "W" );
    }
  };

  /// Default constructor:
  IoComponentMgr() = delete;

  typedef std::map<std::string, IIoComponent*> IoRegistry_t;
  /// Registry of @c IIoComponents
  IoRegistry_t m_ioregistry;

  typedef std::list<IIoComponent*> IoStack_t;
  /// Stack of @c IIoComponents to properly handle order of registration
  IoStack_t m_iostack;

  // This is the registry
  typedef std::multimap<IIoComponent*, IoComponentEntry> IoDict_t;
  typedef IoDict_t::const_iterator                       iodITR;

  IoDict_t m_cdict;

  /// location of the python dictionary
  std::string m_dict_location;

  /// search patterns for special file names (direct I/O protocols)
  Gaudi::Property<std::vector<std::string>> m_directio_patterns{ this,
                                                                 "DirectIOPatterns",
                                                                 {
                                                                     "://",
                                                                 },
                                                                 "Search patterns for direct I/O input names" };

  bool findComp( IIoComponent*, const std::string&, iodITR& ) const;
  bool findComp( IIoComponent*, std::pair<iodITR, iodITR>& ) const;
  bool findComp( const std::string&, std::pair<iodITR, iodITR>& ) const;

  std::string list() const;
};
