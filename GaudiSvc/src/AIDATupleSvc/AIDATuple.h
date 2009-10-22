#ifndef AIDATUPLESVC_AIDATUPLE_H 
#define AIDATUPLESVC_AIDATUPLE_H 1

// Include files
#include <string>
#include "GaudiKernel/xtoa.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/DataObject.h"

#include "GaudiPI/AIDA_Proxy/Tuple.h"
#include "GaudiPI/AIDA_Annotation/AIDA_Annotation.h"

class AIDATuple : public pi_aida::Tuple, public DataObject 
{
public: 
  // Standard constructors
  AIDATuple() {}
  

  AIDATuple( pi_aida::Proxy_Store* store,
             const std::string& storeObj,
             const std::string& title,
             const std::string& description);

  // Copy constructor
  AIDATuple( const AIDATuple& tuple );
  
  // Copy constructor from AIDA interface
  AIDATuple( const AIDA::ITuple& tuple );
  
  // Copy constructor from representation
  //AIDATuple( const REP& rep );

  // Destructor
  ~AIDATuple();

  // Methods from AIDA:  

  // Get the Tuple's title
  virtual std::string title() const {
    return annotation().value( "title" );
  }
  
  
  // Set the Tuple's title
  virtual bool setTitle( const std::string & title ) {
    if ( !annotation().addItem( "title", title ) )
      annotation().setValue( "title", title );
    return true;
  }
    
  // Retrieve pointer to class definition structure
  virtual const CLID& clID() const { return AIDATuple::classID(); }
  static const CLID& classID()     { return CLID_AIDATuple; }

};
#endif // AIDATUPLESVC_AIDATUPLE_H
