// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/DataTypeInfo.h,v 1.6 2008/10/27 19:22:20 marcocle Exp $
#ifndef GAUDIKERNEL_DATATYPES_H
#define GAUDIKERNEL_DATATYPES_H

#include <string>
#include <cstring>
#include "GaudiKernel/SmartRef.h"

class DataObject;
class ContainedObject;
class IOpaqueAddress;

/** @class DataTypeInfo DataTypeInfo.h GaudiKernel/DataTypeInfo.h

    Small class which allows access to internal type IDs.
*/
class GAUDI_API DataTypeInfo {
private:
  // This object cannot be instantiated!
  DataTypeInfo() {}
public:
  // Accepted Data type definitions
  enum Type  {
        UNKNOWN = 0,
        UCHAR,        USHORT,        UINT,        ULONG,
        CHAR,         SHORT,         INT,         LONG,
        BOOL,         FLOAT,         DOUBLE,
        STRING,       NTCHAR,
        OBJECT_REF, CONTAINED_REF, POINTER, OBJECT_ADDR,
        LONG_STRING,  LONG_NTCHAR, LONGLONG,  ULONGLONG
  };
  /// Access to type information: bool
  static Type ID( const bool)             { return BOOL;          }
  /// Access to type information: char
  static Type ID( const char)             { return CHAR;          }
  /// Access to type information: short
  static Type ID( const short)            { return SHORT;         }
  /// Access to type information: int
  static Type ID( const int)              { return INT;           }
  /// Access to type information: long
  static Type ID( const long)             { return LONG;          }
  /// Access to type information: long long
  static Type ID( const long long)        { return LONGLONG;      }
  /// Access to type information: unsigned char
  static Type ID( const unsigned char)    { return UCHAR;         }
  /// Access to type information: unsigned short
  static Type ID( const unsigned short)   { return USHORT;        }
  /// Access to type information: unsigned int
  static Type ID( const unsigned int)     { return UINT;          }
  /// Access to type information: unsigned long
  static Type ID( const unsigned long)    { return ULONG;         }
  /// Access to type information: unsigned long long
  static Type ID( const unsigned long long) { return ULONGLONG;   }
  /// Access to type information: float
  static Type ID( const float)            { return FLOAT;         }
  /// Access to type information: float
  static Type ID( const double)           { return DOUBLE;        }
  /// Access to type information: std::string
  static Type ID( const std::string&)     { return STRING;        }
  /// Access to type information: char* (NULL terminated)
  static Type ID( const char*)            { return NTCHAR;        }
  /// Access to type information: IOpaqueAddress
  static Type ID( const IOpaqueAddress*)  { return OBJECT_ADDR;   }
  /// Access to type information: Pointers
  static Type ID( const void*)            { return POINTER;       }
  /// Access to type information: DataObject
  static Type ID( const SmartRef<DataObject>&)      { return OBJECT_REF;    }
  /// Access to type information: Contained object
  static Type ID( const SmartRef<ContainedObject>&) { return CONTAINED_REF; }

  /// Access to type information: the reverse way
  static Type ID( const std::type_info& typ )   {
    if ( typ == typeid(unsigned char) )
      return UCHAR;
    else if ( typ == typeid(unsigned short) )
      return USHORT;
    else if ( typ == typeid(unsigned int) )
      return UINT;
    else if ( typ == typeid(unsigned long) )
      return ULONG;
    else if ( typ == typeid(unsigned long long) )
      return ULONGLONG;
    else if ( typ == typeid(char) )
      return CHAR;
    else if ( typ == typeid(short) )
      return SHORT;
    else if ( typ == typeid(int) )
      return INT;
    else if ( typ == typeid(long) )
      return LONG;
    else if ( typ == typeid(long long) )
      return LONGLONG;
    else if ( typ == typeid(bool) )
      return BOOL;
    else if ( typ == typeid(float) )
      return FLOAT;
    else if ( typ == typeid(double) )
      return DOUBLE;
    else if ( typ == typeid(std::string) )
      return STRING;
    else if ( typ == typeid(char*) )
      return NTCHAR;
    else if ( typ == typeid(SmartRef<DataObject>) )
      return OBJECT_REF;
    else if ( typ == typeid(SmartRef<ContainedObject>) )
      return CONTAINED_REF;
    else if ( typ == typeid(IOpaqueAddress*) )
      return OBJECT_ADDR;
    else if ( typ == typeid(void*) )
      return POINTER;
    else
      return UNKNOWN;
  }

  /// Access to type information: the reverse way
  static const std::type_info& type( long typ )   {
    switch(typ)   {
    case UCHAR:
      return typeid(unsigned char);
    case USHORT:
      return typeid(unsigned short);
    case UINT:
      return typeid(unsigned int);
    case ULONG:
      return typeid(unsigned long);
    case ULONGLONG:
      return typeid(unsigned long long);
    case CHAR:
      return typeid(char);
    case SHORT:
      return typeid(short);
    case INT:
      return typeid(int);
    case LONG:
      return typeid(long);
    case LONGLONG:
      return typeid(long long);
    case BOOL:
      return typeid(bool);
    case FLOAT:
      return typeid(float);
    case DOUBLE:
      return typeid(double);
    case LONG_STRING:
      return typeid(std::string);
    case STRING:
      return typeid(std::string);
    case NTCHAR:
      return typeid(char*);
    case LONG_NTCHAR:
      return typeid(char*);
    case OBJECT_REF:
      return typeid(SmartRef<DataObject>);
    case CONTAINED_REF:
      return typeid(SmartRef<ContainedObject>);
    case OBJECT_ADDR:
      return typeid(IOpaqueAddress*);
    case POINTER:
    case UNKNOWN:
    default:
      return typeid(void*);
    }
  }

  /// Access to type information: the reverse way
  static long size( long typ )   {
    switch(typ)   {
    case UCHAR:
      return sizeof(unsigned char);
    case USHORT:
      return sizeof(unsigned short);
    case UINT:
      return sizeof(unsigned int);
    case ULONG:
      return sizeof(unsigned long);
    case ULONGLONG:
      return sizeof(unsigned long long);
    case CHAR:
      return sizeof(char);
    case SHORT:
      return sizeof(short);
    case INT:
      return sizeof(int);
    case LONG:
      return sizeof(long);
    case LONGLONG:
      return sizeof(long long);
    case BOOL:
      return sizeof(bool);
    case FLOAT:
      return sizeof(float);
    case DOUBLE:
      return sizeof(double);
    case STRING:
      return sizeof(std::string);
    case LONG_STRING:
      return sizeof(std::string);
    case NTCHAR:
      return sizeof(char*);
    case LONG_NTCHAR:
      return sizeof(char*);
    case OBJECT_ADDR:
    case POINTER:
    case OBJECT_REF:
    case CONTAINED_REF:
    case UNKNOWN:
    default:
      return 0;
    }
  }

  /// Access to type information: the reverse way
  static long size( const std::type_info& typ )   {
    return size( ID(typ) );
  }

  /// Copy data
  static int copy( void* tar, const void* src, long typ, int numObj )   {
    switch(typ)   {
    case UCHAR:      numObj *= sizeof(unsigned char);       break;
    case USHORT:     numObj *= sizeof(unsigned short);      break;
    case UINT:       numObj *= sizeof(unsigned int);        break;
    case ULONG:      numObj *= sizeof(unsigned long);       break;
    case ULONGLONG:  numObj *= sizeof(unsigned long long);  break;
    case CHAR:       numObj *= sizeof(char);                break;
    case SHORT:      numObj *= sizeof(short);               break;
    case INT:        numObj *= sizeof(int);                 break;
    case LONG:       numObj *= sizeof(long);                break;
    case LONGLONG:   numObj *= sizeof(long long);           break;
    case BOOL:       numObj *= sizeof(bool);                break;
    case FLOAT:      numObj *= sizeof(float);               break;
    case DOUBLE:     numObj *= sizeof(double);              break;
    case UNKNOWN:
    default:         numObj *= 0;                           break;
    }
    memcpy(tar, src, numObj);
    return numObj;
  }

  // Access the type name by type ID
  static std::string name(long typ);
  // Access the type name by type_info
  static std::string name(const std::type_info& typ);
  /// Access to type information
  static Type idByName( const std::string& typ );
  /// Access to type information
  static const std::type_info& typeByName( const std::string& typ );
};
#endif // GAUDIKERNEL_DATATYPES_H
