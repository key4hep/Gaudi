/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIEXAMPLE_PROPERTYALG_H
#define GAUDIEXAMPLE_PROPERTYALG_H

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include <Gaudi/Property.h>

#include <map>
#include <string>
#include <vector>

/** @class PropertyAlg
    Trivial Algorithm for tutorial purposes

    @author nobody
*/
class PropertyAlg : public Algorithm {
public:
  /// Constructor of this form must be provided
  PropertyAlg( const std::string& name, ISvcLocator* pSvcLocator );
  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  /// Callbacks for properties
  void readHandler( Gaudi::Details::PropertyBase& );
  void updateHandler( Gaudi::Details::PropertyBase& );

private:
  /// These data members are used in the execution of this algorithm
  /// They are set in the initialization phase by the job options service

  Gaudi::Property<int>                m_int{this, "Int", 100};
  Gaudi::Property<long long>          m_int64{this, "Int64", 100};
  Gaudi::Property<unsigned long long> m_uint64{this, "UInt64", 100};
  Gaudi::Property<double>             m_double{this, "Double", 100.};
  Gaudi::Property<std::string>        m_string{this, "String", "hundred"};
  Gaudi::Property<bool>               m_bool{this, "Bool", true};

  Gaudi::Property<std::vector<int>>                m_intarray{this, "IntArray"};
  Gaudi::Property<std::vector<long long>>          m_int64array{this, "Int64Array"};
  Gaudi::Property<std::vector<unsigned long long>> m_uint64array{this, "UInt64Array"};
  Gaudi::Property<std::vector<double>>             m_doublearray{this, "DoubleArray"};
  Gaudi::Property<std::vector<std::string>>        m_stringarray{this, "StringArray"};
  Gaudi::Property<std::vector<bool>>               m_boolarray{this, "BoolArray"};
  Gaudi::Property<std::vector<double>>             m_emptyarray{this, "EmptyArray"};
  Gaudi::Property<std::vector<double>>             u_doublearrayunits{this, "DoubleArrayWithUnits"};
  Gaudi::Property<std::vector<double>>             u_doublearray{this, "DoubleArrayWithoutUnits"};

  Gaudi::Property<std::vector<std::pair<int, int>>>       u_intpairarray{this, "IntPairArray"};
  Gaudi::Property<std::vector<std::pair<double, double>>> u_doublepairarray{this, "DoublePairArray"};

  Gaudi::Property<std::map<std::string, std::string>> m_strmap{this, "StringMap"};

  Gaudi::CheckedProperty<int>            p_int{this, "PInt", 100, "An integer property"};
  Gaudi::PropertyWithReadHandler<double> p_double{this, "PDouble", 100.};

  Gaudi::Property<std::string> p_string{this, "PString", "hundred"};
  Gaudi::Property<bool>        p_bool{this, "PBool", false};

  Gaudi::Property<std::vector<int>>         p_intarray{this, "PIntArray"};
  Gaudi::Property<std::vector<double>>      p_doublearray{this, "PDoubleArray"};
  Gaudi::Property<std::vector<std::string>> p_stringarray{this, "PStringArray"};
  Gaudi::Property<std::vector<bool>>        p_boolarray{this, "PBoolArray"};
};

#endif // GAUDIEXAMPLE_PROPERTYALG_H
