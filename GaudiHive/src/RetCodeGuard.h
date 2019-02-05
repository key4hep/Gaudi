#ifndef GAUDIHIVE_RETCODEGUARD_H
#define GAUDIHIVE_RETCODEGUARD_H

#include "GaudiKernel/AppReturnCode.h"
#include "GaudiKernel/IProperty.h"

/// Helper class to set the application return code in case of early exit
/// (e.g. exception).
class RetCodeGuard {
public:
  inline RetCodeGuard( const SmartIF<IProperty>& appmgr, int retcode ) : m_appmgr( appmgr ), m_retcode( retcode ) {}
  inline void ignore() { m_retcode = Gaudi::ReturnCode::Success; }
  inline ~RetCodeGuard() {
    if ( UNLIKELY( Gaudi::ReturnCode::Success != m_retcode ) ) { Gaudi::setAppReturnCode( m_appmgr, m_retcode ); }
  }

private:
  SmartIF<IProperty> m_appmgr;
  int                m_retcode;
};

#endif
