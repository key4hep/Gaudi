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
#ifndef GAUDISVC_CPUCRUNCHSVC_H
#define GAUDISVC_CPUCRUNCHSVC_H 1

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/StatusCode.h"

#include "GaudiKernel/ICPUCrunchSvc.h"
#include <chrono>
#include <string>
#include <vector>

class CPUCrunchSvc : public extends<Service, ICPUCrunchSvc> {

public:
  CPUCrunchSvc( const std::string& name, ISvcLocator* svc );

public:
  virtual StatusCode initialize() override;

public:
  // number of seconds to CPUCrunch
  std::chrono::milliseconds crunch_for( const std::chrono::milliseconds& crunchtime ) const override;

private:
  void         calibrate();
  unsigned int getNCaliIters( std::chrono::microseconds runtime ) const;
  void         findPrimes( unsigned int ) const;

  std::vector<unsigned int> m_times_vect;

  Gaudi::Property<std::vector<unsigned int>> m_niters_vect{
      this, "NIterationsVect", {}, "Number of iterations for the calibration." };
  Gaudi::Property<bool>         m_shortCalib{ this, "shortCalib", false, "Enable coarse grained calibration" };
  Gaudi::Property<unsigned int> m_minCalibTime{ this, "minCalibTime", 0,
                                                "Minimum number of milliseconds to do calibration" };
  Gaudi::Property<int>          m_numCalibRuns{ this, "numCalibRuns", 1, "Number of times to do calibration run" };
  Gaudi::Property<float>        m_corrFact{ this, "calibCorrectionFactor", 1.,
                                     "Calibration correction factor to take into account things like turbo boost" };
};

#endif
