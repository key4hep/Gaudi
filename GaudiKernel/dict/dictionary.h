/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define ALLOW_ALL_TYPES
#include <cstdlib>
#include <vector>

// we do not care about deprecation warnings (as they may arise from stub functions)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <Gaudi/Property.h>
#include <GaudiKernel/AlgTool.h>
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/Bootstrap.h>
#include <GaudiKernel/ContainedObject.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/DataStreamTool.h>
#include <GaudiKernel/Debugger.h>
#include <GaudiKernel/Environment.h>
#include <GaudiKernel/EventSelectorDataStream.h>
#include <GaudiKernel/GaudiHandle.h>
#include <GaudiKernel/GenericAddress.h>
#include <GaudiKernel/KeyedObject.h>
#include <GaudiKernel/LinkManager.h>
#include <GaudiKernel/Memory.h>
#include <GaudiKernel/NTuple.h>
#include <GaudiKernel/NTupleImplementation.h>
#include <GaudiKernel/ObjectVector.h>
#include <GaudiKernel/RegistryEntry.h>
#include <GaudiKernel/RndmGenerators.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/SmartDataPtr.h>
#include <GaudiKernel/SmartRefVector.h>
#include <GaudiKernel/System.h>
#include <GaudiKernel/Time.h>

#include <Gaudi/IAuditor.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/IAddressCreator.h>
#include <GaudiKernel/IAlgContextSvc.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IAuditorSvc.h>
#include <GaudiKernel/IChronoStatSvc.h>
#include <GaudiKernel/IClassInfo.h>
#include <GaudiKernel/IClassManager.h>
#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/IConverter.h>
#include <GaudiKernel/ICounterSummarySvc.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IDataSelector.h>
#include <GaudiKernel/IDataSourceMgr.h>
#include <GaudiKernel/IDataStoreAgent.h>
#include <GaudiKernel/IDataStoreLeaves.h>
#include <GaudiKernel/IDataStreamTool.h>
#include <GaudiKernel/IDetDataSvc.h>
#include <GaudiKernel/IEventProcessor.h>
#include <GaudiKernel/IEventTimeDecoder.h>
#include <GaudiKernel/IEvtSelector.h>
#include <GaudiKernel/IExceptionSvc.h>
#include <GaudiKernel/IHistogramSvc.h>
#include <GaudiKernel/IHistorySvc.h>
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/IInspectable.h>
#include <GaudiKernel/IInspector.h>
#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/IMagneticFieldSvc.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/IMonitorSvc.h>
#include <GaudiKernel/INTuple.h>
#include <GaudiKernel/INTupleSvc.h>
#include <GaudiKernel/INamedInterface.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/IPartPropSvc.h>
#include <GaudiKernel/IPartitionControl.h>
#include <GaudiKernel/IPersistencySvc.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/IRndmEngine.h>
#include <GaudiKernel/IRndmGen.h>
#include <GaudiKernel/IRndmGenSvc.h>
#include <GaudiKernel/IRunable.h>
#include <GaudiKernel/ISelectStatement.h>
#include <GaudiKernel/ISerialize.h>
#include <GaudiKernel/IService.h>
#include <GaudiKernel/IStagerSvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/ISvcManager.h>
#include <GaudiKernel/ITHistSvc.h>
#include <GaudiKernel/IToolSvc.h>
#include <GaudiKernel/IUpdateManagerSvc.h>
#include <GaudiKernel/IUpdateable.h>
#include <GaudiKernel/IValidity.h>
#include <GaudiKernel/Incident.h>

namespace GaudiKernelDict {
  struct __Instantiations {
    KeyedObject<int>                           m_1;
    KeyedObject<unsigned int>                  m_2;
    KeyedObject<long>                          m_3;
    KeyedObject<unsigned long>                 m_4;
    SmartRef<DataObject>                       m_5;
    SmartRef<ContainedObject>                  m_6;
    SmartRef<ObjectContainerBase>              m_7;
    SmartRef<KeyedObject<int>>                 m_8;
    SmartRef<KeyedObject<unsigned int>>        m_9;
    SmartRef<KeyedObject<long>>                m_10;
    SmartRef<KeyedObject<unsigned long>>       m_11;
    SmartRefVector<DataObject>                 m_12;
    SmartRefVector<ContainedObject>            m_13;
    SmartRefVector<ObjectContainerBase>        m_14;
    SmartRefVector<KeyedObject<int>>           m_15;
    SmartRefVector<KeyedObject<unsigned int>>  m_16;
    SmartRefVector<KeyedObject<long>>          m_17;
    SmartRefVector<KeyedObject<unsigned long>> m_18;
    //   std::vector<SmartRef<ContainedObject> > m_19;
    //   std::vector<SmartRef<DataObject> > m_20;
    //   std::vector<SmartRef<ObjectContainerBase> > m_21;
    std::vector<LinkManager::Link*>     m_22;
    std::vector<const ContainedObject*> m_23;
    std::vector<ContainedObject*>       m_24;
    NTuple::Item<bool>                  BoolItem;
    NTuple::Item<char>                  CharItem;
    NTuple::Item<unsigned char>         UCharItem;
    NTuple::Item<short>                 ShortItem;
    NTuple::Item<unsigned short>        UShortItem;
    NTuple::Item<long>                  LongItem;
    NTuple::Item<long long>             LongLongItem;
    NTuple::Item<unsigned long>         ULongItem;
    NTuple::Item<unsigned long long>    ULongLongItem;
    NTuple::Item<int>                   IntItem;
    NTuple::Item<unsigned int>          UIntItem;
    NTuple::Item<float>                 FloatItem;
    NTuple::Item<double>                DoubleItem;
    NTuple::Array<bool>                 BoolArray;
    NTuple::Array<char>                 CharArray;
    NTuple::Array<unsigned char>        UCharArray;
    NTuple::Array<short>                ShortArray;
    NTuple::Array<unsigned short>       UShortArray;
    NTuple::Array<long>                 LongArray;
    NTuple::Array<unsigned long>        ULongArray;
    NTuple::Array<int>                  IntArray;
    NTuple::Array<unsigned int>         UIntArray;
    NTuple::Array<float>                FloatArray;
    NTuple::Array<double>               DoubleArray;
    NTuple::Matrix<bool>                BoolMatrix;
    NTuple::Matrix<char>                CharMatrix;
    NTuple::Matrix<unsigned char>       UCharMatrix;
    NTuple::Matrix<short>               ShortMatrix;
    NTuple::Matrix<unsigned short>      UShortMatrix;
    NTuple::Matrix<long>                LongMatrix;
    NTuple::Matrix<unsigned long>       ULongMatrix;
    NTuple::Matrix<int>                 IntMatrix;
    NTuple::Matrix<unsigned int>        UIntMatrix;
    NTuple::Matrix<float>               FloatMatrix;
    NTuple::Matrix<double>              DoubleMatrix;

    SmartDataPtr<DataObject>          p1{ 0, "" };
    SmartDataPtr<ObjectContainerBase> p2{ 0, "" };
  };
} // namespace GaudiKernelDict

class IUpdateManagerSvc::PythonHelper {
public:
  static StatusCode update( IUpdateManagerSvc* ums, void* obj ) { return ums->i_update( obj ); }

  static void invalidate( IUpdateManagerSvc* ums, void* obj ) { return ums->i_invalidate( obj ); }
};

#ifdef _WIN32
// FIXME: (MCl) The generated dictionary produce a few warnings C4345, since I
//              cannot fix them, I just disable them.

// Disable warning C4345: behavior change: an object of POD type constructed with an initializer of the
//                        form () will be default-initialized
#  pragma warning( disable : 4345 )
#endif

#ifdef __ICC
// disable icc warning #858: type qualifier on return type is meaningless
// ... a lot of noise produced by the dictionary
#  pragma warning( disable : 858 )
// disable icc remark #2259: non-pointer conversion from "int" to "const char &" may lose significant bits
//    Strange, things like NTuple::Item<char> produce this warning, as if the operation between chars are done
//    converting them to integers first.
#  pragma warning( disable : 2259 )
// disable icc remark #177: variable "X" was declared but never referenced
#  pragma warning( disable : 177 )
#endif
