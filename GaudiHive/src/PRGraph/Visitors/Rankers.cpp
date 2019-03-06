#include "Rankers.h"

namespace concurrency {

  //--------------------------------------------------------------------------
  bool RankerByProductConsumption::visit( AlgorithmNode& node ) {

    auto& products = node.getOutputDataNodes();
    float rank     = 0;

    for ( auto p : products ) rank += p->getConsumers().size();

    node.setRank( rank );
    /*std::stringstream s;
    s << node.getNodeName() << ", " << rank << "\n";
    std::ofstream myfile;
    myfile.open("AlgoRank.csv", std::ios::app);
    myfile << s.str();
    myfile.close();*/

    return true;
  }

  //--------------------------------------------------------------------------
  bool RankerByCummulativeOutDegree::visit( AlgorithmNode& node ) {

    std::ifstream myfile;
    myfile.open( "InputExecutionPlan.graphml", std::ios::in );

    precedence::PrecTrace execPlan;

    using boost::get;
    using precedence::AlgoTraceProps;

    boost::dynamic_properties dp;
    dp.property( "name", get( &AlgoTraceProps::m_name, execPlan ) );
    dp.property( "index", get( &AlgoTraceProps::m_index, execPlan ) );
    dp.property( "dataRank", get( &AlgoTraceProps::m_rank, execPlan ) );
    dp.property( "runtime", get( &AlgoTraceProps::m_runtime, execPlan ) );

    boost::read_graphml( myfile, execPlan, dp );

    typedef boost::graph_traits<precedence::PrecTrace>::vertex_iterator itV;
    std::pair<itV, itV>                                                 vp;

    for ( vp = boost::vertices( execPlan ); vp.first != vp.second; ++vp.first ) {
      precedence::AlgoTraceVertex v     = *vp.first;
      auto                        index = get( &AlgoTraceProps::m_name, execPlan );
      if ( index[v] == node.getNodeName() ) {
        runThroughAdjacents( v, execPlan );
        float rank = m_nodesSucceeded;
        node.setRank( rank );
        reset();
        // std::cout << "Rank of " << index[v] << " is " << rank << std::endl;
      }
    }

    return true;
  }

  //--------------------------------------------------------------------------
  void RankerByCummulativeOutDegree::runThroughAdjacents(
      boost::graph_traits<precedence::PrecTrace>::vertex_descriptor vertex, precedence::PrecTrace graph ) {
    typename boost::graph_traits<precedence::PrecTrace>::adjacency_iterator itVB;
    typename boost::graph_traits<precedence::PrecTrace>::adjacency_iterator itVE;

    for ( boost::tie( itVB, itVE ) = adjacent_vertices( vertex, graph ); itVB != itVE; ++itVB ) {
      m_nodesSucceeded += 1;
      runThroughAdjacents( *itVB, graph );
    }
  }

  //--------------------------------------------------------------------------
  bool RankerByTiming::visit( AlgorithmNode& node ) {

    std::ifstream myfile;
    myfile.open( "InputExecutionPlan.graphml", std::ios::in );

    precedence::PrecTrace execPlan;
    using boost::get;
    using precedence::AlgoTraceProps;

    boost::dynamic_properties dp;
    dp.property( "name", get( &AlgoTraceProps::m_name, execPlan ) );
    dp.property( "index", get( &AlgoTraceProps::m_index, execPlan ) );
    dp.property( "dataRank", get( &AlgoTraceProps::m_rank, execPlan ) );
    dp.property( "runtime", get( &AlgoTraceProps::m_runtime, execPlan ) );

    boost::read_graphml( myfile, execPlan, dp );

    typedef boost::graph_traits<precedence::PrecTrace>::vertex_iterator itV;
    std::pair<itV, itV>                                                 vp;

    for ( vp = boost::vertices( execPlan ); vp.first != vp.second; ++vp.first ) {
      precedence::AlgoTraceVertex v     = *vp.first;
      auto                        index = get( &AlgoTraceProps::m_name, execPlan );
      if ( index[v] == node.getNodeName() ) {
        auto  index_runtime = get( &AlgoTraceProps::m_runtime, execPlan );
        float rank          = index_runtime[v];
        node.setRank( rank );
        // std::cout << "Rank of " << index[v] << " is " << rank << std::endl;
      }
    }
    return true;
  }

  //--------------------------------------------------------------------------
  bool RankerByEccentricity::visit( AlgorithmNode& node ) {

    std::ifstream myfile;
    myfile.open( "Eccentricity.graphml", std::ios::in );

    precedence::PrecTrace execPlan;

    boost::dynamic_properties dp;
    using boost::get;

    dp.property( "name", get( &precedence::AlgoTraceProps::m_name, execPlan ) );
    dp.property( "Eccentricity", get( &precedence::AlgoTraceProps::m_eccentricity, execPlan ) );

    boost::read_graphml( myfile, execPlan, dp );

    typedef boost::graph_traits<precedence::PrecTrace>::vertex_iterator itV;
    std::pair<itV, itV>                                                 vp;

    for ( vp = boost::vertices( execPlan ); vp.first != vp.second; ++vp.first ) {
      precedence::AlgoTraceVertex v     = *vp.first;
      auto                        index = get( &precedence::AlgoTraceProps::m_name, execPlan );
      if ( index[v] == node.getNodeName() ) {
        auto  index_eccentricity = get( &precedence::AlgoTraceProps::m_eccentricity, execPlan );
        float rank               = index_eccentricity[v];
        node.setRank( rank );
        // std::cout << "Rank of " << index[v] << " is " << rank << std::endl;
      }
    }
    return true;
  }

  //--------------------------------------------------------------------------
  bool RankerByDataRealmEccentricity::visit( AlgorithmNode& node ) {

    // Find eccentricity of the node (only within the data realm of the execution flow graph)
    recursiveVisit( node );

    float rank = m_maxKnownDepth;
    node.setRank( rank );

    // Reset visitor for next nodes, if any
    reset();

    return true;
  }

  //--------------------------------------------------------------------------
  void RankerByDataRealmEccentricity::recursiveVisit( AlgorithmNode& node ) {

    m_currentDepth += 1;

    auto& products = node.getOutputDataNodes();

    if ( products.empty() )
      if ( ( m_currentDepth - 1 ) > m_maxKnownDepth ) m_maxKnownDepth = m_currentDepth - 1;

    for ( auto p : products )
      for ( auto algoNode : p->getConsumers() ) recursiveVisit( *algoNode );

    m_currentDepth -= 1;
  }

} // namespace concurrency
