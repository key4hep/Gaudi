#ifndef DETDESC_ISOLIDFACTORY_H
#define DETDESC_ISOLIDFACTORY_H 1
// GaudiKernel
#include "GaudiKernel/IFactory.h"
// forward declarations
class ISolid;   // DetDesc

/** @class ISolidFactory ISolidFactory.h DetDesc/ISolidFactory.h
 *
 *  An abstract interface to abstract factory to instantiate
 *  solids
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   02/08/2001
 */

class GAUDI_API ISolidFactory: public virtual IFactory {
public:
  /// InterfaceID
  DeclareInterfaceID(ISolidFactory, 2, 0);

  /** create an instance of a concrete ISolid Object
   *  @return pointer to created ISolid Object
   */
  virtual ISolid* instantiate () const = 0;

};

/** @def DECLARE_SOLID
 *  useful macro for declaration of Solid Factories
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   6 May 2002
 */
#define DECLARE_SOLID(x) extern const ISolidFactory& x##Factory; \
                                                     x##Factory.addRef();


// ==========================================================================
#endif ///< DETDESC_ISOLIDFACTORY_H
// ==========================================================================
