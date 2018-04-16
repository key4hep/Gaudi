// ============================================================================
/** @file
 * 	@author    : Markus Frank
 */
// ============================================================================
#define EVTCOLLECTIONSELECTOR_CPP 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/Selector.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/ToStream.h"
// ============================================================================
// Local
// ============================================================================
#include "GaudiExamples/MyTrack.h"
// ============================================================================
#ifdef __ICC
// disable icc warning #1125: function "ISelectStatement::operator()(void *)"
//           is hidden by "Gaudi::Examples::EvtCollectionSelector::operator()"
//            -- virtual function override intended?
#pragma warning( disable : 1125 )
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
#pragma warning( disable : 1572 )
#endif
namespace Gaudi
{
  namespace Examples
  {
    /** @class EvtCollectionSelector
     *  Definition of a small class for further pre-selections
     *  when reading an event collection.
     *  @author:  M.Frank
     *  Version: 1.0
     */
    class EvtCollectionSelector : public NTuple::Selector
    {
    protected:
      NTuple::Item<int>    m_ntrack;
      NTuple::Array<float> m_trkMom;
#ifndef NO_TRKMOMFIX
      NTuple::Array<float> m_trkMomFixed;
#endif
      NTuple::Item<Gaudi::Examples::MyTrack*> m_track;
      int                                     m_cut;

    public:
      EvtCollectionSelector( IInterface* svc ) : NTuple::Selector( svc ), m_cut( 10 ) {}
      ~EvtCollectionSelector() override = default;

      /// Initialization
      StatusCode initialize( NTuple::Tuple* nt ) override
      {
        StatusCode sc = StatusCode::SUCCESS;
        sc            = nt->item( "TrkMom", m_trkMom );
        if ( !sc.isSuccess() ) {
          std::cerr << "EvtCollectionSelector: initialize " << sc << std::endl;
          std::cerr << "TrkMom" << std::endl;
          return sc;
        }
#ifndef NO_TRKMOMFIX
        sc = nt->item( "TrkMomFix", m_trkMomFixed );
        if ( !sc.isSuccess() ) {
          std::cerr << "EvtCollectionSelector: initialize " << sc << std::endl;
          std::cerr << "TrkMomFix" << std::endl;
          return sc;
        }
#endif
        sc = nt->item( "Ntrack", m_ntrack );
        if ( !sc.isSuccess() ) {
          std::cerr << "EvtCollectionSelector: initialize " << sc << std::endl;
          std::cerr << "Ntrack" << std::endl;
          return sc;
        }

        sc = nt->item( "Track", m_track );
        if ( !sc.isSuccess() ) {
          std::cerr << "EvtCollectionSelector: initialize " << sc << std::endl;
          std::cerr << "Track" << std::endl;
          return sc;
        }

        return sc;
      }

      using NTuple::Selector::operator(); // avoid hiding base-class methods
      /// Specialized callback for NTuples
      bool operator()( NTuple::Tuple* /* nt */ ) override
      {
        const int n = m_ntrack;
        std::cout << System::typeinfoName( typeid( *this ) ) << "\t -> #tracks : " << n << std::endl
                  << System::typeinfoName( typeid( *this ) ) << "\t -> Momenta(Var): ";
        for ( int i = 0; i < std::min( 5, n ); ++i ) {
          std::cout << "[" << i << "]=" << m_trkMom[i] << " ";
        }
#ifndef NO_TRKMOMFIX
        std::cout << std::endl << System::typeinfoName( typeid( *this ) ) << "\t -> Momenta(Fix): ";
        for ( int i = 0; i < std::min( 5, n ); ++i ) {
          std::cout << "[" << i << "]=" << m_trkMomFixed[i] << " ";
        }
        for ( int i = 5; i < 99; ++i ) {
          if ( m_trkMomFixed[i] != 0.f ) {
            std::cout << "[" << i << "]= Error in Fixed momentum" << std::endl;
          }
        }
#endif
        std::cout << std::endl;
        std::cout << System::typeinfoName( typeid( *this ) );
        if ( 0 != *m_track ) {
          std::cout << "\t -> Track : "
                    << " px=" << ( *m_track )->px() << " py=" << ( *m_track )->py() << " pz=" << ( *m_track )->pz()
                    << std::endl;
        } else {
          std::cout << " Track* is NULL" << std::endl;
        }
        //
        const bool selected = m_cut < m_ntrack && 0 != *m_track;
        std::cout << System::typeinfoName( typeid( *this ) ) << "   SELECTED : ";
        Gaudi::Utils::toStream( selected, std::cout ) << std::endl;
        //
        return selected; // RETURN
      }
    };
  } // end of namespace Gaudi::Exmaples
} // end of namespace Gaudi
// ============================================================================
DECLARE_NAMESPACE_OBJECT_FACTORY( Gaudi::Examples, EvtCollectionSelector )
// ============================================================================
// The END
// ============================================================================
