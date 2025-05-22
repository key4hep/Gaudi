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

#include <GaudiKernel/Incident.h>

/**
 * @class DataIncident DataIncident.h GaudiKernel/DataIncident.h
 *
 * Data service incident class.
 *
 * @author   Markus Frank
 * @version  1.0
 */
template <class T>
class GAUDI_API ContextIncident : public Incident {
  T m_tag;

public:
  /// Initializing Constructor
  /** @param source   [IN]  Incident source (service or algorithm name)
   * @param type     [IN]  Incident type
   * @param tag      [IN]  Incident tag
   *
   * @return Reference to ContextIncident object.
   */
  ContextIncident( std::string src, std::string typ, T tag )
      : Incident( std::move( src ), std::move( typ ) ), m_tag( std::move( tag ) ) {}

  /// Accesssor to the tag value (CONST)
  const T& tag() const { return m_tag; }
};

/// Specific incident type used by the data-on-demand-service
typedef ContextIncident<std::string> DataIncident;
