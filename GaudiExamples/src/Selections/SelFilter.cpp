// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DataObjectHandle.h"
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
namespace Gaudi {
  namespace Examples {
    /** @class SelFilter
     *  Simple class to create few "containers" in TES
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-07-23
     */
    class SelFilter : public GaudiAlgorithm {
      // ======================================================================
      using Range     = Gaudi::Range_<Gaudi::Examples::MyTrack::ConstVector>;
      using Selection = Gaudi::Examples::MyTrack::Selection;

      DataObjectReadHandle<Range>      m_input{this, "Input", "", "TES location of input container"};
      DataObjectWriteHandle<Selection> m_output{this, "Output", this->name(), "TES location of output container"};
      // ======================================================================
    public:
      // ======================================================================
      /// Constructor
      SelFilter( const std::string& name, ISvcLocator* pSvcLocator ) : GaudiAlgorithm( name, pSvcLocator ) {}

      // using GaudiAlgorithm::GaudiAlgorithm;
      // ======================================================================
      /// the only one essential method
      StatusCode execute() override {

        static Rndm::Numbers flat( randSvc(), Rndm::Flat( -1, 1 ) );

        if ( exist<Gaudi::Examples::MyTrack::Selection>( m_input.objKey() ) ) {
          info() << "Selection at '" << m_input.objKey() << "'" << endmsg;
        } else if ( exist<Gaudi::Examples::MyTrack::Container>( m_input.objKey() ) ) {
          info() << "Container at '" << m_input.objKey() << "'" << endmsg;
        }

        if ( !exist<Range>( m_input.objKey() ) ) {
          err() << "No Range is available at location " << m_input.objKey() << endmsg;
        }

        // get input data in 'blind' way
        const auto& range = m_input.get();

        // create new selection
        auto sample = std::make_unique<Gaudi::Examples::MyTrack::Selection>();

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
        m_output.put( std::move( sample ) );

        return StatusCode::SUCCESS;
      }
    };
    // ========================================================================
  } // namespace Examples
} // end of namespace Gaudi
// ============================================================================
/// The factory (needed for instantiation)
using Gaudi::Examples::SelFilter;
DECLARE_COMPONENT( SelFilter )
// ============================================================================
// The END
// ============================================================================
