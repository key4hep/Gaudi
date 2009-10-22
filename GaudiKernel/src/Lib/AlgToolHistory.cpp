///////////////////////////////////////////////////////////////////////////
//
// $Id: AlgToolHistory.cpp,v 1.1 2006/11/09 10:24:05 mato Exp $
//
// GaudiHistory/AlgToolHistory.cpp
//
// Contains history information for an AlgTool
//
//
// @author: Charles Leggett
//
///////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/AlgToolHistory.h"
#include "GaudiKernel/JobHistory.h"
#include "GaudiKernel/AlgTool.h"

#include <iostream>

using std::ostream;
using std::endl;

AlgToolHistory::AlgToolHistory(const AlgTool& alg, const JobHistory* job)
: 
  m_type(alg.type()),
  m_version("UNKNOWN"),
  m_name(alg.name()),
  m_tool( &alg ),
  m_properties(alg.getProperties()),
  m_jobHistory(job)
{
}

//**********************************************************************

AlgToolHistory::AlgToolHistory(const std::string& algVersion,
			       const std::string& algName,
			       const std::string& algType,
			       const AlgTool* tool,
			       const PropertyList& props,
			       const JobHistory* job):
  m_type(algType),   // FIXME type_info???
  m_version(algVersion),
  m_name(algName),
  m_tool(tool),
  m_properties(props),
  m_jobHistory(job)
{}

//**********************************************************************

const CLID& AlgToolHistory::classID() {

  static CLID CLID_AlgToolHistory = 171959758;   // from `clid AlgToolHistory`
  return CLID_AlgToolHistory;

}

//**********************************************************************

ostream& operator<<(ostream& lhs, const AlgToolHistory& rhs) {
  lhs << "Type: " << rhs.algtool_type() << endl;
  lhs << "Name: " << rhs.algtool_name() << endl;
  lhs << "Version: " << rhs.algtool_version() << endl;
  lhs << "Parent: " << rhs.algtool_instance()->name() << endl;
  // Properties.
  lhs << "Properties: [" << endl;;
  for ( AlgToolHistory::PropertyList::const_iterator
        ipprop=rhs.properties().begin();
        ipprop!=rhs.properties().end(); ++ipprop ) {
    const Property& prop = **ipprop;
    prop.fillStream(lhs);
    lhs << endl;
  }
  lhs << "]" << endl;
  return lhs;
}
