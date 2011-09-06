/**********************************************************
 * $Workfile: $
 * $Revision: 1.2 $
 * $Date: 2006/06/06 16:16:23 $
 * $Author: hmd $
 * $Archive: $
 *
 * Component Comment
 * Copyright Notice
 **********************************************************/

#ifndef INCLUDED_PROPERTYCALLBACKFUNCTOR_H
#define INCLUDED_PROPERTYCALLBACKFUNCTOR_H

#include "GaudiKernel/Kernel.h"

/****************************
 * C++ DECLARATION SECTION
 *   Base Classes Sub-section
 ****************************/

/* Standard C++ Class Headers */

/* Application C++ Class Headers */

/*************************************
 * C++ DECLARATION SECTION
 *   Collaborating Classes Sub-section
 *************************************/

/* Standard C++ Class Declarations */

/* Application C++ Class Declarations */
class Property;


/*******************************
 * CLASS DECLARATION SECTION
 *   Class Interface Sub-section
 *******************************/

class GAUDI_API PropertyCallbackFunctor {

////////////////////
/* Static Members */
////////////////////

public:
// Data and Function Members for Collaborators.
    // Data members
    static const PropertyCallbackFunctor* const nullPropertyCallbackFunctorPointer;
//////////////////////
/* Instance Members */
//////////////////////

public:

  // Data and Function Members for Collaborators.

  // Constructors - None, abstract base class
  // Destructor
  virtual ~PropertyCallbackFunctor() {}

  // Operators
  virtual void operator() ( Property& ) const = 0;

  virtual PropertyCallbackFunctor* clone() const = 0 ;

};

class PropertyCallbackPointerFunctor : public PropertyCallbackFunctor {

////////////////////
/* Static Members */
////////////////////

public:
// Data and Function Members for Collaborators.

    // Typedefs, consts, and enums
    typedef void ( * PtrToCallbackFunction ) ( Property& );

//////////////////////
/* Instance Members */
//////////////////////

public:
  // Data and Function Members for Collaborators.

  // Constructors
  explicit PropertyCallbackPointerFunctor( PtrToCallbackFunction pCF )
    : m_pCF( pCF ) { }
  // Destructor - Compiler generated version will be Ok

  // Operators
  virtual void operator() ( Property& prop ) const
  { m_pCF( prop ); }

  virtual PropertyCallbackPointerFunctor* clone() const
  { return new PropertyCallbackPointerFunctor(*this); }

private:
// Data and Function Members for This Class Implementation.

    // Data members
    PtrToCallbackFunction       m_pCF;

};


template< class T >
class PropertyCallbackMemberFunctor : public PropertyCallbackFunctor {

////////////////////
/* Static Members */
////////////////////

public:
// Data and Function Members for Collaborators.

    // Typedefs, consts, and enums
    typedef void ( T::* PtrToCallbackMember ) ( Property& );

//////////////////////
/* Instance Members */
//////////////////////

public:
  // Data and Function Members for Collaborators.

  // Constructors
  explicit PropertyCallbackMemberFunctor( PtrToCallbackMember pCM, T* instance )
    : m_pCM( pCM ), m_instance( instance ) { }
  // Destructor - Compiler generated version will be Ok

  // Operators
  virtual void operator() ( Property& prop ) const
  { ( m_instance->*m_pCM )( prop ); }

  virtual  PropertyCallbackMemberFunctor* clone() const
  { return new PropertyCallbackMemberFunctor(*this); }

private:
// Data and Function Members for This Class Implementation.

    // Data members
    PtrToCallbackMember m_pCM;
    T*                  m_instance;

};

#endif  // INCLUDED_PROPERTYCALLBACKFUNCTOR_H
