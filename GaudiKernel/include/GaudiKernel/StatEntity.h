/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include <Gaudi/Accumulators.h>

/**
 * backward compatible StatEntity class. Should not be used.
 * Only here for backward compatibility
 */
class StatEntity : public Gaudi::Accumulators::PrintableCounter,
                   public Gaudi::Accumulators::AccumulatorSet<double, Gaudi::Accumulators::atomicity::full, double,
                                                              Gaudi::Accumulators::StatAccumulator,
                                                              Gaudi::Accumulators::BinomialAccumulator> {
public:
  inline static const std::string typeString{ "statentity" };
  using AccParent         = Gaudi::Accumulators::AccumulatorSet<double, Gaudi::Accumulators::atomicity::full, double,
                                                        Gaudi::Accumulators::StatAccumulator,
                                                        Gaudi::Accumulators::BinomialAccumulator>;
  using BinomialAccParent = Gaudi::Accumulators::BinomialAccumulator<Gaudi::Accumulators::atomicity::full, double>;
  using Gaudi::Accumulators::StatAccumulator<Gaudi::Accumulators::atomicity::full, double>::nEntries;
  using AccParent::reset;
  /// the constructor with automatic registration in the owner's counter map
  StatEntity() = default;
  StatEntity( const unsigned long entries, const double flag, const double flag2, const double minFlag,
              const double maxFlag ) {
    reset( std::make_tuple(
        std::make_tuple( std::make_tuple( std::make_tuple( entries, flag ), flag2 ), minFlag, maxFlag ),
        std::make_tuple( 0, 0 ) ) );
  }
  void reset() { AccParent::reset(); }
  void operator=( double by ) {
    this->reset();
    ( *this ) += by;
  }
  StatEntity& operator-=( double by ) {
    ( *this ) += ( -by );
    return *this;
  }
  StatEntity& operator++() {
    ( *this ) += 1.0;
    return *this;
  }
  StatEntity operator++( int ) {
    auto copy = *this;
    ++( *this );
    return copy;
  }
  StatEntity& operator--() {
    ( *this ) += -1.0;
    return *this;
  }
  StatEntity operator--( int ) {
    auto copy = *this;
    --( *this );
    return copy;
  }
  bool operator<( const StatEntity& se ) const {
    return std::make_tuple( nEntries(), sum(), min(), max(), sum2() ) <
           std::make_tuple( se.nEntries(), se.sum(), se.min(), se.max(), se.sum2() );
  };
  // using AccumulatorSet::operator+=;
  StatEntity& operator+=( double by ) {
    this->AccumulatorSet::operator+=( by );
    return *this;
  }
  StatEntity& operator+=( StatEntity by ) {
    mergeAndReset( std::move( by ) );
    return *this;
  }
  unsigned long add( const double v ) {
    *this += v;
    return nEntries();
  }
  unsigned long addFlag( const double v ) { return add( v ); }
  // aliases (a'la ROOT)
  double Sum() const { return sum(); }                // get sum
  double Mean() const { return mean(); }              // get mean
  double MeanErr() const { return meanErr(); }        // get error in mean
  double rms() const { return standard_deviation(); } // get rms
  double Rms() const { return standard_deviation(); } // get rms
  double RMS() const { return standard_deviation(); } // get rms
  double Eff() const { return eff(); }                // get efficiency
  double Min() const { return min(); }                // get minimal value
  double Max() const { return max(); }                // get maximal value
  // some legacy methods, to be removed ...
  double      flag() const { return sum(); }
  double      flag2() const { return sum2(); }
  double      flagMean() const { return mean(); }
  double      flagRMS() const { return standard_deviation(); }
  double      flagMeanErr() const { return meanErr(); }
  double      flagMin() const { return min(); }
  double      flagMax() const { return max(); }
  static bool effCounter( std::string_view name ) {
    using boost::algorithm::icontains;
    return icontains( name, "eff" ) || icontains( name, "acc" ) || icontains( name, "filt" ) ||
           icontains( name, "fltr" ) || icontains( name, "pass" );
  }
  template <typename stream>
  stream& printFormattedImpl( stream& o, const std::string& format ) const {
    boost::format fmt{ format };
    fmt % nEntries() % sum() % mean() % standard_deviation() % min() % max();
    return o << fmt.str();
  }
  std::ostream& printFormatted( std::ostream& o, const std::string& format ) const {
    return printFormattedImpl( o, format );
  }
  MsgStream& printFormatted( MsgStream& o, const std::string& format ) const { return printFormattedImpl( o, format ); }
  using Gaudi::Accumulators::PrintableCounter::print;
  template <typename stream>
  stream& printImpl( stream& o, bool tableFormat, std::string_view name, bool flag, std::string_view fmtHead ) const {
    if ( flag && effCounter( name ) && 0 <= eff() && 0 <= effErr() && sum() <= nEntries() &&
         ( 0 == min() || 1 == min() ) && ( 0 == max() || 1 == max() ) ) {
      // efficiency printing
      if ( tableFormat ) {
        if ( name.empty() ) {
          constexpr auto fmt = "|%|10d| |%|11.5g| |(%|#9.7g| +- %|-#8.7g|)%%|   -------   |   -------   |";
          return o << boost::format{ fmt } % BinomialAccParent::nEntries() % sum() % ( efficiency() * 100 ) %
                          ( efficiencyErr() * 100 );
        } else {
          auto fmt = std::string{ " |*" }.append( fmtHead ).append(
              "|%|10d| |%|11.5g| |(%|#9.7g| +- %|-#8.7g|)%%|   -------   |   -------   |" );
          return o << boost::format{ fmt } % ( std::string{ "\"" }.append( name ).append( "\"" ) ) %
                          BinomialAccParent::nEntries() % sum() % ( efficiency() * 100 ) % ( efficiencyErr() * 100 );
        }
      } else {
        constexpr auto fmt = "#=%|-7lu| Sum=%|-11.5g| Eff=|(%|#9.7g| +- %|-#8.6g|)%%|";
        return o << boost::format{ fmt } % BinomialAccParent::nEntries() % sum() % ( efficiency() * 100 ) %
                        ( efficiencyErr() * 100 );
      }
    } else {
      // Standard printing
      if ( tableFormat ) {
        if ( name.empty() ) {
          constexpr auto fmt = "|%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |";
          return o << boost::format{ fmt } % nEntries() % sum() % mean() % standard_deviation() % min() % max();

        } else {
          auto fmt = std::string{ " | " }.append( fmtHead ).append(
              "|%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |" );
          return o << boost::format{ fmt } % std::string{ "\"" }.append( name ).append( "\"" ) % nEntries() % sum() %
                          mean() % standard_deviation() % min() % max();
        }
      } else {
        constexpr auto fmt = "#=%|-7lu| Sum=%|-11.5g| Mean=%|#10.4g| +- %|-#10.5g| Min/Max=%|#10.4g|/%|-#10.4g|";
        return o << boost::format{ fmt } % nEntries() % sum() % mean() % standard_deviation() % min() % max();
      }
    }
  }
  std::ostream& print( std::ostream& o, bool tableFormat, std::string_view name, bool flag = true,
                       std::string_view fmtHead = "%|-48.48s|%|27t|" ) const {
    return printImpl( o, tableFormat, name, flag, fmtHead );
  }
  MsgStream& print( MsgStream& o, bool tableFormat, std::string_view name, bool flag = true,
                    std::string_view fmtHead = "%|-48.48s|%|27t|" ) const {
    return printImpl( o, tableFormat, name, flag, fmtHead );
  }
  virtual std::ostream& print( std::ostream& o, std::string_view tag ) const override {
    return print( o, true, tag, true );
  }
  virtual MsgStream& print( MsgStream& o, std::string_view tag ) const override { return print( o, true, tag, true ); }
  std::ostream&      print( std::ostream& o, bool tableFormat = false ) const override {
    std::string emptyName;
    return print( o, tableFormat, emptyName, true );
  }
  MsgStream& print( MsgStream& o, bool tableFormat = false ) const override {
    std::string emptyName;
    return print( o, tableFormat, emptyName, true );
  }
  std::string toString() const {
    std::ostringstream ost;
    print( ost );
    return ost.str();
  }
  std::ostream& fillStream( std::ostream& o ) const { return print( o ); }
  MsgStream&    fillStream( MsgStream& o ) const { return print( o ); }
  /// Basic JSON export for Gaudi::Monitoring::Hub support.
  virtual nlohmann::json toJSON() const override {
    return { { "type", typeString },
             { "empty", this->nEntries() == 0 },
             { "nEntries", this->nEntries() },
             { "sum", this->sum() },
             { "mean", this->mean() },
             { "sum2", this->sum2() },
             { "standard_deviation", this->standard_deviation() },
             { "min", this->min() },
             { "max", this->max() },
             { "nTrueEntries", this->nTrueEntries() },
             { "nFalseEntries", this->nFalseEntries() },
             { "efficiency", this->efficiency() },
             { "efficiencyErr", this->efficiencyErr() } };
  }
  static StatEntity fromJSON( const nlohmann::json& j ) {
    StatEntity res;
    res.reset( AccParent::extractJSONData(
        j, { { { { "nEntries", "sum" }, "sum2" }, "min", "max" }, { "nTrueEntries", "nFalseEntries" } } ) );
    return res;
  }
};
