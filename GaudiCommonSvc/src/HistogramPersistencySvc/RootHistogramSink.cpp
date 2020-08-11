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

#include "GaudiKernel/Service.h"

#include <Gaudi/MonitoringHub.h>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TProfile3D.h>

namespace {
  
  void saveRootHisto1D(const std::string& component, const std::string& name, nlohmann::json& j) {
    std::string id = component + "/" + name;
    // extract data from json
    auto jaxis = j.at("axis");
    auto nBins = jaxis[0].at("nBins").get<unsigned int>();
    auto minValue = jaxis[0].at("minValue").get<double>();
    auto maxValue = jaxis[0].at("maxValue").get<double>();
    auto weights = j.at("bins").get<std::vector<double>>();
    assert(weights.size() == nBins+2);
    // Create Root histogram and fill it
    // FIXME , we should use j.at("internalType") to gather precision and respect it
    TH1D histo(id.c_str(), id.c_str(), nBins, minValue, maxValue);
    for (unsigned int i = 0; i < nBins+2; i++) histo.SetBinContent(i, weights[i]);
    // write to file
    histo.Write();
  }
  
  void saveRootHisto2D(const std::string& component, const std::string& name, nlohmann::json& j) {
    std::string id = component + "/" + name;
    // extract data from json
    auto jaxis = j.at("axis");
    auto nBins0 = jaxis[0].at("nBins").get<unsigned int>();
    auto minValue0 = jaxis[0].at("minValue").get<double>();
    auto maxValue0 = jaxis[0].at("maxValue").get<double>();
    auto nBins1 = jaxis[1].at("nBins").get<unsigned int>();
    auto minValue1 = jaxis[1].at("minValue").get<double>();
    auto maxValue1 = jaxis[1].at("maxValue").get<double>();
    auto weights = j.at("bins").get<std::vector<double>>();
    auto totNBins = (nBins0+2)*(nBins1+2);
    assert(weights.size() == totNBins);
    // Create Root histogram and fill it
    // FIXME , we should use j.at("internalType") to gather precision and respect it
    TH2D histo(id.c_str(), id.c_str(), nBins0, minValue0, maxValue0, nBins1, minValue1, maxValue1);
    for (unsigned int i = 0; i < totNBins; i++) histo.SetBinContent(i, weights[i]);
    // write to file
    histo.Write();
  }

  void saveRootHisto3D(const std::string& component, const std::string& name, nlohmann::json& j) {
    std::string id = component + "/" + name;
    // extract data from json
    auto jaxis = j.at("axis");
    auto nBins0 = jaxis[0].at("nBins").get<unsigned int>();
    auto minValue0 = jaxis[0].at("minValue").get<double>();
    auto maxValue0 = jaxis[0].at("maxValue").get<double>();
    auto nBins1 = jaxis[1].at("nBins").get<unsigned int>();
    auto minValue1 = jaxis[1].at("minValue").get<double>();
    auto maxValue1 = jaxis[1].at("maxValue").get<double>();
    auto nBins2 = jaxis[2].at("nBins").get<unsigned int>();
    auto minValue2 = jaxis[2].at("minValue").get<double>();
    auto maxValue2 = jaxis[2].at("maxValue").get<double>();
    auto weights = j.at("bins").get<std::vector<double>>();
    auto totNBins = (nBins0+2)*(nBins1+2)*(nBins2+2);
    assert(weights.size() == totNBins);
    // Create Root histogram and fill it
    // FIXME , we should use j.at("internalType") to gather precision and respect it
    TH3D histo(id.c_str(), id.c_str(), nBins0, minValue0, maxValue0, nBins1, minValue1, maxValue1, nBins2, minValue2, maxValue2);
    for (unsigned int i = 0; i < totNBins; i++) histo.SetBinContent(i, weights[i]);
    // Save histo to file
    histo.Write();
  }

  /// Wrapper around TProfileX to be able to fill it
  template <typename TP>
  struct ProfileWrapper : TP {
    using TP::TP;
    void setBinNEntries(Int_t i, Int_t n) {
      this->fBinEntries.fArray[i] = n;
    }
    void setBinW2(Int_t i, Double_t v) {
      this->fSumw2.fArray[i] = v;
    }
  };
  
  void saveRootProfHisto1D(const std::string& component, const std::string& name, nlohmann::json& j) {
    std::string id = component + "/" + name;
    // extract data from json
    auto jaxis = j.at("axis");
    auto nBins = jaxis[0].at("nBins").get<unsigned int>();
    auto minValue = jaxis[0].at("minValue").get<double>();
    auto maxValue = jaxis[0].at("maxValue").get<double>();
    auto weights = j.at("bins").get<std::vector<std::tuple<std::tuple<unsigned int, double>, double>>>();
    assert(weights.size() == nBins+2);
    // Create Root histogram and fill it
    // FIXME , we should use j.at("internalType") to gather precision and respect it
    ProfileWrapper<TProfile> histo(id.c_str(), id.c_str(), nBins, minValue, maxValue);
    for (unsigned int i = 0; i < nBins+2; i++) {
      auto [c, sumWeight2] = weights[i];
      auto [nEntries, sumWeight] = c;
      histo.setBinNEntries(i, nEntries);
      histo.SetBinContent(i, sumWeight);
      histo.setBinW2(i, sumWeight2);
    }
    // write to file
    histo.Write();
  }
  void saveRootProfHisto2D(const std::string& component, const std::string& name, nlohmann::json& j) {
    std::string id = component + "/" + name;
    // extract data from json
    auto jaxis = j.at("axis");
    auto nBins0 = jaxis[0].at("nBins").get<unsigned int>();
    auto minValue0 = jaxis[0].at("minValue").get<double>();
    auto maxValue0 = jaxis[0].at("maxValue").get<double>();
    auto nBins1 = jaxis[1].at("nBins").get<unsigned int>();
    auto minValue1 = jaxis[1].at("minValue").get<double>();
    auto maxValue1 = jaxis[1].at("maxValue").get<double>();
    auto weights = j.at("bins").get<std::vector<std::tuple<std::tuple<unsigned int, double>, double>>>();
    auto totNBins = (nBins0+2)*(nBins1+2);
    assert(weights.size() == totNBins);
    // Create Root histogram and fill it
    // FIXME , we should use j.at("internalType") to gather precision and respect it
    ProfileWrapper<TProfile2D> histo(id.c_str(), id.c_str(), nBins0, minValue0, maxValue0, nBins1, minValue1, maxValue1);
    for (unsigned int i = 0; i < totNBins; i++) {
      auto [c, sumWeight2] = weights[i];
      auto [nEntries, sumWeight] = c;
      histo.setBinNEntries(i, nEntries);
      histo.SetBinContent(i, sumWeight);
      histo.setBinW2(i, sumWeight2);
    }
    // write to file
    histo.Write();
  }
  
  void saveRootProfHisto3D(const std::string& component, const std::string& name, nlohmann::json& j) {
    std::string id = component + "/" + name;
    // extract data from json
    auto jaxis = j.at("axis");
    auto nBins0 = jaxis[0].at("nBins").get<unsigned int>();
    auto minValue0 = jaxis[0].at("minValue").get<double>();
    auto maxValue0 = jaxis[0].at("maxValue").get<double>();
    auto nBins1 = jaxis[1].at("nBins").get<unsigned int>();
    auto minValue1 = jaxis[1].at("minValue").get<double>();
    auto maxValue1 = jaxis[1].at("maxValue").get<double>();
    auto nBins2 = jaxis[2].at("nBins").get<unsigned int>();
    auto minValue2 = jaxis[2].at("minValue").get<double>();
    auto maxValue2 = jaxis[2].at("maxValue").get<double>();
    auto weights = j.at("bins").get<std::vector<std::tuple<std::tuple<unsigned int, double>, double>>>();
    auto totNBins = (nBins0+2)*(nBins1+2)*(nBins2+2);
    assert(weights.size() == totNBins);
    // Create Root histogram and fill it
    // FIXME , we should use j.at("internalType") to gather precision and respect it
    ProfileWrapper<TProfile3D> histo(id.c_str(), id.c_str(), nBins0, minValue0, maxValue0, nBins1, minValue1, maxValue1, nBins2, minValue2, maxValue2);
    for (unsigned int i = 0; i < totNBins; i++) {
      auto [c, sumWeight2] = weights[i];
      auto [nEntries, sumWeight] = c;
      histo.setBinNEntries(i, nEntries);
      histo.SetBinContent(i, sumWeight);
      histo.setBinW2(i, sumWeight2);
    }
    // write to file
    histo.Write();
  }

}

namespace Gaudi::Histograming {

  class RootHistogramSink : public Service, public Gaudi::Monitoring::Hub::Sink {

  public:

    using Service::Service;

    StatusCode initialize() override {
      StatusCode sc = Service::initialize();
      if ( sc.isFailure() ) return sc;
      serviceLocator()->monitoringHub().addSink( this );
      return StatusCode::SUCCESS;
    }

    StatusCode stop() override {
      auto ok = Service::stop();
      if ( !ok ) return ok;
      m_monitoringEntities.sort( []( const auto& a, const auto& b ) {
                                   return a.component == b.component ? a.name > b.name : a.component > b.component ;
                                 } );
      TFile histoFile("testHisto.root", "RECREATE");
      std::for_each( begin( m_monitoringEntities ), end( m_monitoringEntities ),
                     []( auto& ent ) {
                       auto j = ent.toJSON();
                       auto dim = j.at("dimension").template get<unsigned int>();
                       if (dim > 3) throw std::string("Unable to save ROOT histogram for dimension > 3");
                       if (dim == 0) throw std::string("Invalid histogram json with dimension 0");
                       auto subtype = j.at("subtype").template get<std::string>();
                       if (subtype == "histogram") {
                         switch (dim) {
                         case 1:
                           saveRootHisto1D(ent.component, ent.name, j);
                           break;
                         case 2:
                           saveRootHisto2D(ent.component, ent.name, j);
                           break;
                         case 3 :
                           saveRootHisto3D(ent.component, ent.name, j);
                           break;
                         }
                       } else if (subtype == "profilehistogram") {
                         switch (dim) {
                         case 1:
                           saveRootProfHisto1D(ent.component, ent.name, j);
                           break;
                         case 2:
                           saveRootProfHisto2D(ent.component, ent.name, j);
                           break;
                         case 3 :
                           saveRootProfHisto3D(ent.component, ent.name, j);
                           break;
                         }
                       } else {
                         throw std::string("Unknown histogram subtype : " + subtype);
                       }
                     } );
      histoFile.Close();
      return ok;
    }

    void registerEntity( Gaudi::Monitoring::Hub::Entity ent ) override {
      if ( ent.type == "histogram" ) {
        m_monitoringEntities.emplace_back( std::move( ent ) );
      }
    }

  private:
    std::list<Gaudi::Monitoring::Hub::Entity> m_monitoringEntities;

  };

  DECLARE_COMPONENT( RootHistogramSink )

} // namespace Gaudi::Histograming

