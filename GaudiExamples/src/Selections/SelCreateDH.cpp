// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/AnyDataHandle.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
// Local
// ============================================================================
#include "GaudiExamples/MyTrack.h"
// ============================================================================


typedef Gaudi::NamedRange_<Gaudi::Examples::MyTrack::ConstVector> Range ;

namespace Gaudi
{
  namespace Examples
  {

    // Defining our track range
    /** @class SelCreateDH
     */
    class SelCreateDH : public GaudiAlgorithm
    {
    public:
      // ======================================================================
      /// the only one essential method
      StatusCode execute() override
      {
        // some random number generators, just to provide the numbers
        static Rndm::Numbers  gauss   ( randSvc () , Rndm::Gauss   (   0.0 ,   1.0 ) ) ;
        static Rndm::Numbers  flat    ( randSvc () , Rndm::Flat    (  20.0 , 100.0 ) ) ;

        // create the data
        Gaudi::Examples::MyTrack::Container  tracks;

        // fill it with some "data"
        for ( int i = 0 ; i < 20 ; ++i )
        {
          // create new track
          info () << "Adding track " << i << endmsg;

          Gaudi::Examples::MyTrack* track = new Gaudi::Examples::MyTrack() ;
          // fill it with some "data"
          track -> setPx ( gauss ()           ) ;
          track -> setPy ( gauss ()           ) ;
          track -> setPz ( gauss () + flat () ) ;

          // insert it into the container
          tracks.insert( track ) ;
        }

        // Filtering and creating the subrange
        Range range({ tracks.begin(), tracks.end()});
        Gaudi::Examples::MyTrack::Selection sample;
        const double pxCut =     0 ;

        // select particles with positive px
        sample.insert
          ( range.begin () ,
            range.end   () ,
            [pxCut](const Gaudi::Examples::MyTrack* track) {
            return track->px() > pxCut; });

        info() << "Sample size is "
               << sample.size()
               << "/" << range.size() << endmsg ;

        info() << "Before put - tracks size is "
               << tracks.size() << endmsg ;

        // register the container in TES
        auto ret = m_tracks.put (std::move(tracks));
        info() << ret << endmsg;
        info() << "After put - tracks size is "
               << tracks.size() << endmsg ;

        // register Selection in TES
        m_selectedTracks.put(std::move(sample)) ;

        return StatusCode::SUCCESS;
      }
      // ======================================================================
    public:
      // ======================================================================
      /** standard constructor
       *  @param name the algorithm instance name
       *  @param pSvc pointer to Service Locator
       */
      SelCreateDH ( const std::string& name ,   //    the algorithm instance name
                    ISvcLocator*       pSvc )   // pointer to the Service Locator
        : GaudiAlgorithm ( name , pSvc ),
          m_tracks("/Event/Test/Tracks", Gaudi::DataHandle::Writer, this),
          m_selectedTracks("/Event/Test/SelectedTracks", Gaudi::DataHandle::Writer, this)
      {
        declareProperty("tracks", m_tracks, "All tracks");
        declareProperty("selectedTracks", m_selectedTracks, "Selected tracks");
      }
      // ======================================================================
    private:
      // ======================================================================
      /// copy constructor is disabled
      SelCreateDH(const SelCreateDH&) = delete;
      /// assignement operator is disabled
      SelCreateDH& operator=(const SelCreateDH&) = delete;

      // ======================================================================
      AnyDataHandle<Gaudi::Examples::MyTrack::Container>  m_tracks;
      AnyDataHandle<Gaudi::Examples::MyTrack::Selection>  m_selectedTracks;

      // ======================================================================
    } ;
    // ========================================================================
  } // end of namespace Gaudi::Examples
} // end of namespace Gaudi
// ============================================================================
/// The factory (needed for instantiation)
using Gaudi::Examples::SelCreateDH;
DECLARE_COMPONENT(SelCreateDH)
// ============================================================================
// The END
// ============================================================================

