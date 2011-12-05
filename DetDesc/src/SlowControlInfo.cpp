//$Id: SlowControlInfo.cpp,v 1.2 2006-04-20 14:39:56 ranjard Exp $

#include "DetDesc/SlowControlInfo.h"

//---------------------------------------------------------------------------

/// Constructor
SlowControlInfo::SlowControlInfo( IDetectorElement* de,
				  const std::string& condition )
  : base_class( de, condition )
{
}

//---------------------------------------------------------------------------

/// Destructor
SlowControlInfo::~SlowControlInfo()
{
}

//----------------------------------------------------------------------------
