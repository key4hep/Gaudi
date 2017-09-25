#ifndef GAUDIEXANMPLES_MYAUDTOOL_H
#define GAUDIEXANMPLES_MYAUDTOOL_H 1

// Include files
#include "GaudiKernel/AlgTool.h"
#include "IMyAudTool.h"

/** @class MyAudTool MyAudTool.h
 *  This is an interface class for a example tool
 *
 *  @author Pere Mato
 *  @date   14/10/2001
 */
class MyAudTool : public extends<AlgTool, IMyAudTool>
{
public:
  /// Standard Constructor
  MyAudTool( const std::string& type, const std::string& name, const IInterface* parent );

  /// IMyAudTool interface
  const std::string& message() const override;
  void doErr() override;
  void doFatal() override;
  /// Overriding initialize and finalize
  StatusCode initialize() override;
  StatusCode finalize() override;

protected:
  /// Standard destructor
  ~MyAudTool() override;

private:
  /// Properties
};
#endif // GAUDIEXANMPLES_MYAUDTOOL_H
