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
class MyAudTool : public extends<AlgTool,
                                 IMyAudTool> {
public:

  /// Standard Constructor
  MyAudTool(const std::string& type,
            const std::string& name,
            const IInterface* parent);

  /// IMyAudTool interface
  virtual const std::string&  message() const;
  virtual void  doErr();
  virtual void  doFatal();
  /// Overriding initialize and finalize
  virtual StatusCode initialize();
  virtual StatusCode finalize();

protected:
  /// Standard destructor
   virtual ~MyAudTool( );
private:
  /// Properties
 };
#endif // GAUDIEXANMPLES_MYAUDTOOL_H
