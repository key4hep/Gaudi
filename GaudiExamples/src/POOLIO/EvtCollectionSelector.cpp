// $Id: EvtCollectionSelector.cpp,v 1.8 2007/09/28 11:48:17 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.8 $
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
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/Selector.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/ToStream.h"
// ============================================================================
// Local
// ============================================================================
#include "MyTrack.h"
// ============================================================================
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
      NTuple::Item<int>                       m_ntrack ;
      NTuple::Array<float>                    m_trkMom ;
      NTuple::Item<Gaudi::Examples::MyTrack*> m_track  ;
      int                                     m_cut    ;
    public:
      EvtCollectionSelector ( IInterface* svc ) 
        : NTuple::Selector  ( svc )   
        , m_cut             ( 10  ) 
      {}
      virtual ~EvtCollectionSelector()   { }

      /// Initialization
      virtual StatusCode initialize ( NTuple::Tuple* nt )    
      {
        StatusCode sc = StatusCode::SUCCESS ;
        sc = nt->item ( "TrkMom" , m_trkMom ) ;
        if ( !sc.isSuccess() ) {
          std::cerr << "EvtCollectionSelector: initialize " << sc << std::endl;
          std::cerr << "TrkMom" << std::endl;
          return sc;
        }
        
        sc = nt->item ( "Ntrack" , m_ntrack ) ;
        if ( !sc.isSuccess() ) {
          std::cerr << "EvtCollectionSelector: initialize " << sc << std::endl;
          std::cerr << "Ntrack" << std::endl;
          return sc;
        }

        sc = nt->item ( "Track" , m_track ) ;
        if ( !sc.isSuccess() ) {
          std::cerr << "EvtCollectionSelector: initialize " << sc << std::endl;
          std::cerr << "TrkMom" << std::endl;
          return sc;
        }

        return sc ;
      }
      /// Specialized callback for NTuples
      virtual bool operator() ( NTuple::Tuple* /* nt */ )    
      {
        const int n = m_ntrack ;
        std::cout << System::typeinfoName ( typeid ( *this ) ) 
                  << "\t -> #tracks : " << n << std::endl
                  << System::typeinfoName ( typeid ( *this ) )
                  << "\t -> Momenta : ";
        for ( int i = 0 ; i < std::min ( 5 , n ) ; ++i )  
        { std::cout << "[" << i << "]=" << m_trkMom[i] << " "; }
        std::cout << std::endl;
        std::cout << System::typeinfoName ( typeid ( *this ) ) ;
        if ( 0 != *m_track ) 
        { 
          std::cout << "\t -> Track : " 
                    << " px=" << (*m_track)->px() 
                    << " py=" << (*m_track)->py() 
                    << " pz=" << (*m_track)->pz() << std::endl;
        }
        else { std::cout << " Track* is NULL" << std::endl ; }
        //
        const bool selected = m_cut < m_ntrack && 0 != *m_track ;
        std::cout << System::typeinfoName ( typeid ( *this ) ) 
                  << "   SELECTED : " ;
        Gaudi::Utils::toStream ( selected , std::cout ) << std::endl ;
        //
        return selected ;                         // RETURN 
      }
    };
  } // end of namespace Gaudi::Exmaples 
} // end of namespace Gaudi
// ============================================================================
DECLARE_NAMESPACE_OBJECT_FACTORY(Gaudi::Examples,EvtCollectionSelector) ;
// ============================================================================
// The END 
// ============================================================================
