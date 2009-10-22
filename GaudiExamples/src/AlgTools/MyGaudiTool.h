// $Id: MyGaudiTool.h,v 1.3 2008/10/10 15:18:56 marcocle Exp $
#ifndef GAUDIEXANMPLES_MYGAUDITOOL_H
#define GAUDIEXANMPLES_MYGAUDITOOL_H 1

// Include files
#include "GaudiAlg/GaudiTool.h"
#include "IMyTool.h"

/** @class MyTool MyTool.h
 *  This is an interface class for a example tool
 *
 *  @author Pere Mato
 *  @date   14/10/2001
 */
class MyGaudiTool : public extends2<GaudiTool, IMyTool, IMyOtherTool> {
public:

  /// Standard Constructor
  MyGaudiTool(const std::string& type,
              const std::string& name,
              const IInterface* parent);

  // IMyTool interface
  virtual const std::string&  message() const;
  virtual void  doIt();

  // IMyOtherTool interface
  virtual void doItAgain();

  /// Overriding initialize and finalize
  virtual StatusCode initialize();
  virtual StatusCode finalize();

protected:
  /// Standard destructor
  virtual ~MyGaudiTool( );
private:
  /// Properties
  int          m_int;
  double       m_double;
  std::string  m_string;
  bool         m_bool;
};
#endif // GAUDIEXANMPLES_MYTOOL_H
