// Include files 
#include <cstdlib>
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/GaudiException.h"
#include "AIDATuple.h"

// Instantiation of a static factory class used by clients to create
// empty instances of this class (needed for deserialization schema)
DECLARE_DATAOBJECT_FACTORY(AIDATuple)

// ===========================
// Constructors and destructor
// ===========================

//-------------------------------------------------------
AIDATuple::AIDATuple( pi_aida::Proxy_Store* store,
                      const std::string& storeObj,
                      const std::string& title,
                      const std::string& description)
  : pi_aida::Tuple( *store, storeObj, title, description)
//-------------------------------------------------------
{
  setTitle( title );
  //annotation().addItem;
}


//------------------------------------------------------------
AIDATuple::AIDATuple( const AIDATuple& tuple )
  :pi_aida::Tuple( static_cast<const pi_aida::Tuple&>(tuple) )
   , DataObject()
//------------------------------------------------------------
{
  setTitle( tuple.title() );
}


//------------------------------------------------------------
AIDATuple::AIDATuple( const AIDA::ITuple& tuple )
  :pi_aida::Tuple( static_cast<const AIDA::ITuple&>(tuple) )
   , DataObject()
//------------------------------------------------------------
{
  setTitle( tuple.title() );
}

/*
//------------------------------------------------------------
AIDATuple::AIDATuple( const REP& rep )
  :pi_aida::Tuple()
   , DataObject()
//------------------------------------------------------------
{
  newRep( rep );
  
  setTitle( rep.GetTitle() );
}
*/

//---------------------
AIDATuple::~AIDATuple()
//---------------------
{}
