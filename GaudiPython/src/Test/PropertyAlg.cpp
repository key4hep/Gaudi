// $Id: PropertyAlg.cpp,v 1.2 2007/04/27 14:59:56 hmd Exp $

// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/SmartIF.h"
#include "PropertyAlg.h"


// Static Factory declaration
DECLARE_ALGORITHM_FACTORY(PropertyAlg)

// Constructor
//------------------------------------------------------------------------------
PropertyAlg::PropertyAlg(const std::string& name, ISvcLocator* ploc)
           : Algorithm(name, ploc) {
//------------------------------------------------------------------------------
  // Declare the algorithm's properties

  declareProperty( "Bool",      m_bool   = true);
  declareProperty( "Char",      m_char   = 100);
  declareProperty( "Uchar",     m_uchar  = 100);
  declareProperty( "Schar",     m_schar  = 100);
  declareProperty( "Short",     m_short  = 100);
  declareProperty( "Ushort",    m_ushort = 100);
  declareProperty( "Int",       m_int    = 100);
  declareProperty( "Uint",      m_uint   = 100);
  declareProperty( "Long",      m_long   = 100L);
  declareProperty( "Ulong",     m_ulong  = 100L);
  declareProperty( "Longlong",  m_longlong   = 100LL);
  declareProperty( "Ulonglong", m_ulonglong  = 100LL);
  declareProperty( "Float",     m_float  = 100.0F);
  declareProperty( "Double",    m_double = 100.);
  declareProperty( "String",    m_string = "hundred");

  declareProperty( "BoolArray",      m_boolarray );
  declareProperty( "CharArray",      m_chararray );
  declareProperty( "UcharArray",     m_uchararray );
  declareProperty( "ScharArray",     m_schararray );
  declareProperty( "ShortArray",     m_shortarray );
  declareProperty( "UshortArray",    m_ushortarray );
  declareProperty( "IntArray",       m_intarray );
  declareProperty( "UintArray",      m_uintarray );
  declareProperty( "LongArray",      m_longarray );
  declareProperty( "UlongArray",     m_ulongarray );
  declareProperty( "LonglongArray",  m_longlongarray );
  declareProperty( "UlonglongArray", m_ulonglongarray );
  declareProperty( "FloatArray",     m_floatarray );
  declareProperty( "DoubleArray",    m_doublearray );
  declareProperty( "StringArray",    m_stringarray );
  
}



