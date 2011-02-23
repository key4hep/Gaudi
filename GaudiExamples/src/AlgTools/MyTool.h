// $Id: MyTool.h,v 1.2 2001/12/10 22:43:04 mato Exp $
#ifndef GAUDIEXANMPLES_MYTOOL_H
#define GAUDIEXANMPLES_MYTOOL_H 1

// Include files
#include "GaudiKernel/AlgTool.h"
#include "IMyTool.h"

/** @class MyTool MyTool.h
 *  This is an interface class for a example tool
 *
 *  @author Pere Mato
 *  @date   14/10/2001
 */
class MyTool : public extends1<AlgTool, IMyTool> {
public:

  /// Standard Constructor
  MyTool(const std::string& type,
                const std::string& name,
                const IInterface* parent);

  /// IMyTool interface
  virtual const std::string&  message() const;
  virtual void  doIt();
  /// Overriding initialize and finalize
  virtual StatusCode initialize();
  virtual StatusCode finalize();

protected:
  /// Standard destructor
   virtual ~MyTool( );
private:
  /// Properties
  int          m_int;
  double       m_double;
  std::string  m_string;
  bool         m_bool;
 };
#endif // GAUDIEXANMPLES_MYTOOL_H
