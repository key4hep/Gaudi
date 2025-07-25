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
///////////////////////// -*- C++ -*- /////////////////////////////
// IoComponentMgr.cxx
// Implementation file for class IoComponentMgr
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////

// GaudiMP includes
#include "IoComponentMgr.h"
// FrameWork includes
#include <Gaudi/Property.h>
#include <GaudiKernel/FileIncident.h>
#include <GaudiKernel/IFileMgr.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/ServiceHandle.h>
// BOOST includes
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
// STL includes
#include <algorithm>
#include <array>

#define ON_DEBUG if ( outputLevel() <= MSG::DEBUG )
#define ON_VERBOSE if ( outputLevel() <= MSG::VERBOSE )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

DECLARE_COMPONENT( IoComponentMgr )

using namespace std;

std::ostream& operator<<( std::ostream& os, const IIoComponentMgr::IoMode::Type& m ) {
  switch ( m ) {
  case IIoComponentMgr::IoMode::READ:
    os << "READ";
    break;
  case IIoComponentMgr::IoMode::WRITE:
    os << "WRITE";
    break;
  case IIoComponentMgr::IoMode::RW:
    os << "RW";
    break;
  default:
    os << "INVALID";
    break;
  }

  return os;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode IoComponentMgr::initialize() {
  DEBMSG << "--> initialize()" << endmsg;

  if ( Service::initialize().isFailure() ) {
    error() << "Unable to initialize Service base class" << endmsg;
    return StatusCode::FAILURE;
  }

  auto p_incSvc = service<IIncidentSvc>( "IncidentSvc", true );

  if ( !p_incSvc ) {
    error() << "unable to get the IncidentSvc" << endmsg;
    return StatusCode::FAILURE;
  } else {
    p_incSvc->addListener( this, IncidentType::BeginOutputFile, 100, true );
    p_incSvc->addListener( this, IncidentType::BeginInputFile, 100, true );
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode IoComponentMgr::finalize() {
  DEBMSG << "--> finalize()" << endmsg;

  for ( auto& io : m_iostack ) { io->release(); }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: check if the registry contains a given @c IIoComponent
 */
bool IoComponentMgr::io_hasitem( IIoComponent* iocomponent ) const {
  DEBMSG << "--> io_hasitem()" << endmsg;
  if ( 0 == iocomponent ) { return false; }
  const std::string&           ioname = iocomponent->name();
  IoRegistry_t::const_iterator io     = m_ioregistry.find( ioname );
  return io != m_ioregistry.end();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: check if the registry contains a given @c IIoComponent and
 *          that component had @param `fname` as a filename
 */
bool IoComponentMgr::io_contains( IIoComponent* iocomponent, const std::string& fname ) const {
  DEBMSG << "--> io_contains()" << endmsg;
  if ( 0 == iocomponent ) { return false; }
  const std::string& ioname = iocomponent->name();

  DEBMSG << "io_contains:  c: " << ioname << " f: " << fname << endmsg;

  pair<iodITR, iodITR> fit = m_cdict.equal_range( iocomponent );
  if ( fit.first == fit.second ) {
    return false;
  } else {
    iodITR it;
    for ( it = fit.first; it != fit.second; ++it ) {
      IoComponentEntry ioe = it->second;
      DEBMSG << "   " << ioe << endmsg;
      if ( ioe.m_oldfname == "" ) {
        error() << "IIoComponent " << ioname << "  has empty old filename" << endmsg;
        return false;
      } else if ( ioe.m_oldfname == fname ) {
        return true;
      }
    }
  }

  return false;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: allow a @c IIoComponent to register itself with this
 *          manager so appropriate actions can be taken when e.g.
 *          a @c fork(2) has been issued (this is usually handled
 *          by calling @c IIoComponent::io_reinit on every registered
 *          component)
 */
StatusCode IoComponentMgr::io_register( IIoComponent* iocomponent ) {
  if ( !iocomponent ) {
    error() << "io_register (component) received a NULL pointer !" << endmsg;
    return StatusCode::FAILURE;
  }
  const std::string& ioname = iocomponent->name();
  DEBMSG << "--> io_register(" << ioname << ")" << endmsg;
  auto itr = m_ioregistry.find( ioname );
  if ( itr == m_ioregistry.end() ) {
    DEBMSG << "    registering IoComponent \"" << ioname << "\"" << endmsg;
    iocomponent->addRef(); // ownership...
    m_ioregistry[ioname] = iocomponent;
    m_iostack.push_back( iocomponent );
  } else {
    info() << "IoComponent[" << iocomponent->name() << "] already registered @" << (void*)itr->second << endmsg;
  }
  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: allow a @c IIoComponent to register itself with this
 *          manager so appropriate actions can be taken when e.g.
 *          a @c fork(2) has been issued (this is usually handled
 *          by calling @c IIoComponent::io_reinit on every registered
 *          component)
 */
StatusCode IoComponentMgr::io_register( IIoComponent* iocomponent, IIoComponentMgr::IoMode::Type iomode,
                                        const std::string& fname, const std::string& pfn ) {
  if ( 0 == iocomponent ) { return StatusCode::FAILURE; }
  const std::string& ioname = iocomponent->name();

  DEBMSG << "--> io_register(" << ioname << "," << ( ( iomode == IIoComponentMgr::IoMode::READ ) ? "R" : "W" ) << ","
         << fname << ")" << endmsg;

  if ( !io_hasitem( iocomponent ) ) {
    if ( !io_register( iocomponent ).isSuccess() ) {
      error() << "could not register component [" << iocomponent->name() << "] "
              << "with the I/O component manager !" << endmsg;
      return StatusCode::FAILURE;
    }
  }

  pair<iodITR, iodITR> fit = m_cdict.equal_range( iocomponent );
  if ( fit.first != fit.second ) {
    for ( iodITR it = fit.first; it != fit.second; ++it ) {
      IoComponentEntry ioe = it->second;
      if ( ioe.m_oldfname == fname ) {
        if ( ioe.m_iomode == iomode ) {
          info() << "IoComponent " << ioname << " has already had file " << fname << " registered with i/o mode "
                 << iomode << endmsg;
          return StatusCode::SUCCESS;
        } else {
          warning() << "IoComponent " << ioname << " has already had file " << fname
                    << " registered with a different i/o mode " << ioe.m_iomode << " - now trying " << iomode << endmsg;
        }
      }
    }
  }

  // We need to take into account that boost::filesystem::absolute() does not work with direct I/O inputs
  const std::string& tmp_name = ( pfn.empty() ? fname : pfn );
  bool               special_case =
      std::any_of( begin( m_directio_patterns.value() ), end( m_directio_patterns.value() ),
                   [&]( const std::string& pf ) { return boost::algorithm::contains( tmp_name, pf ); } );
  IoComponentEntry ioc( fname, ( special_case ? tmp_name : boost::filesystem::absolute( tmp_name ).string() ), iomode );
  m_cdict.insert( pair<IIoComponent*, IoComponentEntry>( iocomponent, ioc ) );

  return StatusCode::SUCCESS;
}

/** @brief: retrieve all registered filenames for a given @c IIoComponent
 */
std::vector<std::string> IoComponentMgr::io_retrieve( IIoComponent* iocomponent ) {
  std::vector<std::string> fnames;
  pair<iodITR, iodITR>     pit;

  // Find component and copy all registered file names
  if ( iocomponent != nullptr && findComp( iocomponent, pit ) ) {
    std::transform( pit.first, pit.second, std::back_inserter( fnames ),
                    []( const auto& itr ) { return itr.second.m_oldfname; } );
  }
  return fnames;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: retrieve the new filename for a given @c IIoComponent and
 *          @param `fname` filename
 */
StatusCode IoComponentMgr::io_retrieve( IIoComponent* iocomponent, std::string& fname ) {
  if ( 0 == iocomponent ) { return StatusCode::FAILURE; }

  std::string        ofname = fname;
  const std::string& ioname = iocomponent->name();

  DEBMSG << "--> io_retrieve(" << ioname << "," << fname << ")" << endmsg;

  iodITR it;
  if ( !findComp( iocomponent, ofname, it ) ) {
    DEBMSG << "could not find c: " << ioname << "  old_f: " << ofname << endmsg;
    return StatusCode::FAILURE;
  } else {

    IoDict_t::iterator it;
    for ( it = m_cdict.equal_range( iocomponent ).first; it != m_cdict.equal_range( iocomponent ).second; ++it ) {

      if ( it->second.m_oldfname == ofname ) {
        DEBMSG << "retrieving new name for the component " << iocomponent->name() << " old name: " << ofname
               << ", new name: " << it->second.m_newfname << endmsg;
        fname = it->second.m_newfname;
        return StatusCode::SUCCESS;
      }
    }
  }

  DEBMSG << "Unexpected error! Unable to find entry in the dictionary corresponding to old filename: " << ofname
         << endmsg;
  return StatusCode::FAILURE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: reinitialize the I/O subsystem.
 *  This effectively calls @c IIoComponent::io_reinit on all the registered
 *  @c IIoComponent.
 */
StatusCode IoComponentMgr::io_reinitialize() {
  ON_DEBUG {
    debug() << "--> io_reinitialize()" << endmsg;
    debug() << "reinitializing I/O subsystem..." << endmsg;
    debug() << "Listing all monitored entries: " << std::endl;
    debug() << list() << endmsg;
  }

  bool allgood = true;
  for ( auto& io : m_iostack ) {
    DEBMSG << " [" << io->name() << "]->io_reinit()..." << endmsg;
    if ( !io->io_reinit().isSuccess() ) {
      allgood = false;
      error() << "problem in [" << io->name() << "]->io_reinit() !" << endmsg;
    }
  }

  return allgood ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode IoComponentMgr::io_update( IIoComponent* ioc, const std::string& old_fname, const std::string& new_fname ) {

  DEBMSG << "--> io_update(" << ioc->name() << "," << old_fname << "," << new_fname << ")" << endmsg;

  IoDict_t::iterator it;
  for ( it = m_cdict.equal_range( ioc ).first; it != m_cdict.equal_range( ioc ).second; ++it ) {

    if ( it->second.m_oldfname == old_fname ) {
      DEBMSG << "updating " << ioc->name() << " f: " << old_fname << " -> " << new_fname << endmsg;
      it->second.m_newfname = new_fname;
      return StatusCode::SUCCESS;
    }
  }

  return StatusCode::FAILURE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode IoComponentMgr::io_update( IIoComponent* ioc, const std::string& work_dir ) {

  DEBMSG << "--> io_update(" << ioc->name() << "," << work_dir << ")" << endmsg;

  IoDict_t::iterator it;
  for ( it = m_cdict.equal_range( ioc ).first; it != m_cdict.equal_range( ioc ).second; ++it ) {

    switch ( it->second.m_iomode ) {
    case IIoComponentMgr::IoMode::READ: {
      it->second.m_newfname = it->second.m_oldabspath;
      break;
    }
    case IIoComponentMgr::IoMode::WRITE: {
      boost::filesystem::path oldPath( it->second.m_oldfname );
      if ( oldPath.is_relative() && oldPath.filename() == oldPath.relative_path() ) {

        // Only file name was provided for writing. This is the usual mode of operation
        // ***
        // NB. In such cases it would make sense to set newfname=oldfname, however this will break
        //     existing client codes, which assume that newfname contains "/"
        //     Thus we set newfname=workdir/oldfname
        // ***

        boost::filesystem::path newfname( work_dir );
        newfname /= oldPath;
        it->second.m_newfname = newfname.string();
      } else {
        // New name should be the old absolute path
        it->second.m_newfname = it->second.m_oldabspath;
      }

      break;
    }
    default: {
      // Don't know how to deal with either RW or INVALID
      error() << "Unable to update IoComponent for the mode " << it->second.m_iomode << endmsg;
      return StatusCode::FAILURE;
    }
    }

  } // for

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode IoComponentMgr::io_update_all( const std::string& work_dir ) {
  DEBMSG << "-->io_update_all for the directory " << work_dir << endmsg;
  bool allgood = true;
  for ( IoStack_t::iterator io = m_iostack.begin(), ioEnd = m_iostack.end(); io != ioEnd; ++io ) {
    if ( !io_update( *io, work_dir ).isSuccess() ) {
      allgood = false;
      error() << "problem in [" << ( *io )->name() << "]->io_update() !" << endmsg;
    }
  }
  return allgood ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: finalize the I/O subsystem.
 *  Hook to allow to e.g. give a chance to I/O subsystems to merge output
 *  files. Not sure how to do this correctly though...
 */
StatusCode IoComponentMgr::io_finalize() {
  ON_DEBUG {
    debug() << "--> io_finalize()" << endmsg;
    debug() << "finalizing I/O subsystem..." << endmsg;
    debug() << "Listing all monitored entries: " << std::endl;
    debug() << list() << endmsg;
  }

  bool allgood = true;
  // reverse iteration to unwind component dependencies
  for ( IoStack_t::reverse_iterator io = m_iostack.rbegin(), ioEnd = m_iostack.rend(); io != ioEnd; ++io ) {
    DEBMSG << " [" << ( *io )->name() << "]->io_finalize()..." << endmsg;
    if ( !( *io )->io_finalize().isSuccess() ) {
      allgood = false;
      error() << "problem in [" << ( *io )->name() << "]->io_finalize() !" << endmsg;
    }
  }

  return allgood ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool IoComponentMgr::findComp( IIoComponent* c, const std::string& f, iodITR& itr ) const {

  pair<iodITR, iodITR> pit;
  if ( !findComp( c, pit ) ) {
    itr = pit.first;
    return false;
  }
  return std::any_of( pit.first, pit.second, [&]( IoDict_t::const_reference i ) { return i.second.m_oldfname == f; } );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool IoComponentMgr::findComp( IIoComponent* c, std::pair<iodITR, iodITR>& pit ) const {

  pit = m_cdict.equal_range( c );
  return pit.first != pit.second;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool IoComponentMgr::findComp( const std::string& c, std::pair<iodITR, iodITR>& pit ) const {

  pit.first  = m_cdict.end();
  pit.second = m_cdict.end();

  auto itr = m_ioregistry.find( c );
  return ( itr != m_ioregistry.end() ) && findComp( itr->second, pit );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string IoComponentMgr::list() const {

  ostringstream ost;

  ost << "Listing all IoComponents (" << m_cdict.size() << "): " << endl;
  for ( const auto& i : m_cdict ) { ost << "  " << i.first->name() << "  " << i.second << endl; }

  return ost.str();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void IoComponentMgr::handle( const Incident& i ) {

  pair<iodITR, iodITR> pit;

  if ( i.type() == IncidentType::BeginInputFile ) {

    const FileIncident* fi = dynamic_cast<const FileIncident*>( &i );
    DEBMSG << "BeginInputFile:   s: " << fi->source() << "  t: " << fi->type() << "  n: " << fi->fileName()
           << "  g: " << fi->fileGuid() << endmsg;

    if ( findComp( fi->source(), pit ) ) {
      DEBMSG << "  found component: " << endmsg;
      while ( pit.first != pit.second ) {
        IIoComponent*    c = pit.first->first;
        IoComponentEntry e = pit.first->second;
        DEBMSG << "    c: " << c->name() << "  " << e << endmsg;

        ++pit.first;
      }
    } else {
      DEBMSG << "  could not find component \"" << fi->source() << "\"!" << endmsg;
    }

  } else if ( i.type() == IncidentType::BeginOutputFile ) {

    const FileIncident* fi = dynamic_cast<const FileIncident*>( &i );
    DEBMSG << "BeginOutputFile:   s: " << fi->source() << "  t: " << fi->type() << "  n: " << fi->fileName()
           << "  g: " << fi->fileGuid() << endmsg;

    if ( findComp( fi->source(), pit ) ) {
      DEBMSG << "  found component: " << endmsg;
      while ( pit.first != pit.second ) {
        IIoComponent*    c = pit.first->first;
        IoComponentEntry e = pit.first->second;
        DEBMSG << "    c: " << c->name() << "  " << e << endmsg;

        ++pit.first;
      }
    } else {
      DEBMSG << "  could not find component \"" << fi->source() << "\"!" << endmsg;
    }
  }
}
