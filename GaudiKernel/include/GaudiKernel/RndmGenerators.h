/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/IRndmGen.h>
#include <GaudiKernel/SmartIF.h>
#include <vector>

class IRndmGen;
class IRndmGenSvc;

namespace Rndm {

  template <class TYPE>
  class Generator;

  /** Parameters for the Gauss random number generation
   */
  class GAUDI_API Gauss : public IRndmGen::Param {
  protected:
    /// Generator is the friend
    friend class Generator<Gauss>;
    /// Mean of the Gauss distribution
    double m_mean;
    /// Sigma of the Gauss distribution
    double m_sigma;

  public:
    /// Standard Constructor
    Gauss( double m, double s ) : IRndmGen::Param( IID_IRndmGauss ), m_mean( m ), m_sigma( s ) {}
    /// Access mean value of the distribution
    double mean() const { return m_mean; }
    /// Access width of the distribution
    double sigma() const { return m_sigma; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmGauss; }
    /// Clone parameters
    Gauss* clone() const override { return new Gauss( m_mean, m_sigma ); }
  };

  /** Parameters for the Gauss random number generation
   */
  class GAUDI_API Exponential : public IRndmGen::Param {
  protected:
    /// Mean value of the exponential distribution
    double m_mean;

  public:
    /// Standard Constructor
    Exponential( double m ) : IRndmGen::Param( IID_IRndmExponential ), m_mean( m ) {}
    /// Access mean value of the distribution
    double mean() const { return m_mean; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmExponential; }
    /// Clone parameters
    Exponential* clone() const override { return new Exponential( m_mean ); }
  };

  /** Parameters for the Chi2 distributed random number generation
   */
  class GAUDI_API Chi2 : public IRndmGen::Param {
    friend class Generator<Chi2>;

  protected:
    /// Number of degrees of freedom
    long m_nDOF;

  public:
    /// Standard Constructor
    Chi2( long n_dof ) : IRndmGen::Param( IID_IRndmChi2 ), m_nDOF( n_dof ) {}
    /// Access mean value of the distribution
    long nDOF() const { return m_nDOF; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmChi2; }
    /// Clone parameters
    Chi2* clone() const override { return new Chi2( m_nDOF ); }
  };

  /** Parameters for the BreitWigner distributed random number generation
   */
  class GAUDI_API BreitWigner : public IRndmGen::Param {
    friend class Generator<BreitWigner>;

  protected:
    /// Mean and Gamma parameter of the Breit-Wigner distribution
    double m_mean, m_gamma;

  public:
    /// Standard Constructor
    BreitWigner( double m, double g ) : IRndmGen::Param( IID_IRndmBreitWigner ), m_mean( m ), m_gamma( g ) {}
    /// Access mean value of the distribution
    double mean() const { return m_mean; }
    /// Access width of the distribution
    double gamma() const { return m_gamma; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmBreitWigner; }
    /// Clone parameters
    BreitWigner* clone() const override { return new BreitWigner( m_mean, m_gamma ); }
  };

  /** Parameters for the Landau distributed random number generation
   */
  class GAUDI_API Landau : public IRndmGen::Param {
    friend class Generator<Landau>;

  protected:
    /// Mean and Gamma parameter of the Breit-Wigner distribution
    double m_mean, m_sigma;

  public:
    /// Standard Constructor
    Landau( double m, double s ) : IRndmGen::Param( IID_IRndmLandau ), m_mean( m ), m_sigma( s ) {}
    /// Access mean value of the distribution
    double mean() const { return m_mean; }
    /// Access width of the distribution
    double sigma() const { return m_sigma; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmLandau; }
    /// Clone parameters
    Landau* clone() const override { return new Landau( m_mean, m_sigma ); }
  };

  /** Parameters for the BreitWigner distributed random number generation
      with cut off;
  */
  class GAUDI_API BreitWignerCutOff : public IRndmGen::Param {
    friend class Generator<BreitWignerCutOff>;

  protected:
    /// Mean, Gamma and cut off parameter of the Breit-Wigner distribution
    double m_mean, m_gamma, m_cut;

  public:
    /// Standard Constructor
    BreitWignerCutOff( double m, double g, double c )
        : IRndmGen::Param( IID_IRndmBreitWignerCutOff ), m_mean( m ), m_gamma( g ), m_cut( c ) {}
    /// Access mean value of the distribution
    double mean() const { return m_mean; }
    /// Access width of the distribution
    double gamma() const { return m_gamma; }
    /// Access width of the distribution
    double cutOff() const { return m_cut; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmBreitWignerCutOff; }
    /// Clone parameters
    BreitWignerCutOff* clone() const override { return new BreitWignerCutOff( m_mean, m_gamma, m_cut ); }
  };

  /** Parameters for the StudentT distributed random number generation
   */
  class GAUDI_API StudentT : public IRndmGen::Param {
    friend class Generator<StudentT>;

  protected:
    /// StudentT distribution parameter
    double m_aValue;

  public:
    /// Standard Constructor
    StudentT( double a ) : IRndmGen::Param( IID_IRndmStudentT ), m_aValue( a ) {}
    /// Access A parameter
    double aValue() const { return m_aValue; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmStudentT; }
    /// Clone parameters
    StudentT* clone() const override { return new StudentT( m_aValue ); }
  };

  /** Parameters for the Gamma distributed  random number generation
   */
  class GAUDI_API Gamma : public IRndmGen::Param {
    friend class Generator<Gamma>;

  protected:
    /// k Value
    double m_kValue;
    /// Lambda parameter
    double m_lambda;

  public:
    /// Standard Constructor
    Gamma( double k, double l ) : IRndmGen::Param( IID_IRndmGamma ), m_kValue( k ), m_lambda( l ) {}
    /// Access K parameter
    double kValue() const { return m_kValue; }
    /// Access Lambda parameter
    double lambda() const { return m_lambda; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmGamma; }
    /// Clone parameters
    Gamma* clone() const override { return new Gamma( m_kValue, m_lambda ); }
  };

  /** Parameters for the Poisson distributed random number generation with
   *  a given mean.
   */
  class GAUDI_API Poisson : public IRndmGen::Param {
    friend class Generator<Poisson>;

  protected:
    /// Mean value of the Poisson distribution
    double m_mean;

  public:
    /// Standard Constructor
    Poisson( double m ) : IRndmGen::Param( IID_IRndmPoisson ), m_mean( m ) {}
    /// Access mean value of the distribution
    double mean() const { return m_mean; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmPoisson; }
    /// Clone parameters
    Poisson* clone() const override { return new Poisson( m_mean ); }
  };

  /** Parameters for the Binomial distributed random number generation.
      The returned values are in fact integers
  */
  class GAUDI_API Binomial : public IRndmGen::Param {
  protected:
    /// Number of events the binomial destribution corresponds to
    long m_nEvent;
    /// And the probability for having success
    double m_probability;

  public:
    /// Standard Constructor
    Binomial( long n, double p ) : IRndmGen::Param( IID_IRndmBinomial ), m_nEvent( n ), m_probability( p ) {}
    /// Access number of events
    long nEvent() const { return m_nEvent; }
    /// Access number of events
    double probability() const { return m_probability; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmBinomial; }
    /// Clone parameters
    Binomial* clone() const override { return new Binomial( m_nEvent, m_probability ); }
  };

  /** Parameters for the flat random number generation within boundaries
   *  [minimum, maximum]
   */
  class GAUDI_API Flat : public IRndmGen::Param {
  protected:
    /// Lower boundary for random numbers
    double m_minimum;
    /// Upper boundary for random numbers
    double m_maximum;

  public:
    /// Standard Constructor
    Flat( double mi, double ma ) : IRndmGen::Param( IID_IRndmFlat ), m_minimum( mi ), m_maximum( ma ) {}
    /// Access lower edge
    double minimum() const { return m_minimum; }
    /// Access upper edge
    double maximum() const { return m_maximum; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmFlat; }
    /// Clone parameters
    Flat* clone() const override { return new Flat( m_minimum, m_maximum ); }
  };

  /** Parameters for the bit value generation: returns values 0 and 1
   */
  class GAUDI_API Bit : public IRndmGen::Param {
  public:
    /// Standard Constructor
    Bit() : IRndmGen::Param( IID_IRndmBit ) {}
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmBit; }
    /// Clone parameters
    Bit* clone() const override { return new Bit(); }
  };

  /** Generate a random number Generator following generally distributed random
      values, given a user-defined probability distribution function.

      The probability distribution function (Pdf) must be provided by the user
      as an array of positive real number. The array size must also be
      provided. The Pdf doesn't need to be normalized to 1.
      if IntType = 0 ( default value ) a uniform random number is
      generated. The uniform number is then transformed
      to the user's distribution using the cumulative probability
      distribution constructed from his histogram. The cumulative
      distribution is inverted using a binary search for the nearest
      bin boundary and a linear interpolation within the
      bin. Therefore a constant density within each bin is generated.
      if IntType = 1 no interpolation is performed and the result is a
      discrete distribution.
  */
  class GAUDI_API DefinedPdf : public IRndmGen::Param {
  protected:
    /// Vector containing probability distribution function
    std::vector<double> m_pdf;
    /// Interpolation type
    long m_interpolation;

  public:
    /// Standard Constructor
    DefinedPdf( const std::vector<double>& pdf, long intpol )
        : IRndmGen::Param( IID_IRndmDefinedPdf ), m_pdf( pdf ), m_interpolation( intpol ) {}
    /// Access pdf
    std::vector<double>& pdf() { return m_pdf; }
    /// Access interpolation type
    long interpolation() const { return m_interpolation; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmDefinedPdf; }
    /// Clone parameters
    DefinedPdf* clone() const override { return new DefinedPdf( m_pdf, m_interpolation ); }
  };

  /** Parameters for the Gaussian tail number generation
   */
  class GAUDI_API GaussianTail : public IRndmGen::Param {
  protected:
    /// Cut on the Gaussian tail distribution
    double m_cut;
    /// Sigma of the Gauss ditribution
    double m_sigma;

  public:
    /// Standard Constructor
    GaussianTail( double a, double s ) : IRndmGen::Param( IID_IRndmGaussianTail ), m_cut( a ), m_sigma( s ) {}
    /// Access cut value of the distribution
    double cut() const { return m_cut; }
    /// Access sigma of the distribution
    double sigma() const { return m_sigma; }
    /// Identifier for factory
    static const InterfaceID& typeID() { return IID_IRndmGaussianTail; }
    /// Clone parameters
    GaussianTail* clone() const override { return new GaussianTail( m_cut, m_sigma ); }
  };

  /** Random number accessor
      This small class encapsulates the use of the random number generator.
      The sole pupose of this class is to hide the usage of the interface and
      make the whole thing more user friendly. The object is usable
      directly after creation.

      The typical usage is:
      Rndm::Numbers numbers();
      if ( numbers.initialize(rndmGenSvc, Rndm::Gauss(0.5,0.2)).isSuccess() )	{
        for ( int i = 0; i < 10; i++ )  {
          value = numbers();
          ...
        }
      }
  */
  class GAUDI_API Numbers {
  protected:
    /// Pointer to random number generator
    SmartIF<IRndmGen> m_generator;

  public:
    /// Standard constructor
    Numbers() = default;
    /// Copy constructor
    Numbers( const Numbers& ) = default;
    /// Initializing constructor
    Numbers( const SmartIF<IRndmGenSvc>& svc, const IRndmGen::Param& par );
    /// Standard destructor
    virtual ~Numbers();
    /// Initialization
    virtual StatusCode initialize( const SmartIF<IRndmGenSvc>& svc, const IRndmGen::Param& par );
    /// Finalization
    virtual StatusCode finalize();
    /// Check if the number supply is possible
    operator bool() const { return m_generator; }
    /// Operator () for the use within STL
    double operator()() const { return this->shoot(); }
    /// Pop a new number from the buffer
    double pop() const { return this->shoot(); }
    /// Pop a new number from the buffer
    double shoot() const { return m_generator ? m_generator->shoot() : -1; }
    /// Pop a new number from the buffer
    StatusCode shootArray( std::vector<double>& array, long num, long start = 0 ) const {
      return m_generator ? m_generator->shootArray( array, num, start ) : StatusCode::FAILURE;
    }
  };
} // namespace Rndm
