// $Id: Printer.h,v 1.2 2005/10/14 12:57:13 mato Exp $
// ============================================================================
#ifndef GAUDIPYTHON_PRINTER_H 
#define GAUDIPYTHON_PRINTER_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <string>
#include <sstream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/DataObject.h"
// ============================================================================

namespace GaudiPython 
{
  /** @struct Printer Printer.h GaudiPython/Printer.h
   *  
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2005-08-05
   */
  template <class TYPE>
  struct Printer 
  {
    static std::string print 
    ( const TYPE& object ) 
    {
      std::stringstream stream ;
      stream << object << std::endl ;
      return stream.str() ;
    } ;
  } ;
  template<> 
  struct Printer<ContainedObject>
  {
    static std::string print 
    ( const ContainedObject& object ) 
    {
      std::ostringstream stream ;
      object.fillStream( stream ) ;
      return stream.str() ;
    } ;
  } ;
  template<> 
  struct Printer<DataObject>
  {
    static std::string print ( const DataObject& type ) 
    {
      std::ostringstream stream ;
      type.fillStream( stream ) ;
      return stream.str() ;
    } ;
  };

}; // end of namespace GaudiPython 

#endif // GAUDIPYTHON_PRINTER_H
// ============================================================================
