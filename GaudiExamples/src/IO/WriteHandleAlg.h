#ifndef HANDLEWRITE_ALG_H
#define HANDLEWRITE_ALG_H

#include "GaudiAlg/GaudiAlgorithm.h"

#include "GaudiKernel/DataObjectHandle.h"

// Event Model related classes
#include "GaudiExamples/Counter.h"
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"

using namespace Gaudi::Examples;

//------------------------------------------------------------------------------

class WriteHandleAlg : public GaudiAlgorithm
{
public:
  WriteHandleAlg( const std::string& n, ISvcLocator* l ) : GaudiAlgorithm( n, l ) {}

  bool isClonable() const override { return true; }

  /// the execution of the algorithm
  StatusCode execute() override;

private:
  Gaudi::Property<bool> m_useHandle{this, "UseHandle", true, "Specify the usage of the handle to write"};

  DataObjectWriteHandle<Collision> m_output_handle{this, "Output", "/Event/MyCollision"};
};

#endif
