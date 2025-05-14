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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_HistoUtils

#include <Gaudi/Histograming/Sink/Utils.h>

#include <boost/test/unit_test.hpp>

namespace {
  // compare json objects containing floating point number safely (i.e. not with operator==)
  bool CompareJson( nlohmann::json const& lhs, nlohmann::json const& rhs ) {
    auto lhs_type = lhs.type();
    auto rhs_type = rhs.type();
    // all numbers are considered as floats
    if ( lhs_type == nlohmann::detail::value_t::number_integer ||
         lhs_type == nlohmann::detail::value_t::number_unsigned )
      lhs_type = nlohmann::detail::value_t::number_float;
    if ( rhs_type == nlohmann::detail::value_t::number_integer ||
         rhs_type == nlohmann::detail::value_t::number_unsigned )
      rhs_type = nlohmann::detail::value_t::number_float;
    if ( lhs_type == rhs_type ) {
      switch ( lhs_type ) {
      case nlohmann::detail::value_t::number_float: {
        // the main change compared to existing operator ==
        double a = lhs.get<double>();
        double b = rhs.get<double>();
        return std::abs( a - b ) <= std::min( std::abs( a ), std::abs( b ) ) * std::numeric_limits<double>::epsilon();
      }
      case nlohmann::detail::value_t::array:
        if ( lhs.size() != rhs.size() ) return false;
        // call recusrsively ourselves
        for ( unsigned int i = 0; i < lhs.size(); i++ ) {
          if ( !CompareJson( lhs[i], rhs[i] ) ) return false;
        }
        return true;
      case nlohmann::detail::value_t::object:
        // check verything in lhs is in rhs
        for ( auto it = lhs.cbegin(); it != lhs.cend(); ++it ) {
          if ( rhs.find( it.key() ) == rhs.end() ) return false;
          // call recusrsively ourselves
          if ( !CompareJson( it.value(), rhs[it.key()] ) ) return false;
        }
        // check rhs has no extra keys
        for ( auto it = rhs.cbegin(); it != rhs.cend(); ++it ) {
          if ( lhs.find( it.key() ) == lhs.end() ) return false;
        }
        return true;
      default:
        // fall back to default comparison for other types (integers, strings, ...)
        return lhs == rhs;
      }
    }
    return false;
  }
} // namespace

BOOST_AUTO_TEST_CASE( test_taxis_to_json ) {
  TAxis a{ 16, -4.5, 12.5 };
  a.SetTitle( "MyNiceAxis" );
  nlohmann::json j{ a };
  nlohmann::json expected{ { { "maxValue", 12.5 }, { "minValue", -4.5 }, { "nBins", 16 }, { "title", "MyNiceAxis" } } };
  BOOST_CHECK_MESSAGE( j == expected,
                       "Bad TAxis conversion to json. Got :\n" + j.dump() + "\nExpected : \n" + expected.dump() );
}

BOOST_AUTO_TEST_CASE( test_histo1D_to_json ) {
  TH1D h( "MyNiceName", "MyNiceTitle", 16, -4.5, 12.5 );
  h.Fill( -12.0 );
  for ( unsigned int i = 0; i < 5; i++ ) h.Fill( 0 );
  h.GetXaxis()->SetTitle( "MyNiceAxisTitle" );
  nlohmann::json j{ h };
  nlohmann::json expected{
      { { "type", "histogram:Histogram:double" },
        { "title", "MyNiceTitle" },
        { "dimension", 1 },
        { "empty", false },
        { "nEntries", 6 },
        { "nTotEntries", 5 },
        { "sum", 0 },
        { "mean", 0 },
        { "sum2", 0 },
        { "axis", { { { "maxValue", 12.5 }, { "minValue", -4.5 }, { "nBins", 16 }, { "title", "MyNiceAxisTitle" } } } },
        { "bins", { 1.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } } } };
  BOOST_CHECK_MESSAGE(
      CompareJson( j, expected ),
      "Bad TH1D conversion to json.\nDifference between result and expectation (json patch format):\n" +
          nlohmann::json::diff( j, expected ).dump() );
}

BOOST_AUTO_TEST_CASE( test_histo2D_to_json ) {
  TH2D h( "MyNiceName", "MyNiceTitle", 2, -1, 1, 2, 3, 5 );
  h.Fill( -12.0, -7.0 );
  for ( unsigned int i = 0; i < 5; i++ ) h.Fill( -0.5, 3.2 );
  h.GetXaxis()->SetTitle( "MyNiceAxisTitle" );
  nlohmann::json j{ h };
  nlohmann::json expected{
      { { "type", "histogram:Histogram:double" },
        { "title", "MyNiceTitle" },
        { "dimension", 2 },
        { "empty", false },
        { "nEntries", 6 },
        { "nTotEntries", 5 },
        { "sumx", -2.5 },
        { "sumx2", 1.25 },
        { "meanx", -0.5 },
        { "sumy", 16 },
        { "sumy2", 51.2 },
        { "sumxy", -8 },
        { "meany", 3.2 },
        { "axis",
          { { { "maxValue", 1.0 }, { "minValue", -1.0 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } },
            { { "maxValue", 5.0 }, { "minValue", 3.0 }, { "nBins", 2 }, { "title", "" } } } },
        { "bins", { 1.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } } } };
  BOOST_CHECK_MESSAGE(
      CompareJson( j, expected ),
      "Bad TH2D conversion to json.\nDifference between result and expectation (json patch format):\n" +
          nlohmann::json::diff( j, expected ).dump() );
}

BOOST_AUTO_TEST_CASE( test_histo3D_to_json ) {
  TH3D h( "MyNiceName", "MyNiceTitle", 2, -1, 1, 2, 3, 5, 1, -4, -2 );
  h.Fill( -12.0, -7.0, -8.0 );
  for ( unsigned int i = 0; i < 5; i++ ) h.Fill( -0.5, 3.2, -3.5 );
  h.GetXaxis()->SetTitle( "MyNiceAxisTitle" );
  nlohmann::json j{ h };
  nlohmann::json expected{
      { { "type", "histogram:Histogram:double" },
        { "title", "MyNiceTitle" },
        { "dimension", 3 },
        { "empty", false },
        { "nEntries", 6 },
        { "nTotEntries", 5 },
        { "sumx", -2.5 },
        { "sumx2", 1.25 },
        { "meanx", -0.5 },
        { "sumy", 16 },
        { "sumy2", 51.2 },
        { "sumxy", -8 },
        { "meany", 3.2 },
        { "sumz", -17.5 },
        { "sumz2", 61.25 },
        { "sumxz", 8.75 },
        { "sumyz", -56 },
        { "meanz", -3.5 },
        { "axis",
          { { { "maxValue", 1.0 }, { "minValue", -1.0 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } },
            { { "maxValue", 5.0 }, { "minValue", 3.0 }, { "nBins", 2 }, { "title", "" } },
            { { "maxValue", -2.0 }, { "minValue", -4.0 }, { "nBins", 1 }, { "title", "" } } } },
        { "bins", { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                    0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } } } };
  BOOST_CHECK_MESSAGE(
      CompareJson( j, expected ),
      "Bad TH3D conversion to json.\nDifference between result and expectation (json patch format):\n" +
          nlohmann::json::diff( j, expected ).dump() );
}

BOOST_AUTO_TEST_CASE( test_prof1D_to_json ) {
  TProfile h( "MyNiceName", "MyNiceTitle", 2, -0.5, 1.5 );
  h.Fill( -12.0, 2.0 );
  for ( unsigned int i = 0; i < 5; i++ ) h.Fill( 0.0, 3.0 );
  h.GetXaxis()->SetTitle( "MyNiceAxisTitle" );
  nlohmann::json j{ h };
  nlohmann::json expected{
      { { "type", "histogram:ProfileHistogram:double" },
        { "title", "MyNiceTitle" },
        { "dimension", 1 },
        { "empty", false },
        { "nEntries", 6 },
        { "axis", { { { "maxValue", 1.5 }, { "minValue", -0.5 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } } } },
        { "bins",
          { { { 1.0, 2.0 }, 4.0 }, { { 5.0, 15.0 }, 45.0 }, { { 0.0, 0.0 }, 0.0 }, { { 0.0, 0.0 }, 0.0 } } } } };
  BOOST_CHECK_MESSAGE(
      CompareJson( j, expected ),
      "Bad TProfile conversion to json.\nDifference between result and expectation (json patch format):\n" +
          nlohmann::json::diff( j, expected ).dump() + "\n" + j.dump() + "\n" + expected.dump() );
}

BOOST_AUTO_TEST_CASE( test_prof2D_to_json ) {
  TProfile2D h( "MyNiceName", "MyNiceTitle", 2, -0.5, 1.5, 2, 2.5, 4.5 );
  h.Fill( -12.0, -8.0, 2.0 );
  for ( unsigned int i = 0; i < 5; i++ ) h.Fill( 0.0, 3.0, 3.0 );
  h.GetXaxis()->SetTitle( "MyNiceAxisTitle" );
  nlohmann::json j{ h };
  nlohmann::json expected{
      { { "type", "histogram:ProfileHistogram:double" },
        { "title", "MyNiceTitle" },
        { "dimension", 2 },
        { "empty", false },
        { "nEntries", 6 },
        { "axis",
          { { { "maxValue", 1.5 }, { "minValue", -0.5 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } },
            { { "maxValue", 4.5 }, { "minValue", 2.5 }, { "nBins", 2 }, { "title", "" } } } },
        { "bins",
          { { { 1.0, 2.0 }, 4.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 5.0, 15.0 }, 45.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 },
            { { 0.0, 0.0 }, 0.0 } } } } };
  BOOST_CHECK_MESSAGE(
      CompareJson( j, expected ),
      "Bad TProfile2D conversion to json.\nDifference between result and expectation (json patch format):\n" +
          nlohmann::json::diff( j, expected ).dump() + "\n" + j.dump() + "\n" + expected.dump() );
}

namespace {
  void checkAxis( TAxis axis, int nbins, double min, double max, const std::string& title ) {
    BOOST_CHECK( axis.GetNbins() == nbins );
    BOOST_CHECK_CLOSE( axis.GetXmin(), min, 1.e-4 );
    BOOST_CHECK_CLOSE( axis.GetXmax(), max, 1.e-4 );
    BOOST_CHECK( axis.GetTitle() == title );
  }
} // namespace

using namespace Gaudi::Histograming::Sink;

BOOST_AUTO_TEST_CASE( test_json_to_taxis ) {
  nlohmann::json j{ { "maxValue", 12.5 }, { "minValue", -4.5 }, { "nBins", 16 }, { "title", "MyNiceAxis" } };
  auto           a = details::jsonToAxis( j );
  TAxis          axis( a.nBins, a.minValue, a.maxValue );
  checkAxis( axis, 16, -4.5, 12.5, "" );
}

BOOST_AUTO_TEST_CASE( test_json_to_histo1D ) {
  nlohmann::json j{
      { "type", "histogram:Histogram:double" },
      { "title", "MyNiceTitle" },
      { "dimension", 1 },
      { "empty", false },
      { "nEntries", 6 },
      { "axis", { { { "maxValue", 12.5 }, { "minValue", -4.5 }, { "nBins", 16 }, { "title", "MyNiceAxisTitle" } } } },
      { "bins", { 1.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } } };
  std::string dir{ "dir" };
  std::string name{ "subdir/name" };
  auto [histo, fullDir] = jsonToRootHistogram<Traits<false, TH1D, 1>>( dir, name, j );
  BOOST_CHECK( fullDir == "dir/subdir" );
  BOOST_CHECK( strcmp( histo.GetTitle(), "MyNiceTitle" ) == 0 );
  BOOST_CHECK( histo.GetDimension() == 1 );
  BOOST_CHECK_CLOSE( histo.GetEntries(), 6, 1.e-4 );
  checkAxis( *histo.GetXaxis(), 16, -4.5, 12.5, "MyNiceAxisTitle" );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinContent( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 5 ), 5.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 5 ), std::sqrt( 5.0 ), 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_json_to_histo2D ) {
  nlohmann::json j{ { "type", "histogram:Histogram:double" },
                    { "title", "MyNiceTitle" },
                    { "dimension", 2 },
                    { "empty", false },
                    { "nEntries", 6 },
                    { "axis",
                      { { { "maxValue", 1.0 }, { "minValue", -1.0 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } },
                        { { "maxValue", 5.0 }, { "minValue", 3.0 }, { "nBins", 2 }, { "title", "" } } } },
                    { "bins", { 1.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } } };
  std::string    dir{ "dir" };
  std::string    name{ "subdir/name" };
  auto [histo, fullDir] = jsonToRootHistogram<Traits<false, TH2D, 2>>( dir, name, j );
  BOOST_CHECK( fullDir == "dir/subdir" );
  BOOST_CHECK( strcmp( histo.GetTitle(), "MyNiceTitle" ) == 0 );
  BOOST_CHECK( histo.GetDimension() == 2 );
  BOOST_CHECK_CLOSE( histo.GetEntries(), 6, 1.e-4 );
  checkAxis( *histo.GetXaxis(), 2, -1.0, 1.0, "MyNiceAxisTitle" );
  checkAxis( *histo.GetYaxis(), 2, 3.0, 5.0, "" );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinContent( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 5 ), 5.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 5 ), std::sqrt( 5.0 ), 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_json_to_histo3D ) {
  nlohmann::json j{ { "type", "histogram:Histogram:double" },
                    { "title", "MyNiceTitle" },
                    { "dimension", 3 },
                    { "empty", false },
                    { "nEntries", 6 },
                    { "axis",
                      { { { "maxValue", 1.0 }, { "minValue", -1.0 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } },
                        { { "maxValue", 5.0 }, { "minValue", 3.0 }, { "nBins", 2 }, { "title", "" } },
                        { { "maxValue", -2.0 }, { "minValue", -4.0 }, { "nBins", 1 }, { "title", "" } } } },
                    { "bins", { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } } };
  std::string    dir{ "dir" };
  std::string    name{ "subdir/name" };
  auto [histo, fullDir] = jsonToRootHistogram<Traits<false, TH3D, 3>>( dir, name, j );
  BOOST_CHECK( fullDir == "dir/subdir" );
  BOOST_CHECK( strcmp( histo.GetTitle(), "MyNiceTitle" ) == 0 );
  BOOST_CHECK( histo.GetDimension() == 3 );
  BOOST_CHECK_CLOSE( histo.GetEntries(), 6, 1.e-4 );
  checkAxis( *histo.GetXaxis(), 2, -1.0, 1.0, "MyNiceAxisTitle" );
  checkAxis( *histo.GetYaxis(), 2, 3.0, 5.0, "" );
  checkAxis( *histo.GetZaxis(), 1, -4.0, -2.0, "" );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinContent( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 21 ), 5.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 21 ), std::sqrt( 5.0 ), 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_json_to_histo1D_root ) {
  nlohmann::json j{
      { "type", "histogram:Histogram:double" },
      { "title", "MyNiceTitle" },
      { "dimension", 1 },
      { "empty", false },
      { "nEntries", 6 },
      { "nTotEntries", 5 },
      { "sum", 4 },
      { "sum2", 10 },
      { "axis", { { { "maxValue", 12.5 }, { "minValue", -4.5 }, { "nBins", 16 }, { "title", "MyNiceAxisTitle" } } } },
      { "bins", { 1.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } } };
  std::string dir{ "dir" };
  std::string name{ "subdir/name" };
  auto [histo, fullDir] = jsonToRootHistogram<Traits<false, TH1D, 1>>( dir, name, j );
  BOOST_CHECK( fullDir == "dir/subdir" );
  BOOST_CHECK( strcmp( histo.GetTitle(), "MyNiceTitle" ) == 0 );
  BOOST_CHECK( histo.GetDimension() == 1 );
  BOOST_CHECK_CLOSE( histo.GetEntries(), 6, 1.e-4 );
  checkAxis( *histo.GetXaxis(), 16, -4.5, 12.5, "MyNiceAxisTitle" );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinContent( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 5 ), 5.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 5 ), std::sqrt( 5.0 ), 1.e-4 );
  double stats[13];
  histo.GetStats( stats );
  BOOST_CHECK_CLOSE( stats[0], 5, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[1], 5, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[2], 4, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[3], 10, 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_json_to_histo2D_root ) {
  nlohmann::json j{ { "type", "histogram:Histogram:double" },
                    { "title", "MyNiceTitle" },
                    { "dimension", 2 },
                    { "empty", false },
                    { "nEntries", 6 },
                    { "nTotEntries", 5 },
                    { "sumx", -2.5 },
                    { "sumx2", 1.25 },
                    { "sumy", 16 },
                    { "sumy2", 51.2 },
                    { "sumxy", -8 },
                    { "axis",
                      { { { "maxValue", 1.0 }, { "minValue", -1.0 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } },
                        { { "maxValue", 5.0 }, { "minValue", 3.0 }, { "nBins", 2 }, { "title", "" } } } },
                    { "bins", { 1.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } } };
  std::string    dir{ "dir" };
  std::string    name{ "subdir/name" };
  auto [histo, fullDir] = jsonToRootHistogram<Traits<false, TH2D, 2>>( dir, name, j );
  BOOST_CHECK( fullDir == "dir/subdir" );
  BOOST_CHECK( strcmp( histo.GetTitle(), "MyNiceTitle" ) == 0 );
  BOOST_CHECK( histo.GetDimension() == 2 );
  BOOST_CHECK_CLOSE( histo.GetEntries(), 6, 1.e-4 );
  checkAxis( *histo.GetXaxis(), 2, -1.0, 1.0, "MyNiceAxisTitle" );
  checkAxis( *histo.GetYaxis(), 2, 3.0, 5.0, "" );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinContent( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 5 ), 5.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 5 ), std::sqrt( 5.0 ), 1.e-4 );
  double stats[13];
  histo.GetStats( stats );
  BOOST_CHECK_CLOSE( stats[0], 5, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[1], 5, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[2], -2.5, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[3], 1.25, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[4], 16, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[5], 51.2, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[6], -8, 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_json_to_histo3D_root ) {
  nlohmann::json j{ { "type", "histogram:Histogram:double" },
                    { "title", "MyNiceTitle" },
                    { "dimension", 3 },
                    { "empty", false },
                    { "nEntries", 6 },
                    { "nTotEntries", 2 },
                    { "sumx", -2.5 },
                    { "sumx2", 1.25 },
                    { "sumy", 16 },
                    { "sumy2", 51.2 },
                    { "sumxy", -8 },
                    { "sumz", -17.5 },
                    { "sumz2", 61.25 },
                    { "sumxz", 8.75 },
                    { "sumyz", -56 },
                    { "axis",
                      { { { "maxValue", 1.0 }, { "minValue", -1.0 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } },
                        { { "maxValue", 5.0 }, { "minValue", 3.0 }, { "nBins", 2 }, { "title", "" } },
                        { { "maxValue", -2.0 }, { "minValue", -4.0 }, { "nBins", 1 }, { "title", "" } } } },
                    { "bins", { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } } };
  std::string    dir{ "dir" };
  std::string    name{ "subdir/name" };
  auto [histo, fullDir] = jsonToRootHistogram<Traits<false, TH3D, 3>>( dir, name, j );
  BOOST_CHECK( fullDir == "dir/subdir" );
  BOOST_CHECK( strcmp( histo.GetTitle(), "MyNiceTitle" ) == 0 );
  BOOST_CHECK( histo.GetDimension() == 3 );
  BOOST_CHECK_CLOSE( histo.GetEntries(), 6, 1.e-4 );
  checkAxis( *histo.GetXaxis(), 2, -1.0, 1.0, "MyNiceAxisTitle" );
  checkAxis( *histo.GetYaxis(), 2, 3.0, 5.0, "" );
  checkAxis( *histo.GetZaxis(), 1, -4.0, -2.0, "" );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinContent( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 21 ), 5.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 1 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 21 ), std::sqrt( 5.0 ), 1.e-4 );
  double stats[13];
  histo.GetStats( stats );
  BOOST_CHECK_CLOSE( stats[0], 2, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[1], 2, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[2], -2.5, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[3], 1.25, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[4], 16, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[5], 51.2, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[6], -8, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[7], -17.5, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[8], 61.25, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[9], 8.75, 1.e-4 );
  BOOST_CHECK_CLOSE( stats[10], -56, 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_json_to_prof1D ) {
  nlohmann::json j{
      { "type", "histogram:ProfileHistogram:double" },
      { "title", "MyNiceTitle" },
      { "dimension", 1 },
      { "empty", false },
      { "nEntries", 6 },
      { "axis", { { { "maxValue", 1.5 }, { "minValue", -0.5 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } } } },
      { "bins", { { { 1.0, 2.0 }, 4.0 }, { { 5.0, 15.0 }, 55.0 }, { { 0.0, 0.0 }, 0.0 }, { { 0.0, 0.0 }, 0.0 } } } };
  std::string dir{ "dir" };
  std::string name{ "subdir/name" };
  auto [histo, fullDir] = jsonToRootHistogram<Traits<true, TProfile, 1>>( dir, name, j );
  BOOST_CHECK( fullDir == "dir/subdir" );
  BOOST_CHECK( strcmp( histo.GetTitle(), "MyNiceTitle" ) == 0 );
  BOOST_CHECK( histo.GetDimension() == 1 );
  BOOST_CHECK_CLOSE( histo.GetEntries(), 6, 1.e-4 );
  checkAxis( *histo.GetXaxis(), 2, -0.5, 1.5, "MyNiceAxisTitle" );
  auto* sumw2 = histo.GetSumw2();
  BOOST_CHECK_CLOSE( histo.GetBinEntries( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 0 ), 2.0, 1.e-4 );
  BOOST_CHECK_CLOSE( sumw2->At( 0 ), 4.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 0 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinEntries( 1 ), 5.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 1 ), 3.0, 1.e-4 );
  BOOST_CHECK_CLOSE( sumw2->At( 1 ), 55.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 1 ), std::sqrt( 0.4 ), 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinEntries( 2 ), 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinContent( 2 ), 1.e-4 );
  BOOST_CHECK_SMALL( sumw2->At( 2 ), 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 2 ), 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_json_to_prof2D ) {
  nlohmann::json j{ { "type", "histogram:ProfileHistogram:double" },
                    { "title", "MyNiceTitle" },
                    { "dimension", 2 },
                    { "empty", false },
                    { "nEntries", 6 },
                    { "axis",
                      { { { "maxValue", 1.5 }, { "minValue", -0.5 }, { "nBins", 2 }, { "title", "MyNiceAxisTitle" } },
                        { { "maxValue", 4.5 }, { "minValue", 2.5 }, { "nBins", 2 }, { "title", "" } } } },
                    { "bins",
                      { { { 1.0, 2.0 }, 4.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 5.0, 15.0 }, 55.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 },
                        { { 0.0, 0.0 }, 0.0 } } } };
  std::string    dir{ "dir" };
  std::string    name{ "subdir/name" };
  auto [histo, fullDir] = jsonToRootHistogram<Traits<true, TProfile2D, 2>>( dir, name, j );
  BOOST_CHECK( fullDir == "dir/subdir" );
  BOOST_CHECK( strcmp( histo.GetTitle(), "MyNiceTitle" ) == 0 );
  BOOST_CHECK( histo.GetDimension() == 2 );
  BOOST_CHECK_CLOSE( histo.GetEntries(), 6, 1.e-4 );
  checkAxis( *histo.GetXaxis(), 2, -0.5, 1.5, "MyNiceAxisTitle" );
  checkAxis( *histo.GetYaxis(), 2, 2.5, 4.5, "" );
  auto* sumw2 = histo.GetSumw2();
  BOOST_CHECK_CLOSE( histo.GetBinEntries( 0 ), 1.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 0 ), 2.0, 1.e-4 );
  BOOST_CHECK_CLOSE( sumw2->At( 0 ), 4.0, 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 0 ), 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinEntries( 5 ), 5.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinContent( 5 ), 3.0, 1.e-4 );
  BOOST_CHECK_CLOSE( sumw2->At( 5 ), 55.0, 1.e-4 );
  BOOST_CHECK_CLOSE( histo.GetBinError( 5 ), std::sqrt( 0.4 ), 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinEntries( 2 ), 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinContent( 2 ), 1.e-4 );
  BOOST_CHECK_SMALL( sumw2->At( 2 ), 1.e-4 );
  BOOST_CHECK_SMALL( histo.GetBinError( 2 ), 1.e-4 );
}

BOOST_AUTO_TEST_CASE( formatName ) {
  using Gaudi::Histograming::Sink::detail::formatName;

  BOOST_CHECK( formatName( "123abc", 15 ) == "123abc" );
  BOOST_CHECK( formatName( "123abc123abc123", 15 ) == "123abc123abc123" );
  BOOST_CHECK( formatName( "123abc123abc123abc", 15 ) == "123ab...c123abc" );
  // just to check an even width...
  BOOST_CHECK( formatName( "123abc123abc123abc", 14 ) == "123ab...123abc" );
}
