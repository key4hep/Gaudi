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
#ifndef GAUDIEXAMPLE_HELLOWORLD_H
#define GAUDIEXAMPLE_HELLOWORLD_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include <Gaudi/Property.h>

/** @class HelloWorld
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class HelloWorld : public Algorithm {
public:
  /// Constructor of this form must be provided
  HelloWorld( const std::string& name, ISvcLocator* pSvcLocator );

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  bool m_initialized;
  /// These data members are used in the execution of this algorithm
  /// They are set in the initialisation phase by the job options service
  Gaudi::Property<int>         m_int;
  Gaudi::Property<double>      m_double;
  Gaudi::Property<std::string> m_string;
  int                          t_int{0};
  double                       t_double{0};
  std::string                  t_string;
};

#endif // GAUDIEXAMPLE_HELLOWORLD_H
