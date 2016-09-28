#ifndef GAUDIEXAMPLE_PROPERTYALG_H
#define GAUDIEXAMPLE_PROPERTYALG_H

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

/** @class PropertyAlg
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class PropertyAlg : public Algorithm
{
public:
  /// Constructor of this form must be provided
  using Algorithm::Algorithm;

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() { return StatusCode::SUCCESS; }
  StatusCode execute() { return StatusCode::SUCCESS; }
  StatusCode finalize() { return StatusCode::SUCCESS; }

private:
  /// These data members are used in the execution of this algorithm
  /// They are set in the initialisation phase by the job options service

  Gaudi::Property<bool> m_bool{this, "Bool", true, "doc for Bool property"};
  Gaudi::Property<char> m_char{this, "Char", 100, "doc for Char property"};
  Gaudi::Property<signed char> m_uchar{this, "Uchar", 100, "doc for Uchar property"};
  Gaudi::Property<unsigned char> m_schar{this, "Schar", 100, "doc for Schar property"};
  Gaudi::Property<short> m_short{this, "Short", 100, "doc for Short property"};
  Gaudi::Property<unsigned short> m_ushort{this, "Ushort", 100, "doc for Ushort property"};
  Gaudi::Property<int> m_int{this, "Int", 100, "doc for Int property"};
  Gaudi::Property<unsigned int> m_uint{this, "Uint", 100, "doc for Uint property"};
  Gaudi::Property<long> m_long{this, "Long", 100L, "doc for Long property"};
  Gaudi::Property<unsigned long> m_ulong{this, "Ulong", 100L, "doc for Ulong property"};
  Gaudi::Property<long long> m_longlong{this, "Longlong", 100LL, "doc for Longlong property"};
  Gaudi::Property<unsigned long long> m_ulonglong{this, "Ulonglong", 100LL, "doc for Ulonglong property"};
  Gaudi::Property<float> m_float{this, "Float", 100.0F, "doc for Float property"};
  Gaudi::Property<double> m_double{this, "Double", 100., "doc for Double property"};
  Gaudi::Property<std::string> m_string{this, "String", "hundred", "doc for String property"};

  Gaudi::Property<std::vector<bool>> m_boolarray{this, "BoolArray", {}, "doc for BoolArray property"};
  Gaudi::Property<std::vector<char>> m_chararray{this, "CharArray", {}, "doc for CharArray property"};
  Gaudi::Property<std::vector<signed char>> m_uchararray{this, "UcharArray", {}, "doc for UcharArray property"};
  Gaudi::Property<std::vector<unsigned char>> m_schararray{this, "ScharArray", {}, "doc for ScharArray property"};
  Gaudi::Property<std::vector<short>> m_shortarray{this, "ShortArray", {}, "doc for ShortArray property"};
  Gaudi::Property<std::vector<unsigned short>> m_ushortarray{this, "UshortArray", {}, "doc for UshortArray property"};
  Gaudi::Property<std::vector<int>> m_intarray{this, "IntArray", {}, "doc for IntArray property"};
  Gaudi::Property<std::vector<unsigned int>> m_uintarray{this, "UintArray", {}, "doc for UintArray property"};
  Gaudi::Property<std::vector<long>> m_longarray{this, "LongArray", {}, "doc for LongArray property"};
  Gaudi::Property<std::vector<unsigned long>> m_ulongarray{this, "UlongArray", {}, "doc for UlongArray property"};
  Gaudi::Property<std::vector<long long>> m_longlongarray{this, "LonglongArray", {}, "doc for LonglongArray property"};
  Gaudi::Property<std::vector<unsigned long long>> m_ulonglongarray{
      this, "UlonglongArray", {}, "doc for UlonglongArray property"};
  Gaudi::Property<std::vector<float>> m_floatarray{this, "FloatArray", {}, "doc for FloatArray property"};
  Gaudi::Property<std::vector<double>> m_doublearray{this, "DoubleArray", {}, "doc for DoubleArray property"};
  Gaudi::Property<std::vector<std::string>> m_stringarray{this, "StringArray", {}, "doc for StringArray property"};
};

#endif // GAUDIEXAMPLE_PROPERTYALG_H
