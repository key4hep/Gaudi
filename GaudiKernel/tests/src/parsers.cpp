#include "GaudiKernel/VectorsAsProperty.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/VectorMap.h"
#include <boost/test/minimal.hpp>

#include "GaudiKernel/ToStream.h"

#include "GaudiKernel/Parsers.h"


using namespace Gaudi::Parsers;


int test_main(int /*argc*/, char** /*argv*/)             // note the name!
{
//==============================================================================
// StringGrammar
//==============================================================================
    {
        std::string result;
        BOOST_CHECK(parse(result, "'Hello \\ \\' world'"));
        BOOST_CHECK(result=="Hello \\ ' world");
    }
//==============================================================================
// IntGrammar
//==============================================================================
    {
        int result;
        BOOST_CHECK(parse(result, "1000"));
        BOOST_CHECK(result==1000);
    }

    {
        long result;
        BOOST_CHECK(parse(result, "1000"));
        BOOST_CHECK(result==1000);
    }

    {
        long result;
        BOOST_CHECK(parse(result, "1L"));
        BOOST_CHECK(result==1);
    }

    {
        long result;
        BOOST_CHECK(parse(result, "100L"));
        BOOST_CHECK(result==100);
    }
//==============================================================================
// CharGrammar
//==============================================================================
    {
        char result;
        BOOST_CHECK(parse(result, "'a'"));
        BOOST_CHECK(result=='a');
    }
//==============================================================================
// BoolGrammar
//==============================================================================
    {
        bool result;
        BOOST_CHECK(parse(result, "True"));
        BOOST_CHECK(result);
        BOOST_CHECK(parse(result, "False"));
        BOOST_CHECK(!result);
    }
//==============================================================================
// RealGrammar
//==============================================================================
    {
        double result;
        BOOST_CHECK( parse(result, "1.5E2") );
        BOOST_CHECK( result == 150);
    }

    {
        float result;
        BOOST_CHECK( parse(result, "1.5E2") );
        BOOST_CHECK( result == 150);
    }
//==============================================================================
// VectorGramar
//==============================================================================
    {
        std::vector<int> result;
        BOOST_CHECK(parse(result, "[1, 2,  3] // Test Comments"));
        BOOST_CHECK(result.size()==3);
        BOOST_CHECK(result[0]==1);
        BOOST_CHECK(result[1]==2);
        BOOST_CHECK(result[2]==3);
    }

    {
        std::vector<std::vector<double> > result;
        BOOST_CHECK(parse(result, "[[1, 2]/* Test comments */,[3]]"));
        BOOST_CHECK(result.size()==2);
        BOOST_CHECK(result[0][0]==1);
        BOOST_CHECK(result[0][1]==2);
        BOOST_CHECK(result[1][0]==3);
    }

    {
        std::vector<double> result;
        BOOST_CHECK(parse(result, "[1.1, 2.2 ]"));
        BOOST_CHECK(result.size()==2);
        BOOST_CHECK(result[0]==1.1);
        BOOST_CHECK(result[1]==2.2);
    }

    /*{
        std::set<double> result;
        BOOST_CHECK(parse(result, "[1.1, 2.2 ]"));
        BOOST_CHECK(result.size()==2);
        BOOST_CHECK(result.count(1.1)==1);
        BOOST_CHECK(result.count(2.2)==1);
    }

    {
      std::list<double> result;
      BOOST_CHECK(parse(result, "[1.1, 2.2 ]"));
      BOOST_CHECK(result.size()==2);
      BOOST_CHECK(result.front()==1.1);
      BOOST_CHECK(result.back()==2.2);
    }*/
//==============================================================================
// PairGramar
//==============================================================================
    {

        std::pair<double, double> result;
        BOOST_CHECK(parse(result, "(10.1, 10)"));
        BOOST_CHECK(result.first == 10.1);
        BOOST_CHECK(result.second == 10);
    }

//==============================================================================
// MapGramar
//==============================================================================

    {
        std::map<std::string, int> result;
        BOOST_CHECK(parse(result, "{'key':10, 'key1'=20}"));
        BOOST_CHECK(result.size() == 2);
        BOOST_CHECK(result["key"] == 10);
        BOOST_CHECK(result["key1"] == 20);
    }

    {
        std::map<std::string, std::vector<double> > result;
        BOOST_CHECK(parse(result, "{'key':[10.0,20.0]}"));
        BOOST_CHECK(result.size() == 1);
        BOOST_CHECK(result["key"].size() == 2);
        BOOST_CHECK(result["key"][0] == 10);
    }


    {
        GaudiUtils::VectorMap<Gaudi::StringKey, double> result;
        BOOST_CHECK(parse(result, "{'key':10, 'key1'=20}"));
        BOOST_CHECK(result.size() == 2);
        Gaudi::StringKey key = std::string("key");
        BOOST_CHECK(result.at(key) == 10);
        key = std::string("key1");
        BOOST_CHECK(result.at(key) == 20);
    }
//==============================================================================
// Pnt3DTypes
//==============================================================================
    {
        Gaudi::XYZPoint result;
        BOOST_CHECK(parse(result, "(px:10.0, py:11.0, pZ:12.0)"));
        BOOST_CHECK(result.X() == 10.0);
        BOOST_CHECK(result.Y() == 11.0);
        BOOST_CHECK(result.Z() == 12.0);

    }
//==============================================================================
// Pnt4DTypes
//==============================================================================
    {
        Gaudi::LorentzVector result;
        BOOST_CHECK(parse(result, "(px:10.0, py:11.0, pZ:12.0;100.0)"));
        BOOST_CHECK(result.X() == 10.0);
        BOOST_CHECK(result.Y() == 11.0);
        BOOST_CHECK(result.Z() == 12.0);
        BOOST_CHECK(result.T() == 100.0);
    }

//==============================================================================
// Pnt4DTypes
//==============================================================================
    {
        Gaudi::Histo1DDef result;
        BOOST_CHECK(parse(result, "('test', 1.0,2.0, 100)"));
        BOOST_CHECK (result.title() == "test");
        BOOST_CHECK (result.lowEdge() == 1.0);
        BOOST_CHECK (result.highEdge() == 2.0);
        BOOST_CHECK (result.bins() == 100);
        Gaudi::Utils::toStream(result, std::cout);
        std::cout << result << std::endl;
    }

    {
        Gaudi::Histo1DDef result;
        BOOST_CHECK(parse(result, "(1.0,2.0, 'test', 100)"));
        BOOST_CHECK (result.title() == "test");
        BOOST_CHECK (result.lowEdge() == 1.0);
        BOOST_CHECK (result.highEdge() == 2.0);
        BOOST_CHECK (result.bins() == 100);
    }

//==============================================================================
    return 0;
}



