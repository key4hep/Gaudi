// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPython/src/Test/PropertyAlg.h,v 1.1 2005/10/27 16:57:57 mato Exp $
#ifndef GAUDIEXAMPLE_PROPERTYALG_H
#define GAUDIEXAMPLE_PROPERTYALG_H

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

/** @class PropertyAlg
    Trivial Algorithm for tutotial purposes
    
    @author nobody
*/
class PropertyAlg : public Algorithm {
public:
  /// Constructor of this form must be provided
  PropertyAlg(const std::string& name, ISvcLocator* pSvcLocator); 

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() { return StatusCode::SUCCESS; }
  StatusCode execute() { return StatusCode::SUCCESS; }
  StatusCode finalize() { return StatusCode::SUCCESS; }

private:
  /// These data members are used in the execution of this algorithm
  /// They are set in the initialisation phase by the job options service

  bool           m_bool;
  char           m_char;
  signed char    m_schar;
  unsigned char  m_uchar;
  short          m_short;
  unsigned short m_ushort;
  int            m_int;
  unsigned int   m_uint;
  long           m_long;
  unsigned long  m_ulong;
  long long      m_longlong;
  unsigned long long m_ulonglong;
  float          m_float;
  double         m_double;
  std::string    m_string;

  std::vector<bool>               m_boolarray;
  std::vector<char>               m_chararray;
  std::vector<unsigned char>      m_uchararray;
  std::vector<signed char>        m_schararray;
  std::vector<short>              m_shortarray;
  std::vector<unsigned short>     m_ushortarray;
  std::vector<int>                m_intarray;
  std::vector<unsigned int>       m_uintarray;
  std::vector<long>               m_longarray;
  std::vector<unsigned long>      m_ulongarray;
  std::vector<long long>          m_longlongarray;
  std::vector<unsigned long long> m_ulonglongarray;
  std::vector<float>              m_floatarray;
  std::vector<double>             m_doublearray;
  std::vector<std::string>        m_stringarray;

};

#endif    // GAUDIEXAMPLE_PROPERTYALG_H
