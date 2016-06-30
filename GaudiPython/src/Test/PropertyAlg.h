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

  PropertyWithValue<bool> m_bool{this, "Bool", true, "doc for Bool property"};
  PropertyWithValue<char> m_char{this, "Char", 100, "doc for Char property"};
  PropertyWithValue<signed char> m_uchar{this, "Uchar", 100, "doc for Uchar property"};
  PropertyWithValue<unsigned char> m_schar{this, "Schar", 100, "doc for Schar property"};
  PropertyWithValue<short> m_short{this, "Short", 100, "doc for Short property"};
  PropertyWithValue<unsigned short> m_ushort{this, "Ushort", 100, "doc for Ushort property"};
  PropertyWithValue<int> m_int{this, "Int", 100, "doc for Int property"};
  PropertyWithValue<unsigned int> m_uint{this, "Uint", 100, "doc for Uint property"};
  PropertyWithValue<long> m_long{this, "Long", 100L, "doc for Long property"};
  PropertyWithValue<unsigned long> m_ulong{this, "Ulong", 100L, "doc for Ulong property"};
  PropertyWithValue<long long> m_longlong{this, "Longlong", 100LL, "doc for Longlong property"};
  PropertyWithValue<unsigned long long> m_ulonglong{this, "Ulonglong", 100LL, "doc for Ulonglong property"};
  PropertyWithValue<float> m_float{this, "Float", 100.0F, "doc for Float property"};
  PropertyWithValue<double> m_double{this, "Double", 100., "doc for Double property"};
  PropertyWithValue<std::string> m_string{this, "String", "hundred", "doc for String property"};

  PropertyWithValue<std::vector<bool>> m_boolarray{this, "BoolArray", {}, "doc for BoolArray property"};
  PropertyWithValue<std::vector<char>> m_chararray{this, "CharArray", {}, "doc for CharArray property"};
  PropertyWithValue<std::vector<signed char>> m_uchararray{this, "UcharArray", {}, "doc for UcharArray property"};
  PropertyWithValue<std::vector<unsigned char>> m_schararray{this, "ScharArray", {}, "doc for ScharArray property"};
  PropertyWithValue<std::vector<short>> m_shortarray{this, "ShortArray", {}, "doc for ShortArray property"};
  PropertyWithValue<std::vector<unsigned short>> m_ushortarray{this, "UshortArray", {}, "doc for UshortArray property"};
  PropertyWithValue<std::vector<int>> m_intarray{this, "IntArray", {}, "doc for IntArray property"};
  PropertyWithValue<std::vector<unsigned int>> m_uintarray{this, "UintArray", {}, "doc for UintArray property"};
  PropertyWithValue<std::vector<long>> m_longarray{this, "LongArray", {}, "doc for LongArray property"};
  PropertyWithValue<std::vector<unsigned long>> m_ulongarray{this, "UlongArray", {}, "doc for UlongArray property"};
  PropertyWithValue<std::vector<long long>> m_longlongarray{
      this, "LonglongArray", {}, "doc for LonglongArray property"};
  PropertyWithValue<std::vector<unsigned long long>> m_ulonglongarray{
      this, "UlonglongArray", {}, "doc for UlonglongArray property"};
  PropertyWithValue<std::vector<float>> m_floatarray{this, "FloatArray", {}, "doc for FloatArray property"};
  PropertyWithValue<std::vector<double>> m_doublearray{this, "DoubleArray", {}, "doc for DoubleArray property"};
  PropertyWithValue<std::vector<std::string>> m_stringarray{this, "StringArray", {}, "doc for StringArray property"};
};

#endif // GAUDIEXAMPLE_PROPERTYALG_H
