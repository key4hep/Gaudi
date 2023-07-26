/***********************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/FilterPredicate.h>

namespace Gaudi::Examples {
  struct Prescaler final : Gaudi::Functional::FilterPredicate<bool()> {
    Prescaler( const std::string& name, ISvcLocator* pSvc ) : FilterPredicate( name, pSvc ) {
      m_percentPass.verifier().setBounds( 0, 100 );
    }

    bool operator()() const override {
      // if the current pass efficiency is less then requested, we let the event go through
      const bool pass = m_passed.eff() * 100. <= m_percentPass;
      m_passed += pass;
      if ( msgLevel( MSG::DEBUG ) ) { debug() << ( pass ? "filter passed" : "filter failed" ) << endmsg; }
      return pass;
    }

    Gaudi::CheckedProperty<double> m_percentPass{ this, "PercentPass", 100.0,
                                                  "percentage of events that should be passed" };

    mutable Gaudi::Accumulators::BinomialCounter<> m_passed{ this, "passed" };
  };
  DECLARE_COMPONENT( Prescaler );
} // namespace Gaudi::Examples
