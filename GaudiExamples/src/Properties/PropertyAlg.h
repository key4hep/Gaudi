// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiExamples/src/Properties/PropertyAlg.h,v 1.6 2008/10/30 22:40:56 marcocle Exp $
#ifndef GAUDIEXAMPLE_PROPERTYALG_H
#define GAUDIEXAMPLE_PROPERTYALG_H

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

/** @class PropertyAlg
    Trivial Algorithm for tutorial purposes

    @author nobody
*/
class PropertyAlg : public Algorithm {
public:
  /// Constructor of this form must be provided
  PropertyAlg(const std::string& name, ISvcLocator* pSvcLocator);
  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  /// Callbacks for properties
  void readHandler(Property&);
  void updateHandler(Property&);

private:
  /// These data members are used in the execution of this algorithm
  /// They are set in the initialization phase by the job options service

  int         m_int;
  long long   m_int64;
  unsigned long long   m_uint64;
  double      m_double;
  std::string m_string;
  bool        m_bool;

  std::vector<int>         m_intarray;
  std::vector<long long>   m_int64array;
  std::vector<unsigned long long>   m_uint64array;
  std::vector<double>      m_doublearray;
  std::vector<std::string> m_stringarray;
  std::vector<bool>        m_boolarray;
  std::vector<double>      m_emptyarray;
  std::vector<double>      u_doublearrayunits;
  std::vector<double>      u_doublearray;

  std::vector<std::pair<int,int> > u_intpairarray;
  std::vector<std::pair<double,double> > u_doublepairarray;

  IntegerProperty  p_int;
  DoubleProperty   p_double;
  StringProperty   p_string;
  BooleanProperty  p_bool;

  IntegerArrayProperty  p_intarray;
  DoubleArrayProperty   p_doublearray;
  StringArrayProperty   p_stringarray;
  BooleanArrayProperty  p_boolarray;
};

#endif    // GAUDIEXAMPLE_PROPERTYALG_H
