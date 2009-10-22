// $Id: IAlgManager.h,v 1.7 2008/06/02 14:20:38 marcocle Exp $
#ifndef GAUDIKERNEL_IALGMANAGER_H
#define GAUDIKERNEL_IALGMANAGER_H

// Include files
#include "GaudiKernel/IComponentManager.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/TypeNameString.h"
#include <string>
#include <list>

// Forward class declaration
class IService;
class IAlgorithm;
class ISvcLocator;

/** @class IAlgManager IAlgManager.h GaudiKernel/IAlgManager.h

    The IAlgManager is the interface implemented by the Algorithm Factory in the
    Application Manager to support management functions. Clients use this
    interface to declare abstract algorithm factories, and to create concrete
    instances of algorithms. There are currently 3 methods for the declaration:
    static creator method (creator), an abstract factory (factory) or a sharable
    library (module).

    @author Pere Mato
*/
class GAUDI_API IAlgManager: virtual public IComponentManager {
public:
  /// InterfaceID
  DeclareInterfaceID(IAlgManager,5,0);

  /// Add an algorithm to the list of known algorithms
  virtual StatusCode addAlgorithm( IAlgorithm* alg     // Pointer to the Algorithm
                                 ) = 0;
  /// Remove an algorithm from the list of known algorithms
  virtual StatusCode removeAlgorithm( IAlgorithm* alg  // Pointer to the Algorithm
                                 ) = 0;
  /// Create an instance of a algorithm type that has been declared beforehand and assigns to it a name.
  /// It returns a pointer to an IAlgorithm.
  virtual StatusCode createAlgorithm( const std::string& algtype,  // Algorithm type name
                                      const std::string& algname,  // Algorithm name to be assigned
                                      IAlgorithm*& alg,            // Returned algorithm
                                      bool managed = false         // Flag to indicate if the algorithm is managed
                                    ) = 0;
#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
  /// Find an algorithm with given name in the list of known algorithms
  virtual StatusCode getAlgorithm( const std::string& name,        // Algorithm name to be searched
                                   IAlgorithm*& alg                // Returned algorithm
                                 ) const {
    SmartIF<IAlgorithm> &si = const_cast<IAlgManager*>(this)->algorithm(name, false);
    alg = si.get();
    return si.isValid() ? StatusCode::SUCCESS : StatusCode::FAILURE;
  }
#endif
  /// Check the existence of an algorithm with a given name in the list of known algorithms
  virtual bool existsAlgorithm( const std::string& name            // Algorithm name to be searched
                              ) const = 0;
  /// Return the list of Algorithms
  virtual const std::list<IAlgorithm*>& getAlgorithms() const = 0;

#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
  /// Initializes the list of "managed" algorithms
  virtual StatusCode initializeAlgorithms() { return initialize(); }

  /// Starts the list of "managed" algorithms
  virtual StatusCode startAlgorithms() { return start(); }

  /// Stops the list of "managed" algorithms
  virtual StatusCode stopAlgorithms() { return stop(); }

  /// Finalizes the list of "managed" algorithms
  virtual StatusCode finalizeAlgorithms() { return finalize(); }

  /// Initializes the list of "managed" algorithms
  virtual StatusCode reinitializeAlgorithms() { return reinitialize(); }

  /// Starts the list of "managed" algorithms
  virtual StatusCode restartAlgorithms() { return restart(); }
#endif

  /// Returns a smart pointer to a service.
  virtual SmartIF<IAlgorithm> &algorithm(const Gaudi::Utils::TypeNameString &typeName, const bool createIf = true) = 0;

  /// Returns a smart pointer to the requested interface of a service.
  template <typename T>
  inline SmartIF<T> algorithm(const Gaudi::Utils::TypeNameString &typeName, const bool createIf = true) {
    return SmartIF<T>(algorithm(typeName, createIf));
  }

};


#endif  // GAUDI_IALGMANAGER_H
