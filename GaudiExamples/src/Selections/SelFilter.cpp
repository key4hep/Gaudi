// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
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
    /** @class SelFilter
     *  Simple class to create few "containers" in TES
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-07-23
     */
    class SelFilter : public GaudiAlgorithm
    {
    public:
      // ======================================================================
      /// Constructor
      using GaudiAlgorithm::GaudiAlgorithm;
      // ======================================================================
      /// the only one essential method
      StatusCode execute() override
      {

        typedef Gaudi::NamedRange_<Gaudi::Examples::MyTrack::ConstVector> Range;

        static Rndm::Numbers flat( randSvc(), Rndm::Flat( -1, 1 ) );

        if ( exist<Gaudi::Examples::MyTrack::Selection>( m_input ) ) {
          info() << "Selection at '" << m_input.value() << "'" << endmsg;
        } else if ( exist<Gaudi::Examples::MyTrack::Container>( m_input ) ) {
          info() << "Container at '" << m_input.value() << "'" << endmsg;
        }

        if ( !exist<Range>( m_input ) ) {
          err() << "No Range is available at location " << m_input.value() << endmsg;
        }

        // get input data in 'blind' way
        Range range = get<Range>( m_input );

        // create new selection
        Gaudi::Examples::MyTrack::Selection* sample = new Gaudi::Examples::MyTrack::Selection();

        const double pxCut = flat();
        const double pyCut = flat();

        // select particles with 'large' px
        sample->insert( range.begin(), range.end(),
                        [pxCut]( const Gaudi::Examples::MyTrack* track ) { return track->px() > pxCut; } );

        const size_t size = sample->size();

        // remove the particles with 'small' py
        sample->erase( [pyCut]( const Gaudi::Examples::MyTrack* track ) { return track->py() < pyCut; } );

        info() << "Sample size is " << range.size() << "/" << size << "/" << sample->size() << endmsg;

        // register it in TES
        put( sample, name() );

        return StatusCode::SUCCESS;
      }
      // ======================================================================
    private:
      // ======================================================================
      Gaudi::Property<std::string> m_input{this, "Input", "", "TES location of input container"};
      // ======================================================================
    };
    // ========================================================================
  } // end of namespace Gaudi::Examples
} // end of namespace Gaudi
// ============================================================================
/// The factory (needed for instantiation)
using Gaudi::Examples::SelFilter;
DECLARE_COMPONENT( SelFilter )
// ============================================================================
// The END
// ============================================================================
