/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/INTupleSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/NTuple.h>

#include <cmath>
#include <memory>
#include <string>
#include <vector>

/** Books a configurable grid of column-wise tuples: many trees, many columns,
 *  few entries. That is the shape where per-branch write buffers dominate a job,
 *  and where NTupleSvc.DiskBuffer is meant to help, so it is what both the
 *  equivalence test and the benchmark need.
 *
 *  Every column kind the converter handles appears: scalars of each type,
 *  arrays and matrices counted by an index (two indexes, each shared by several
 *  columns), and arrays and matrices of fixed size. Array lengths vary from
 *  entry to entry, including zero, since a zero-length column is where an
 *  off-by-one in the offset walk would hide.
 */
class WideNTupleAlgorithm : public Algorithm {
public:
  using Algorithm::Algorithm;

  StatusCode initialize() override;
  StatusCode execute() override;

private:
  Gaudi::Property<int>         m_nTuples{ this, "NTuples", 4, "number of tuples to book" };
  Gaudi::Property<int>         m_nColumns{ this, "Columns", 64, "approximate number of columns per tuple" };
  Gaudi::Property<int>         m_entries{ this, "EntriesPerTuple", 10, "entries written to each tuple" };
  Gaudi::Property<int>         m_maxArray{ this, "MaxArraySize", 8, "declared maximum of the variable columns" };
  Gaudi::Property<std::string> m_dir{ this, "Directory", "MyTuples", "top-level directory" };

  /// One booked tuple and everything written into it
  struct Block {
    NTuple::Tuple*                      tuple = nullptr;
    NTuple::Item<int>                   n;
    NTuple::Item<int>                   m;
    std::vector<NTuple::Item<double>>   d;
    std::vector<NTuple::Item<float>>    f;
    std::vector<NTuple::Item<int>>      i;
    std::vector<NTuple::Item<bool>>     b;
    std::vector<NTuple::Array<double>>  var;
    std::vector<NTuple::Array<int>>     var2;
    std::vector<NTuple::Array<float>>   fixed;
    std::vector<NTuple::Matrix<double>> mat;
    std::vector<NTuple::Matrix<float>>  fmat;
  };
  std::vector<std::unique_ptr<Block>> m_blocks;
  int                                 m_event = 0;
};

DECLARE_COMPONENT( WideNTupleAlgorithm )

StatusCode WideNTupleAlgorithm::initialize() {
  StatusCode sc = Algorithm::initialize();
  if ( sc.isFailure() ) return sc;

  // Split the requested width evenly over the column kinds
  const int nEach = std::max( 1, m_nColumns / 9 );

  for ( int t = 0; t < m_nTuples; ++t ) {
    auto        blk  = std::make_unique<Block>();
    std::string path = m_dir.value() + "/t" + std::to_string( t );
    blk->tuple       = ntupleSvc()->book( path, CLID_ColumnWiseTuple, "wide tuple " + std::to_string( t ) );
    if ( !blk->tuple ) {
      error() << "cannot book " << path << endmsg;
      return StatusCode::FAILURE;
    }
    blk->d.resize( nEach );
    blk->f.resize( nEach );
    blk->i.resize( nEach );
    blk->b.resize( nEach );
    blk->var.resize( nEach );
    blk->var2.resize( nEach );
    blk->fixed.resize( nEach );
    blk->mat.resize( nEach );
    blk->fmat.resize( nEach );

    // The index has to be declared before the columns it counts: ROOT resolves a
    // leaf count by name against the leaves already in the tree.
    if ( blk->tuple->addItem( "n", blk->n, 0, m_maxArray.value() ).isFailure() ) return StatusCode::FAILURE;
    if ( blk->tuple->addItem( "m", blk->m, 0, m_maxArray.value() ).isFailure() ) return StatusCode::FAILURE;
    for ( int c = 0; c < nEach; ++c ) {
      const auto s = std::to_string( c );
      if ( blk->tuple->addItem( "d" + s, blk->d[c] ).isFailure() ) return StatusCode::FAILURE;
      if ( blk->tuple->addItem( "f" + s, blk->f[c] ).isFailure() ) return StatusCode::FAILURE;
      if ( blk->tuple->addItem( "i" + s, blk->i[c] ).isFailure() ) return StatusCode::FAILURE;
      if ( blk->tuple->addItem( "b" + s, blk->b[c] ).isFailure() ) return StatusCode::FAILURE;
      if ( blk->tuple->addIndexedItem( "var" + s, blk->n, blk->var[c] ).isFailure() ) return StatusCode::FAILURE;
      if ( blk->tuple->addIndexedItem( "var2_" + s, blk->m, blk->var2[c] ).isFailure() ) return StatusCode::FAILURE;
      if ( blk->tuple->addItem( "fix" + s, 3, blk->fixed[c] ).isFailure() ) return StatusCode::FAILURE;
      if ( blk->tuple->addIndexedItem( "mat" + s, blk->n, 2, blk->mat[c] ).isFailure() ) return StatusCode::FAILURE;
      if ( blk->tuple->addItem( "fmat" + s, 2, 3, blk->fmat[c] ).isFailure() ) return StatusCode::FAILURE;
    }
    m_blocks.push_back( std::move( blk ) );
  }
  info() << "booked " << m_blocks.size() << " tuples of about " << m_nColumns.value() << " columns" << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode WideNTupleAlgorithm::execute() {
  const int e = m_event++;
  if ( e >= m_entries ) return StatusCode::SUCCESS;

  for ( size_t t = 0; t < m_blocks.size(); ++t ) {
    auto& blk = *m_blocks[t];
    // Zero on some entries on purpose
    blk.n = ( e * 3 + static_cast<int>( t ) ) % ( m_maxArray + 1 );
    blk.m = ( e * 5 + 2 * static_cast<int>( t ) ) % ( m_maxArray + 1 );
    for ( size_t c = 0; c < blk.d.size(); ++c ) {
      const double v = 1000.0 * t + 10.0 * e + c;
      blk.d[c]       = v;
      blk.f[c]       = static_cast<float>( -v );
      blk.i[c]       = static_cast<int>( v );
      blk.b[c]       = ( ( e + c ) % 2 ) == 0;
      for ( int k = 0; k < blk.n; ++k ) {
        blk.var[c][k]    = v + 0.25 * k;
        blk.mat[c][k][0] = v + 0.5 * k;
        blk.mat[c][k][1] = v - 0.5 * k;
      }
      for ( int k = 0; k < blk.m; ++k ) blk.var2[c][k] = static_cast<int>( v ) - k;
      for ( int k = 0; k < 3; ++k ) blk.fixed[c][k] = static_cast<float>( v + k );
      for ( int k = 0; k < 2; ++k )
        for ( int l = 0; l < 3; ++l ) blk.fmat[c][k][l] = static_cast<float>( v + 10 * k + l );
    }
    if ( blk.tuple->write().isFailure() ) {
      error() << "cannot fill tuple " << t << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}
