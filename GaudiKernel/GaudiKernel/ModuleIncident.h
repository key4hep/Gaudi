#ifndef GAUDIKERNEL_MODULEINCIDENT_H
#define GAUDIKERNEL_MODULEINCIDENT_H

// Include files
#include "GaudiKernel/Incident.h"
#include <string>

/**
 * @class ModuleIncident
 * @brief base class for Module-related incident
 * @author P. Calafiura
 */
class ModuleIncident : public Incident {
protected:
  /// @name protected structors
  //@{
  ModuleIncident( std::string source, // Source(service or alg) name)
                  std::string type,   // Type (load, unload, ...)
                  std::string module  // module(DLL) in question
                  )
      : Incident( std::move( source ), std::move( type ) ), m_module( std::move( module ) ) {}
  virtual ~ModuleIncident() = default;
  //@}

public:
  /// @name Accessors
  //@{
  using Incident::source;
  using Incident::type;
  /// the model (DLL) being worked on
  const std::string& module() const { return m_module; }
  //@}
private:
  /// the model (DLL) being worked on
  std::string m_module;
};

/**
 * @class ModuleLoadedIncident
 * @brief fired when a module (DLL) is loaded
 * @author P. Calafiura
 */
class ModuleLoadedIncident : public ModuleIncident {
public:
  static std::string TYPE() { return "ModuleLoaded"; }
  ModuleLoadedIncident( std::string source, // Source(service or alg) name)
                        std::string module  // module(DLL) in question
                        )
      : ModuleIncident( std::move( source ), TYPE(), std::move( module ) ) {}
};

#endif // GAUDIKERNEL_MODULEINCIDENT_H
