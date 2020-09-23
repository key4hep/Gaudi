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

#include <fmt/format.h>

#include <deque>
#include <algorithm>

namespace {

  struct Axis {
    unsigned int nBins;
    double minValue;
    double maxValue;
  };
  
  Axis toAxis( nlohmann::json& jAxis ) {
    return { jAxis.at("nBins").get<unsigned int>(),
             jAxis.at("minValue").get<double>(),
             jAxis.at("maxValue").get<double>() };
  }

  template<typename ROOTHisto, typename WEIGHT, std::size_t... index, typename UpdFunc>
  void saveRootHistoInternal(const std::string& component, const std::string& name, nlohmann::json& j,
                             std::index_sequence<index...>, UpdFunc f) {
    auto id = fmt::format("{}/{}", component, name );
    // extract data from json
    auto jsonAxis = j.at("axis");
    auto axis = std::array{ toAxis( jsonAxis[index] )... };
    auto weights = j.at("bins").get<WEIGHT>();
    // compute total number of bins, multiplying bins per axis
    auto totNBins = ((axis[index].nBins+2) * ...);
    assert(weights.size() == totNBins);
    // Create Root histogram calling constructors with the args tuple
    auto histo = std::make_from_tuple<ROOTHisto>( std::tuple_cat( std::tuple{ id.c_str(), id.c_str() },
                                                                  std::tuple{ axis[index].nBins, axis[index].minValue, axis[index].maxValue} ... ) );
    // fill Histo
    for (unsigned int i = 0; i < totNBins; i++) f(histo, i, weights);
    // write to file
    histo.Write();
  }

  template<unsigned int ND, typename ROOTHisto>
  void saveRootHisto(const std::string& component, const std::string& name, nlohmann::json& j) {
    saveRootHistoInternal<ROOTHisto, std::vector<double>>
      (component, name, j, std::make_index_sequence<ND>(),
       [](ROOTHisto& histo, unsigned int i, auto& weights) { histo.SetBinContent(i, weights[i]); });
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

  template<unsigned int ND, typename ROOTHisto>
  void saveRootProfHisto(const std::string& component, const std::string& name, nlohmann::json& j) {
    using Histo = ProfileWrapper<ROOTHisto>;
    saveRootHistoInternal<Histo, std::vector<std::tuple<std::tuple<unsigned int, double>, double>>>
      (component, name, j, std::make_index_sequence<ND>(),
       [](Histo& histo, unsigned int i, auto& weights) {
         auto [c, sumWeight2] = weights[i];
         auto [nEntries, sumWeight] = c;
         histo.setBinNEntries(i, nEntries);
         histo.SetBinContent(i, sumWeight);
         histo.setBinW2(i, sumWeight2);
       });
  }

}

namespace Gaudi::Histograming::Sink {

  class Root : public Service, public Gaudi::Monitoring::Hub::Sink {

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
      std::sort(begin(m_monitoringEntities), end(m_monitoringEntities),
                []( const auto& a, const auto& b ) {
                  return a.component == b.component ? a.name > b.name : a.component > b.component ;
                } );
      TFile histoFile("testHisto.root", "RECREATE");
      std::for_each( begin( m_monitoringEntities ), end( m_monitoringEntities ),
                     []( auto& ent ) {
                       auto j = ent.toJSON();
                       auto dim = j.at("dimension").template get<unsigned int>();
                       if (dim > 3) throw GaudiException("Unable to save ROOT histogram for dimension > 3", "Histogram::Sink::Root", StatusCode::FAILURE);
                       if (dim == 0) throw GaudiException("Invalid histogram json with dimension 0", "Histogram::Sink::Root", StatusCode::FAILURE);
                       auto type = j.at("type").template get<std::string>();
                       if ( type == "histogram:Histogram" ||
                            type == "histogram:WeightedHistogram" ) {
                         switch (dim) {
                         case 1:
                           saveRootHisto<1, TH1D>(ent.component, ent.name, j);
                           break;
                         case 2:
                           saveRootHisto<2, TH2D>(ent.component, ent.name, j);
                           break;
                         case 3 :
                           saveRootHisto<3, TH3D>(ent.component, ent.name, j);
                           break;
                         }
                       } else if ( type == "histogram:ProfileHistogram" ||
                                   type == "histogram:WeightedProfileHistogram" ) {
                         switch (dim) {
                         case 1:
                           saveRootProfHisto<1, TProfile>(ent.component, ent.name, j);
                           break;
                         case 2:
                           saveRootProfHisto<2, TProfile2D>(ent.component, ent.name, j);
                           break;
                         case 3 :
                           saveRootProfHisto<3, TProfile3D>(ent.component, ent.name, j);
                           break;
                         }
                       } else {
                         throw GaudiException("Unknown histogram type : " + type, "Histogram::Sink::Root", StatusCode::FAILURE);
                       }
                     } );
      histoFile.Close();
      return ok;
    }

    void registerEntity( Gaudi::Monitoring::Hub::Entity ent ) override {
      if ( std::string_view( ent.type ).substr( 0, 10 ) == "histogram:" ) {
        m_monitoringEntities.emplace_back( std::move( ent ) );
      }
    }

  private:
    std::deque<Gaudi::Monitoring::Hub::Entity> m_monitoringEntities;

  };

  DECLARE_COMPONENT( Root )

} // namespace Gaudi::Histograming::Sink

