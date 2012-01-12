// $Id: SolidException.h,v 1.5 2003-04-25 08:52:23 sponce Exp $
#ifndef        __DETDESC_SOLID_SOLIDEXCEPTION_H__
#define        __DETDESC_SOLID_SOLIDEXCEPTION_H__
/// GaudiKernle
#include "GaudiKernel/GaudiException.h" 
///
class ISolid;
///

class SolidException : public GaudiException 
{  
  ///
public:
  ///
  SolidException( const std::string&     message      , 
                  const ISolid*          solid  =  0  ); 
  ///
  SolidException( const std::string   &  message      ,
                  const GaudiException&  Exception    ,  
                  const ISolid*          solid  =  0  ); 
  ///
  virtual ~SolidException() throw();
  ///
  virtual std::ostream&   printOut( std::ostream& os = std::cerr ) const ;
  virtual MsgStream&      printOut( MsgStream&    os             ) const ;
  virtual GaudiException* clone   ()                               const ; 
  ///
 private:
  ///
  const ISolid*     m_se_solid         ;       // the "author" of the exception 
  ///
};


#endif    //   __DETDESC_SOLID_SOLIDEXCEPTION_H__

