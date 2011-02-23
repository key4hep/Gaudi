// $Id: RndmTypeInfos.cpp,v 1.1 2003/02/18 09:10:49 mato Exp $
// Include files
#include <typeinfo>

// Local include files
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/RndmGenerators.h"

#ifdef __ICC
// disable icc remark #177: declared but never referenced
#pragma warning(disable:177)
#endif

/**
 * This is needed to insure that the typeinfo symbols are defined
 * in the GaudiKernel library. However it is never used.
 * Such a definition insures that every other library linking with the
 * GaudiKernel library will share the same definition of these typeinfo
 * and hence will be able to do dynamic_casts into the corresponding types.
 *
 * @author Sebastien Ponce
 */
namespace {
  const std::type_info& Param = typeid(IRndmGen::Param);
  const std::type_info& GaussInfo = typeid(Rndm::Gauss);
  const std::type_info& ExponentialInfo = typeid(Rndm::Exponential);
  const std::type_info& Chi2Info = typeid(Rndm::Chi2);
  const std::type_info& BreitWignerInfo = typeid(Rndm::BreitWigner);
  const std::type_info& LandauInfo = typeid(Rndm::Landau);
  const std::type_info& BreitWignerCutOffInfo = typeid(Rndm::BreitWignerCutOff);
  const std::type_info& StudentTInfo = typeid(Rndm::StudentT);
  const std::type_info& GammaInfo = typeid(Rndm::Gamma);
  const std::type_info& PoissonInfo = typeid(Rndm::Poisson);
  const std::type_info& BinomialInfo = typeid(Rndm::Binomial);
  const std::type_info& FlatInfo = typeid(Rndm::Flat);
  const std::type_info& BitInfo = typeid(Rndm::Bit);
  const std::type_info& DefinedPdfInfo = typeid(Rndm::DefinedPdf);
  const std::type_info& GaussianTailInfo = typeid(Rndm::GaussianTail);
}
