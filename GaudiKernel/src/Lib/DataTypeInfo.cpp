// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/DataTypeInfo.cpp,v 1.5 2008/10/27 19:22:21 marcocle Exp $
//	====================================================================
//
//	DataTypeInfo.cpp
//	--------------------------------------------------------------------
//
//	Package   : Kernel
//              Data Object types - implementation
//
//	Author    : Markus Frank
//  History:
//  +---------+----------------------------------------------+---------+
//  |    Date |                 Comment                      | Who     |
//  +---------+----------------------------------------------+---------+
//  | 3/07/00 | Initial version                              | M.Frank |
//  +---------+----------------------------------------------+---------+
//	====================================================================
#define GAUDI_KERNEL_DATATYPEINFO_CPP 1

#include "GaudiKernel/System.h"
#include "GaudiKernel/DataTypeInfo.h"
#include "GaudiKernel/IOpaqueAddress.h"

// Access to type information
DataTypeInfo::Type DataTypeInfo::idByName( const std::string& typ )   {
  if ( typ == "unsigned char"         || typ == "const unsigned char" )
    return UCHAR;
  else if ( typ == "unsigned short"   || typ == "const unsigned short" )
    return USHORT;
  else if ( typ == "unsigned int"     || typ == "const unsigned int" )
    return UINT;
  else if ( typ == "unsigned long"    || typ == "const unsigned long" )
    return ULONG;
  else if ( typ == "char"             || typ == "const char" )
    return CHAR;
  else if ( typ == "short"            || typ == "const short" )
    return SHORT;
  else if ( typ == "int"              || typ == "const int" )
    return INT;
  else if ( typ == "long"             || typ == "const long" )
    return LONG;
  else if ( typ == "bool"             || typ == "const bool" )
    return BOOL;
  else if ( typ == "float"            || typ == "const float" )
    return FLOAT;
  else if ( typ == "double"           || typ == "const double" )
    return DOUBLE;
  else if ( typ == "std::string"      || typ == "const std::string" )
    return STRING;
  else if ( typ == "char*"            || typ == "const char*" )
    return NTCHAR;
  else if ( typ == "IOpaqueAddress*"  || typ == "const IOpaqueAddress*" )
    return OBJECT_ADDR;
  else if ( typ == "SmartRef<DataObject>"      || typ == "const SmartRef<DataObject>" )
    return OBJECT_REF;
  else if ( typ == "SmartRef<ContainedObject>" || typ == "const SmartRef<ContainedObject>" )
    return CONTAINED_REF;
  else if ( typ == "void*"            || typ == "const void*" )
    return POINTER;
  else
    return UNKNOWN;
}

// Access to type information
const std::type_info& DataTypeInfo::typeByName( const std::string& typ )   {
  if ( typ == "unsigned char"         || typ == "const unsigned char" )
    return typeid(unsigned char);
  else if ( typ == "unsigned short"   || typ == "const unsigned short" )
    return typeid(unsigned short);
  else if ( typ == "unsigned int"     || typ == "const unsigned int" )
    return typeid(unsigned int);
  else if ( typ == "unsigned long"    || typ == "const unsigned long" )
    return typeid(unsigned long);
  else if ( typ == "char"             || typ == "const char" )
    return typeid(char);
  else if ( typ == "short"            || typ == "const short" )
    return typeid(short);
  else if ( typ == "int"              || typ == "const int" )
    return typeid(int);
  else if ( typ == "long"             || typ == "const long" )
    return typeid(long);
  else if ( typ == "bool"             || typ == "const bool" )
    return typeid(bool);
  else if ( typ == "float"            || typ == "const float" )
    return typeid(float);
  else if ( typ == "double"           || typ == "const double" )
    return typeid(double);
  else if ( typ == "std::string"      || typ == "const std::string" )
    return typeid(std::string);
  else if ( typ == "char*"            || typ == "const char*" )
    return typeid(char*);
  else if ( typ == "IOpaqueAddress*"  || typ == "const IOpaqueAddress*" )
    return typeid(IOpaqueAddress);
  else if ( typ == "SmartRef<DataObject>"      || typ == "const SmartRef<DataObject>" )
    return typeid(DataObject*);
  else if ( typ == "SmartRef<ContainedObject>" || typ == "const SmartRef<ContainedObject>" )
    return typeid(SmartRef<ContainedObject>);
  else if ( typ == "void*"            || typ == "const void*" )
    return typeid(void*);
  else
    return typeid(void*);
}

// Access the type name by type ID
std::string DataTypeInfo::name(long typ)   {
  switch(typ)   {
  case UCHAR:
    return System::typeinfoName(typeid(unsigned char));
  case USHORT:
    return System::typeinfoName(typeid(unsigned short));
  case UINT:
    return System::typeinfoName(typeid(unsigned int));
  case ULONG:
    return System::typeinfoName(typeid(unsigned long));
  case CHAR:
    return System::typeinfoName(typeid(char));
  case SHORT:
    return System::typeinfoName(typeid(short));
  case INT:
    return System::typeinfoName(typeid(int));
  case LONG:
    return System::typeinfoName(typeid(long));
  case BOOL:
    return System::typeinfoName(typeid(bool));
  case FLOAT:
    return System::typeinfoName(typeid(float));
  case DOUBLE:
    return System::typeinfoName(typeid(double));
  case STRING:
    return System::typeinfoName(typeid(std::string));
  case LONG_STRING:
    return System::typeinfoName(typeid(std::string));
  case NTCHAR:
    return System::typeinfoName(typeid(char*));
  case LONG_NTCHAR:
    return System::typeinfoName(typeid(char*));
  case OBJECT_ADDR:
    return System::typeinfoName(typeid(IOpaqueAddress*));
  case OBJECT_REF:
    return System::typeinfoName(typeid(SmartRef<DataObject>));
  case CONTAINED_REF:
    return System::typeinfoName(typeid(SmartRef<ContainedObject>));
  case POINTER:
    return System::typeinfoName(typeid(void*));
  case UNKNOWN:
  default:
    return "";
  }
}

// Access the type name by type_info
std::string DataTypeInfo::name(const std::type_info& typ)    {
  if ( typ == typeid(unsigned char) )
    return DataTypeInfo::name(UCHAR);
  else if ( typ == typeid(unsigned short) )
    return DataTypeInfo::name(USHORT);
  else if ( typ == typeid(unsigned int) )
    return DataTypeInfo::name(UINT);
  else if ( typ == typeid(unsigned long) )
    return DataTypeInfo::name(ULONG);
  else if ( typ == typeid(char) )
    return DataTypeInfo::name(CHAR);
  else if ( typ == typeid(short) )
    return DataTypeInfo::name(SHORT);
  else if ( typ == typeid(int) )
    return DataTypeInfo::name(INT);
  else if ( typ == typeid(long) )
    return DataTypeInfo::name(LONG);
  else if ( typ == typeid(bool) )
    return DataTypeInfo::name(BOOL);
  else if ( typ == typeid(float) )
    return DataTypeInfo::name(FLOAT);
  else if ( typ == typeid(double) )
    return DataTypeInfo::name(DOUBLE);
  else if ( typ == typeid(std::string) )
    return DataTypeInfo::name(STRING);
  else if ( typ == typeid(char*) )
    return DataTypeInfo::name(NTCHAR);
  else if ( typ == typeid(IOpaqueAddress*) )
    return DataTypeInfo::name(OBJECT_ADDR);
  else if ( typ == typeid(SmartRef<DataObject>) )
    return DataTypeInfo::name(OBJECT_REF);
  else if ( typ == typeid(SmartRef<ContainedObject>) )
    return DataTypeInfo::name(CONTAINED_REF);
  else if ( typ == typeid(void*) )
    return DataTypeInfo::name(POINTER);
  else
    return DataTypeInfo::name(UNKNOWN);
}
