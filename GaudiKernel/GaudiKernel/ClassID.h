// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/ClassID.h,v 1.16 2006/11/13 15:10:42 hmd Exp $
#ifndef GAUDIKERNEL_CLASSID_H
#define GAUDIKERNEL_CLASSID_H

// Include files
#include "GaudiKernel/Kernel.h"

/// Class ID definition
typedef unsigned int CLID;

// Definition of the Class ID's needed by the Framework itself
// The ID's reserved range for the framework is 0-199
// For other ID's refer to the EventModel.cpp file in the Event Model package

static const CLID CLID_NULL    = 0;
static const CLID CLID_Catalog = 3;

static const CLID CLID_Run   = 100;
//static const CLID CLID_Event = 110;
static const CLID CLID_ObjectVector = 1<<17;  // ObjectVector (bit 17 set)
static const CLID CLID_ObjectList   = 1<<18;  // ObjectList   (bit 18 set)
static const CLID CLID_Any          = 0x0000FFFF;

static const CLID CLID_StatisticsFile      = 40;
static const CLID CLID_StatisticsDirectory = 41;

// CLIDs for Histograms
static const CLID CLID_H1D          = 32;     // 1D, gravity bin, fixed binning
static const CLID CLID_H1DVar       = 33;     // 1D, gravity bin, variable binning
static const CLID CLID_ProfileH     = 34;     // 1D, profile bin, fixed binning
static const CLID CLID_ProfileHVar  = 35;     // 1D, profile bin, variable binning
static const CLID CLID_H2D          = 36;     // 2D, weighted bin, fixed binning
static const CLID CLID_H2DF         = 37;     // 2D, float weighted bin, fixed binning
static const CLID CLID_H2DVar       = 38;     // 2D, weighted bin, variable binning
static const CLID CLID_ProfileH2    = 39;     // 2D, profile bin, fixed binnint
static const CLID CLID_H3D          = 50;     // 3D, weighted bin, fixed binning
static const CLID CLID_H3DF         = 51;     // 3D, float weighted bin, fixed binning
static const CLID CLID_H3DVar       = 52;     // 3D, weighted bin, variable binning

// CLIDs for the NTuples  
static const CLID CLID_NTupleFile       = CLID_StatisticsFile;
static const CLID CLID_NTupleDirectory  = CLID_StatisticsDirectory;
static const CLID CLID_RowWiseTuple     = 42; // Row-wise N-tuple
static const CLID CLID_ColumnWiseTuple  = 43; // Column-wise N-tuple

// CLIDs fot the AIDA tuples
// static const CLID CLID_AIDATuple        = 69; // AIDA Tuple

// Data storage technique identifiers
const long TEST_StorageType       = 0x00;

const long SICB_StorageType       = 0x01;
const long ROOT_StorageType       = 0x02;
const long OBJY_StorageType       = 0x03;
const long ESC_StorageType        = 0x04;
const long CDF_StorageType        = 0x05;
const long HBOOK_StorageType      = 0x06;
const long XML_StorageType        = 0x07;
const long DBOOMS_StorageType     = 0x08;
const long CONDDB_StorageType     = 0x09;
const long ACCESS_StorageType     = 0x0a;
const long EXCEL_StorageType      = 0x0b;
const long TEXTJET_StorageType    = 0x0c;
const long SQLSERVER_StorageType  = 0x0d;
const long MYSQL_StorageType      = 0x0e;
const long ORACLE_StorageType     = 0x0f;
const long XMLSTRING_StorageType  = 0x10;
const long POOL_StorageType       = 0x100;
const long POOL_ROOT_StorageType  = 0x200;
const long POOL_ROOTKEY_StorageType  = 0x201;
const long POOL_ROOTTREE_StorageType = 0x202;
const long POOL_ACCESS_StorageType   = 0x400;
const long POOL_MYSQL_StorageType    = 0x800;
const long POOL_ORACLE_StorageType   = 0x900;
const long RAWDATA_StorageType       = 0xA000000;
const long RAWDESC_StorageType       = 0xC000000;
const long MBM_StorageType           = 0xB000000;

// define pseudo-storages for GiGa Conversion Services
const long GiGaKine_StorageType   = 0x32; // = 50 (DEC)
const long GiGaGeom_StorageType   = 0x33; // = 51 (DEC)
const long GiGaTraj_StorageType   = 0x34; // 
const long GiGaHits_StorageType   = 0x35; // 



#endif // GAUDIKERNEL_CLASSID_H

