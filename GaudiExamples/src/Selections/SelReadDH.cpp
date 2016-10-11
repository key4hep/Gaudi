// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/AnyDataHandle.h"
#include "GaudiKernel/SharedObjectsContainer.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
// Local
// ============================================================================
#include "GaudiExamples/MyTrack.h"
// ============================================================================

namespace Gaudi
{
  namespace Examples
  {
    /** @class SelRead
     */
    class SelReadDH : public GaudiAlgorithm
    {
    public:
      // ======================================================================
      /// the only one essential method
      virtual StatusCode execute()
      {
        // Printing out the tracks
        {
          info() << "=========== Tracks ==========" << endmsg; 
          auto tracks = m_tracks.get();
          int i = 0;
          for(auto t:  *tracks) {
            info() <<  "(" << t->px() << ", " << t->py()
                   << ", " << t->pz() << ")" 
                   << endmsg;
            i++;
          }
          info() << "=========== Tracks count: " << i << endmsg;
        }

        // Printing out the tracks from the range
        {
        info() << "=========== Range Tracks ==========" << endmsg; 
        const Gaudi::NamedRange_<std::vector<const MyTrack *>> ranget = m_rangeTracks.get();
           auto range = m_rangeTracks.get();
           info() << typeid(ranget).name() << endmsg;

           int i = 0;
           for(auto t:  ranget) {
             info() <<  "(" << t->px() << ", " << t->py()
                    << ", " << t->pz() << ")" 
                    << endmsg;
             i++;
             }
             info() << "=========== Range Tracks count: " << i << endmsg;
           
           }

         // And then the selection
          {
           info() << "=========== Selected Tracks ==========" << endmsg;        
           auto selectedTracks = m_selectedTracks.get();
                     int i = 0;
           for(auto t:  selectedTracks) {
             std::cout <<  "(" << t->px() << ", " << t->py()
                       << ", " << t->pz() << ")" 
                       << std::endl;
             i++;
             }        
             info() << "=========== Selected Tracks count: " << i << endmsg;
             }

         return StatusCode::SUCCESS;
       }

       // ======================================================================
     public:
       // ======================================================================
       /** standard constructor
        *  @param name the algorithm instance name
        *  @param pSvc pointer to Service Locator
        */
       SelReadDH ( const std::string& name ,   //    the algorithm instance name
                   ISvcLocator*       pSvc )   // pointer to the Service Locator
         : GaudiAlgorithm ( name , pSvc ),
           m_tracks("/Event/Test/Tracks", Gaudi::DataHandle::Reader, this),
           m_selectedTracks("/Event/Test/SelectedTracks", Gaudi::DataHandle::Reader, this),
           m_rangeTracks("/Event/Test/Tracks", Gaudi::DataHandle::Reader, this)
       {
         declareProperty("tracks", m_tracks, "All tracks");
         declareProperty("selectedTracks", m_selectedTracks, "Selected tracks");
         declareProperty("rangeTracks", m_rangeTracks, "Range on selected tracks");
       }
       // ======================================================================
     private:
       // ======================================================================
       /// copy constructor is disabled
       SelReadDH(const SelReadDH&) = delete;
       /// assignement operator is disabled
       SelReadDH& operator=(const SelReadDH&) = delete;

       // ======================================================================
       AnyDataHandle<Gaudi::Examples::MyTrack::Container>  m_tracks;
      AnyDataHandle<Gaudi::Examples::MyTrack::Range>  m_selectedTracks; 
       AnyDataHandle<Gaudi::Examples::MyTrack::Range>  m_rangeTracks;

      // ======================================================================
    } ;
    // ========================================================================
  } // end of namespace Gaudi::Examples
} // end of namespace Gaudi
// ============================================================================
/// The factory (needed for instantiation)
using Gaudi::Examples::SelReadDH;
DECLARE_COMPONENT(SelReadDH)
// ============================================================================
// The END
// ============================================================================

