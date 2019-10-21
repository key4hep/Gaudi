#ifndef GAUDIEXAMPLE_PARENTALG_H
#define GAUDIEXAMPLE_PARENTALG_H 1

// Include files
#include <Gaudi/Sequence.h>

/** @class ParentAlg
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class ParentAlg : public Gaudi::Sequence {
public:
  using Gaudi::Sequence::Sequence;

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute( const EventContext& ctx ) const override;
  StatusCode finalize() override;

private:
  Gaudi::Algorithm* m_subalg1{nullptr};
  Gaudi::Algorithm* m_subalg2{nullptr};
};

#endif // GAUDIEXAMPLE_PARENTALG_H
