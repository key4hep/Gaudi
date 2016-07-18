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

  IntegerProperty           m_int     {this, "Int", 100};
  LongLongProperty          m_int64   {this, "Int64", 100};
  UnsignedLongLongProperty  m_uint64  {this, "UInt64", 100};
  DoubleProperty            m_double  {this, "Double", 100.};
  StringProperty            m_string  {this, "String", "hundred"};
  BooleanProperty           m_bool    {this, "Bool", true};

  IntegerArrayProperty          m_intarray             {this, "IntArray"};
  LongLongArrayProperty         m_int64array           {this, "Int64Array"};
  UnsignedLongLongArrayProperty m_uint64array          {this, "UInt64Array"};
  DoubleArrayProperty           m_doublearray          {this, "DoubleArray"};
  StringArrayProperty           m_stringarray          {this, "StringArray"};
  BooleanArrayProperty          m_boolarray            {this, "BoolArray"};
  DoubleArrayProperty           m_emptyarray           {this, "EmptyArray"};
  DoubleArrayProperty           u_doublearrayunits     {this, "DoubleArrayWithUnits"};
  DoubleArrayProperty           u_doublearray          {this, "DoubleArrayWithoutUnits"};

  SimpleProperty<std::vector<std::pair<int,int>>>       u_intpairarray    {this, "IntPairArray"};
  SimpleProperty<std::vector<std::pair<double,double>>> u_doublepairarray {this, "DoublePairArray"};

  CheckedProperty<int>  p_int     {this, "PInt", 100, "An integer property"};
  PropertyWithReadHandler<double>  p_double  {this, "PDouble", 100.};
  StringProperty   p_string  {this, "PString", "hundred"};
  BooleanProperty  p_bool    {this, "PBool", false};

  IntegerArrayProperty  p_intarray     {this, "PIntArray"};
  DoubleArrayProperty   p_doublearray  {this, "PDoubleArray"};
  StringArrayProperty   p_stringarray  {this, "PStringArray"};
  BooleanArrayProperty  p_boolarray    {this, "PBoolArray"};
};

#endif    // GAUDIEXAMPLE_PROPERTYALG_H
