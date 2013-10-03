// $Id:
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.37 $
// ============================================================================
// Python must always be the first.
#include "Python.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ParticleProperty.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/PropertyCallbackFunctor.h"
#include "GaudiKernel/Chrono.h"
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/SerializeSTL.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/MapBase.h"
#include "GaudiKernel/Map.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/VectorMap.h"
#include "GaudiKernel/Range.h"

#ifdef _WIN32
#include "GaudiKernel/GaudiHandle.h"
#endif

#ifdef __ICC
// disable icc remark #177: declared but never referenced
#pragma warning(disable:177)
// disable icc warning #1125: function "C::X()" is hidden by "Y::X" -- virtual function override intended?
#pragma warning(disable:1125)
#endif

// Force visibility of the classes
#define class class GAUDI_API
#ifdef _WIN32
#include "AIDA/IAnnotation.h"
#endif
#include "AIDA/IHistogram.h"
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
#include "AIDA/IAxis.h"
#undef class

#include "GaudiPython/Helpers.h"

#include "GaudiPython/GaudiPython.h"
#include "GaudiPython/Interface.h"
#include "GaudiPython/Algorithm.h"
#include "GaudiPython/AlgDecorators.h"
#include "GaudiPython/HistoDecorator.h"
#include "GaudiPython/TupleDecorator.h"
#include "GaudiPython/Printer.h"
#include "GaudiPython/CallbackStreamBuf.h"

#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/Print.h"
#include "GaudiAlg/GaudiTupleAlg.h"
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiAlg/GaudiHistoID.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiCommon.h"
#include "GaudiAlg/IErrorTool.h"
#include "GaudiAlg/IGenericTool.h"
#include "GaudiAlg/IHistoTool.h"
#include "GaudiAlg/ITupleTool.h"
#include "GaudiAlg/ISequencerTimerTool.h"
// Added to avoid warnings about inlined functions never implemented.
#include "GaudiAlg/GaudiHistos.icpp"

#include "GaudiUtils/Aida2ROOT.h"
#include "GaudiUtils/IFileCatalog.h"
#include "GaudiUtils/IFileCatalogMgr.h"
#include "GaudiUtils/IIODataManager.h"
#include "GaudiUtils/HistoStats.h"
#include "GaudiUtils/HistoDump.h"
#include "GaudiUtils/HistoStrings.h"

#include <iostream>
#include <istream>
#include <sstream>


template class std::basic_stringstream<char>;

// needed to find operator<< implemented in GaudiUtils.
using namespace GaudiUtils;

namespace GaudiPython
{

  template <class TYPE>
  struct _Property
  {
    TYPE                       m_type ;
    SimpleProperty<TYPE>       m_prop ;
    SimplePropertyRef<TYPE>    m_ref  ;
    // constructor
    _Property()
      : m_type()
      , m_prop()
      , m_ref ( "" , m_type )
    {}
    //
  } ;

  //--- Template instantiations
  struct __Instantiations
  {
    std::vector<Property*>              i0      ;
    std::vector<const Property*>        i00     ;
    std::vector<IRegistry*>             i00000  ;

    std::allocator<IRegistry*>          a0  ;
    std::allocator<IAlgorithm*>         a1  ;
    std::allocator<IService*>           a2  ;

    std::list<IAlgorithm*>              i01     ;
    std::list<IService*>                i02     ;
    std::list<const IFactory*>          i023    ;

    std::vector<IService*>              i05_1   ;
    std::vector<IAlgTool*>              i05_2   ;
    std::vector<const StatEntity*>      i05_3   ;
    std::vector<GaudiAlg::ID>           i05_4   ;
    std::vector<AIDA::IHistogram1D*>    i05_5   ;
    std::vector<AIDA::IHistogram2D*>    i05_6   ;
    std::vector<AIDA::IHistogram3D*>    i05_7   ;
    std::vector<AIDA::IProfile1D*>      i05_8   ;
    std::vector<AIDA::IProfile2D*>      i05_9   ;

    //Gaudi::IIODataManager              *gu_i1000;

    GaudiUtils::VectorMap<int,double>   i034 ;


    GaudiPython::PyAlg<GaudiAlgorithm>  _alg0 ;
    GaudiPython::PyAlg<GaudiHistoAlg>   _alg1 ;
    GaudiPython::PyAlg<GaudiTupleAlg>   _alg2 ;

    GaudiPython::Matrix _mtrx ;
    GaudiPython::Vector _vctr ;
    std::vector<std::vector<double> > _vct1 ;

    // primitives:
    _Property<bool>                                               pp_01 ;
    _Property<char>                                               pp_02 ;
    _Property<signed char>                                        pp_03 ;
    _Property<unsigned char>                                      pp_04 ;
    _Property<short>                                              pp_05 ;
    _Property<unsigned short>                                     pp_06 ;
    _Property<int>                                                pp_07 ;
    _Property<unsigned int>                                       pp_08 ;
    _Property<long>                                               pp_09 ;
    _Property<unsigned long>                                      pp_10 ;
    _Property<long long>                                          pp_11 ;
    _Property<unsigned long long>                                 pp_12 ;
    _Property<float>                                              pp_13 ;
    _Property<double>                                             pp_14 ;
    _Property<long double>                                        pp_15 ;
    _Property<std::string>                                        pp_16 ;

    // vectors of primitives
    _Property<std::vector<bool> >                                 vp_01 ;
    _Property<std::vector<char> >                                 vp_02 ;
    _Property<std::vector<signed char> >                          vp_03 ;
    _Property<std::vector<unsigned char> >                        vp_04 ;
    _Property<std::vector<short> >                                vp_05 ;
    _Property<std::vector<unsigned short> >                       vp_06 ;
    _Property<std::vector<int> >                                  vp_07 ;
    _Property<std::vector<unsigned int> >                         vp_08 ;
    _Property<std::vector<long> >                                 vp_09 ;
    _Property<std::vector<unsigned long> >                        vp_10 ;
    _Property<std::vector<long long> >                            vp_11 ;
    _Property<std::vector<unsigned long long> >                   vp_12 ;
    _Property<std::vector<float> >                                vp_13 ;
    _Property<std::vector<double> >                               vp_14 ;
    _Property<std::vector<long double> >                          vp_15 ;
    _Property<std::vector<std::string> >                          vp_16 ;

    // some extended types
    _Property<std::pair<int,int> >                                ep_01 ;
    _Property<std::pair<double,double> >                          ep_02 ;
    _Property<std::vector<std::pair<double,double> > >            ep_04 ;
    _Property<std::vector<std::vector<std::string> > >            ep_05 ;
    _Property<std::vector<std::vector<double> > >                 ep_06 ;
    _Property<std::map<int,double> >                              ep_07 ;
    _Property<std::map<std::string,std::string> >                 ep_08 ;
    _Property<std::map<std::string,int> >                         ep_09 ;
    _Property<std::map<std::string,double> >                      ep_10 ;
    _Property<std::map<std::string,std::vector<std::string> > >   ep_11 ;
    _Property<std::map<std::string,std::vector<int> > >           ep_12 ;
    _Property<std::map<std::string,std::vector<double> > >        ep_13 ;

    __Instantiations  () ;
    ~__Instantiations () ;

  };

} // end of namespace GaudiPython

namespace __gnu_cxx { struct dummy {}; }  // hack to please CINT

#ifdef _WIN32
#pragma warning ( disable : 4345 )
#pragma warning ( disable : 4624 )
#endif

#ifdef __ICC
// disable icc warning #191: type qualifier is meaningless on cast type
// ... a lot of noise produced by the dictionary
#pragma warning(disable:191)
#endif

// ============================================================================
// The END
// ============================================================================
