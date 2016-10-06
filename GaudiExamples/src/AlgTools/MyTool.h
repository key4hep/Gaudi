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
class MyTool : public extends<AlgTool,
                              IMyTool> {
public:

  /// Standard Constructor
  MyTool(const std::string& type,
                const std::string& name,
                const IInterface* parent);

  /// IMyTool interface
  const std::string&  message() const override;
  void  doIt() override;
  /// Overriding initialize and finalize
  StatusCode initialize() override;
  StatusCode finalize() override;

protected:
  /// Standard destructor
   ~MyTool( ) override;
private:
  /// Properties
  int          m_int;
  double       m_double;
  std::string  m_string;
  bool         m_bool;
 };
#endif // GAUDIEXANMPLES_MYTOOL_H
