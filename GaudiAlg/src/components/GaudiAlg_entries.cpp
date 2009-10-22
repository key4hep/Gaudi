#include "GaudiAlg/EventCounter.h"
#include "GaudiAlg/Prescaler.h"
#include "GaudiAlg/Sequencer.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiSequencer.h"
#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_ALGORITHM_FACTORY( EventCounter   );
DECLARE_ALGORITHM_FACTORY( Prescaler      );
DECLARE_ALGORITHM_FACTORY( Sequencer      );
DECLARE_ALGORITHM_FACTORY( GaudiAlgorithm );
DECLARE_ALGORITHM_FACTORY( GaudiSequencer );

DECLARE_FACTORY_ENTRIES( GaudiAlg ) {
    DECLARE_ALGORITHM( EventCounter   );
    DECLARE_ALGORITHM( Prescaler      );
    DECLARE_ALGORITHM( Sequencer      );
    DECLARE_ALGORITHM( GaudiAlgorithm );
    DECLARE_ALGORITHM( GaudiSequencer );
    DECLARE_TOOL( SequencerTimerTool );
    DECLARE_TOOL( ErrorTool  ) ;
    DECLARE_TOOL( HistoTool  ) ;
    DECLARE_TOOL( TupleTool  ) ;
}
