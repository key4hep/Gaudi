// $Id: XmlCode.h,v 1.1.1.1 2007/01/15 13:56:38 hmd Exp $
#ifndef DATALISTENERSVC_XMLCODE_H 
#define DATALISTENERSVC_XMLCODE_H 1
#include <iostream>


// Include files

/** @class XmlCode XmlCode.h DataListenerSvc/XmlCode.h
 *  
 *  XmlCode contains methods to write out XML tags. It is used in conjunction with DataListenerSvc to create the XML log files
 *  which can be subsequently later parsed. Indent levels are specified manually by the user who must make sure everything is
 *  consistent. 
 *  
 *  @author Ben King
 *  @date   2006-07-06
 */
//template <class Type> class XmlStream {

  
class XmlCode {
public: 
  
  /// Standard constructor
  XmlCode() {};

  /// Destructor
  virtual ~XmlCode() {};

/** returns an opening XML tag
 *  @param data Name of the tag
 *  @param level Amount of indenting to print, i.e. what generation of tag
 *  @param attribute Attribute to include in tag (if used)
 *  @param val Value of the Attribute (if used)
 *  with this class. 
 */

  /// Return an opening XML tag which takes "attribute:value" pair
  std::string tagBegin(std::string data, int level, std::string attribute, std::string val)
  {
    std::string indent="";
    
    for (int i=0; i<level; i++){
      indent = indent + "  ";
    }
    
    return "\n" + indent + "<" + data + " " + attribute + "=\"" + val + "\">"; // A start tag will start a line
  }

  /// return an opening XML tag 
  std::string tagBegin(std::string data, int level)
  {
    std::string indent="";
    
    for (int i=0; i<level; i++){
      indent = indent + "  ";
    }
    
    return "\n" + indent + "<" + data + ">"; // A start tag will start a line
  }

  /// return a closing XML tag
  /**
   * Will return a new line if the tag is given any indent, i.e. it is a parent tag
   */
  std::string tagEnd(std::string data, int level)
  {

    if (level != 0) {
      std::string indent="";
    
      for (int i=0; i<level; i++){
        indent = indent + "  ";
      }

      return "\n" + indent + "</" + data + ">\n";
    }
    else {
      return "</" + data + ">"; // An end tag will end a line
    }

  }
    
  /// Return just the data given to the function
  std::string data(std::string data)
  {
    return data; // Data will come after a string so no new line
  }


  // XML declaration
  /** XML declaration
   *
   *  @param vers XML version number
   *  @param cod XML encoding
   *  @param stand XML standalone
   *
   */
  std::string declaration(std::string vers, std::string cod, std::string stand)
  {
    return "<?xml version=\"" + vers + "\" encoding=\"" + cod + "\" standalone=\"" + stand + "\"?>";
  }
  
    
    
  

protected:

private:

  std::string tag;
  

};


#endif // DATALISTENERSVC_XMLCODE_H
