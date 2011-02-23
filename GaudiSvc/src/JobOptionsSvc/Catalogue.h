// $Id: Catalogue.h,v 1.5 2007/05/24 14:41:22 hmd Exp $
// ============================================================================
// CVS tag $Name:  $ 
// ============================================================================
#ifndef JOBOPTIONSSVC_CATALOGUE_H 
#define JOBOPTIONSSVC_CATALOGUE_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <string>
#include <vector>
#include <map>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// local
// ============================================================================
#include "Position.h"
// ============================================================================
namespace Gaudi 
{ 
  namespace Parsers
  {
    class PropertyEntry;

    /** @struct _NoCaseCmp_
     *  case-insensitive comparison criteria for strings 
     */
    struct _NoCaseCmp_ 
      : public std::binary_function<std::string,std::string,bool>
    {
      bool operator() ( const std::string& s1 , 
                        const std::string& s2 ) const ;                  
    };
    
    /** @class  Catalogue Catalogue.h
     *
     *  Catalogue of PropertyEntry
     *
     *  @author:  Alexander Mazurov
     *  @date: 2006-02-17
     */
    class Catalogue 
     {
    public:
      typedef std::map<std::string,std::vector<PropertyEntry> > CatalogueT;
       /** Find properties of object by object name
        *  @return StatusCode::SUCCESS if catalogue contains object properties
        *  @param objName Object name
        *  @param list Result list
        */
       StatusCode findProperties(const std::string& objName, 
         std::vector<PropertyEntry>& list) const;
     
        /** Find property in catalogue
        *  @return StatusCode::SUCCESS -  if catalogue contains property
        *  @param objName Object name
        *  @param propName Property name
        *  @param property 
        */
       StatusCode findProperty(const std::string& objName, 
          const std::string& propName, PropertyEntry& property) const;
       
       /** Add property to catalogue
        *  @param objName Name of object
        *  @param property Property
        */
       void addProperty(const std::string& objName, 
          const PropertyEntry& property);

       /** Add property to catalogue
        *  @param objName Name of object
        *  @param propName Property name
        *  @param Value as string
        */
       StatusCode addProperty(const std::string& objName, 
          const std::string& propName,const std::string& value);      
          
       /** Return all objects names and their properties info
        *  @return objects names and their properties info
        */
       CatalogueT catalogue() const;
       
       /// print the content of the catalogue to std::ostream
       std::ostream& fillStream ( std::ostream& out ) const ;
       
     private:
       typedef std::map<std::string,PropertyEntry,_NoCaseCmp_> PropertiesStoreT;
       typedef std::map<std::string,PropertiesStoreT> ObjectsStoreT;
       // Properties. Key of map is object name. Key of value map is property name
       ObjectsStoreT m_properties;
    };

    class PropertyEntry 
    {
    public:
      /// Constructor
      PropertyEntry(){}
     /** Constructor
      *  @param name Name of property
      *  @param value String value of property. 
      */
      PropertyEntry(const std::string& name, 
                            const std::string& value);    
     /** Creator
      *  @param name Name of property
      *  @param value Vector value of property
      */
      PropertyEntry(const std::string& name, 
                 const std::vector<std::string>& value);
                            
     /** Constructor
      *  @param name Name of property
      *  @param value String value of property. 
      *  @param pos Property position in file
      */
      PropertyEntry(const std::string& name, 
        const std::string& value,const Position& pos);
     /** Creator
      *  @param name Name of property
      *  @param value Vector value of property
      *  @param pos Property position in file
      */
      PropertyEntry(const std::string& name, 
                 const std::vector<std::string>& value,const Position& pos);
                            
     /** Name of property
      *  @return Name of property
      */
     const std::string& name(void) const {return m_name;};

     /// Set name
     void setName(const std::string& name){m_name=name;};   

     /** Value of property
      *  @return Value of property
      */
     std::string value(void) const;

     /// Property position
     Position position(void) const{return m_position;}  

    
    /** Add values to vector value
      *  @return StatusCode::SUCCESS if property value is vector 
      *           (and we can add values)
      *  @param values Values to add
      */
     StatusCode addValues(const std::vector<std::string>& values);
     
     /** Remove values from property value
      *  @return StatusCode::SUCCESS if property value is vector 
      *         (and we can remove values)
      *  @param values 
      *  @param count Count of removed items
      */
     StatusCode removeValues(const std::vector<std::string>& values, int& count);
     
     /** Clear property vector value
      *  @return StatusCode::SUCCESS if property value is vector 
      *          (and we can clear values)
      */
      StatusCode clear(void);
      
      /// remove environment variables 
      StatusCode removeEnv() ;

  private:
     std::string m_name;
     std::vector<std::string> m_value;
     Position m_position;
     bool m_isVector;
    };

  } // end of namespace Parsers
} // end of namespace Gaudi
// ============================================================================
/// printout operator 
// ============================================================================
std::ostream& operator<<( std::ostream& o ,
                          const Gaudi::Parsers::Catalogue& c ) ;
// ============================================================================

// ============================================================================
// The END 
// ============================================================================
#endif // JOBOPTIONSSVC_CATALOGUE_H
// ============================================================================
