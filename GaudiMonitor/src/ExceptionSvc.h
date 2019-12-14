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
#ifndef GAUDISVC_EXCEPTIONSVC_H
#define GAUDISVC_EXCEPTIONSVC_H
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IExceptionSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
// ============================================================================
/** @class ExceptionSvc
 *  Simple implementation of IExceptionSvc abstract interface
 *  @author (1) ATLAS collaboration
 *  @author (2) modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-08
 */
// ============================================================================
class ExceptionSvc : public extends<Service, IExceptionSvc> {
public:
  /// Handle caught GaudiExceptions
  StatusCode handle( const INamedInterface& o, const GaudiException& e ) const override; ///< Handle caught exceptions
  /// Handle caught std::exceptions
  StatusCode handle( const INamedInterface& o, const std::exception& e ) const override; ///< Handle caught exceptions
  /// Handle caught (unknown)exceptions
  StatusCode handle( const INamedInterface& o ) const override; ///< Handle caught exceptions
  /// Handle errors
  StatusCode handleErr( const INamedInterface& o, const StatusCode& s ) const override; ///< Handle errors
public:
  /// initialize the service
  StatusCode initialize() override;

public:
  using extends::extends;

private:
  // default constructor is disabled
  ExceptionSvc() = delete; ///< no default constructor
  // copy constructor is disabled
  ExceptionSvc( const ExceptionSvc& ) = delete;
  // assignment operator is disabled
  ExceptionSvc& operator=( const ExceptionSvc& ) = delete;
  // process exceptions
  virtual StatusCode process( const INamedInterface& o ) const;

private:
  enum Policy { ALL, NONE };
  enum ReturnState { SUCCESS, FAILURE, RECOVERABLE, RETHROW, DEFAULT };

  Policy                             m_mode_exc = ALL, m_mode_err = NONE;
  std::map<std::string, ReturnState> m_retCodesExc, m_retCodesErr;

  Gaudi::Property<std::string> m_mode_exc_s{this, "Catch", "ALL"};
  Gaudi::Property<std::string> m_mode_err_s{this, "Errors", "NONE"};
};

// ============================================================================
#endif // GAUDISVC_EXCEPTIONSVC_H
