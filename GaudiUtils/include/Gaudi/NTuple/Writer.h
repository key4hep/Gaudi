/***********************************************************************************\
* (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Algorithm.h>
#include <Gaudi/Functional/Consumer.h>
#include <Gaudi/Interfaces/IFileSvc.h>
#include <Gaudi/details/BranchWrapper.h>
#include <GaudiKernel/StdArrayAsProperty.h>
#include <TFile.h>
#include <TTree.h>
#include <fmt/format.h>
#include <gsl/pointers>
#include <gsl/span>
#include <mutex>
#include <numeric>
#include <tuple>
#include <utility>

namespace Gaudi::NTuple {

  /**
   * @struct Writer
   * @brief Base template for NTuple::Writer.
   * Actual specializations of this template provide the functionality.
   */
  template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>>
  struct SimpleWriter {
    SimpleWriter() = delete;
  };

  /**
   * @struct SimpleWriter
   * @brief Specialized Algorithm for NTuple writing.
   *
   * Provides the functionality for initializing branches in a ROOT TTree,
   * transforming input data, filling the tree, and writing it to a file.
   *
   * An algorithm inheriting from this class needs to implement the operator() and can call fillTree
   * as many times as needed per event. Note that fillTree is internally synchronized, so calling it
   * is thread safe.
   * The main Properties are :
   *  - OutputFile : the entry in the FileSvc saying into which file the NTuple should be written
   *  - NTupleName : the name of the NTuple
   *  - BranchNames : list of names of the NTuple Branches/Columns. Must match the number of items
   *    in the OUTPUTS parameter pack
   *
   * @tparam OUTPUTs Variadic template parameters representing output data types.
   * @tparam INPUTs Variadic template parameters representing input data types.
   */
  template <typename... OUTPUTs, typename... INPUTs, typename Traits_>
  struct SimpleWriter<std::tuple<OUTPUTs...>( const INPUTs&... ), Traits_>
      : Gaudi::Functional::Consumer<void( const INPUTs&... ), Traits_> {
    using Consumer_t = Gaudi::Functional::Consumer<void( const INPUTs&... ), Traits_>;
    using Consumer_t::Consumer_t;

    Gaudi::Property<std::string> m_fileId{ this, "OutputFile", "NTuple", "Identifier for the TFile to write to." };
    Gaudi::Property<std::string> m_ntupleTname{ this, "NTupleName", this->name(), "Name of the TTree." };
    Gaudi::Property<std::array<std::string, sizeof...( OUTPUTs )>> m_branchNames{
        this, "BranchNames", {}, "Names of the tree branches." }; // Names for the tree branches

    // Fill the TTree with transformed data
    void fillTree( const std::tuple<OUTPUTs...>& transformedData ) const {
      std::scoped_lock lock{ m_mtx };
      std::apply(
          [&]( const auto&... elems ) {
            size_t index = 0;
            ( ..., m_branchWrappers[index++].setDataPtr( const_cast<void*>( static_cast<const void*>( &elems ) ) ) );
          },
          transformedData );
      m_tree->Fill();
    }

    // Initialize the algorithm, set up the ROOT file and a TTree branch for each input location
    virtual StatusCode initialize() override {
      return Consumer_t::initialize().andThen( [this]() {
        m_file = m_fileSvc->getFile( m_fileId );
        if ( !m_file ) {
          this->error() << "Failed to retrieve TFile." << endmsg;
          return StatusCode::FAILURE;
        }
        this->initTree( m_file, m_ntupleTname.value(), m_branchNames.value(), this->name() );
        return StatusCode::SUCCESS;
      } );
    }

    // Finalize the algorithm by writing the TTree to the file and closing it
    virtual StatusCode finalize() override {
      this->writeTree( m_file, this->name() );
      return Consumer_t::finalize();
    }

  private:
    // Initialize the TTree and creates branches
    void initTree( const std::shared_ptr<TFile>& file, const std::string& ntupleName,
                   const gsl::span<std::string, sizeof...( OUTPUTs )> branchNames, const std::string& algName ) {
      file->cd();
      m_tree = std::make_unique<TTree>( ntupleName.c_str(), "Tree of Writer Algorithm" ).release();
      m_branchWrappers.reserve( m_branchWrappers.size() + sizeof...( OUTPUTs ) );
      createBranchesForOutputs( branchNames, std::make_index_sequence<sizeof...( OUTPUTs )>{}, algName );
    }

    // Create branches in the TTree based on the provided names and output data types
    template <std::size_t... Is>
    void createBranchesForOutputs( const gsl::span<std::string, sizeof...( OUTPUTs )> branchNames,
                                   const std::index_sequence<Is...>, const std::string& algName ) const {
      ( ..., m_branchWrappers.emplace_back(
                 m_tree, System::typeinfoName( typeid( std::tuple_element_t<Is, std::tuple<OUTPUTs...>> ) ),
                 branchNames[Is], "", algName ) );
    }

    // Write the TTree to the associated ROOT file
    void writeTree( const std::shared_ptr<TFile>& file, const std::string& algName ) {
      file->cd();
      if ( m_tree->Write() <= 0 ) {
        throw GaudiException( "Failed to write TTree to ROOT file.", algName, StatusCode::FAILURE );
      }
      m_tree = nullptr;
    }

    ServiceHandle<Gaudi::Interfaces::IFileSvc>  m_fileSvc{ this, "FileSvc", "FileSvc" };
    TTree*                                      m_tree = nullptr;   // Pointer to the TTree being written to
    mutable std::vector<details::BranchWrapper> m_branchWrappers{}; // Container for BranchWrapper objects
    std::shared_ptr<TFile>                      m_file = nullptr;   // Pointer to the ROOT file
    mutable std::mutex                          m_mtx;              // Mutex for thread-safe operations
  };

  /**
   * @struct Writer
   * @brief Base template for NTuple::Writer.
   * Actual specializations of this template provide the functionality.
   */
  template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>>
  struct Writer {
    Writer() = delete; // If you wish to not provide any transformation for your data, please use the
                       // NTuple::GenericWriter
    // algorithm
  };

  /**
   * @struct Writer
   * @brief Specialized algorithm for NTuple writing where one line is written per event
   * and a transformation from INPUTS to the stored items, aka OUTPUTS
   *
   * A class inheriting Writer only has to provide the transform method converting
   * INPUTS into OUTPUTS.
   * Write inherits from SimpleWriter and thus has the same Properties
   *
   * @tparam OUTPUTs Variadic template parameters representing output data types.
   * @tparam INPUTs Variadic template parameters representing input data types.
   */
  template <typename... OUTPUTs, typename... INPUTs, typename Traits_>
  struct Writer<std::tuple<OUTPUTs...>( const INPUTs&... ), Traits_>
      : SimpleWriter<std::tuple<OUTPUTs...>( const INPUTs&... ), Traits_> {

    using SimpleWriter_t = SimpleWriter<std::tuple<OUTPUTs...>( const INPUTs&... ), Traits_>;
    using SimpleWriter_t::SimpleWriter_t;

    /**
     * Transform input data to the desired output format. Must be overridden in derived classes.
     * @param inputs Variadic inputs of types specified by INPUTs...
     * @return A tuple of output data corresponding to OUTPUTs...
     */
    virtual std::tuple<OUTPUTs...> transform( const INPUTs&... inputs ) const = 0;

    // Execute the algorithm for each event, retrieving data from the event store and writing it to the TTree
    void operator()( const INPUTs&... inputs ) const override {
      auto transformedData = transform( inputs... );
      this->fillTree( transformedData );
    }
  };

} // namespace Gaudi::NTuple
