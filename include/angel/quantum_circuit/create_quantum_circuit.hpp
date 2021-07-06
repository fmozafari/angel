#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Operators/All.h>
#include <vector>

namespace angel
{

using MC_gates_t = std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>;

template<class Network>
inline void create_qc_for_MCgates( Network& qc, MC_gates_t gates, std::vector<uint32_t> order )
{
  std::vector<tweedledum::Qubit> q;
  std::vector<tweedledum::Cbit> c;

  /* create qubits and cbits */
  for ( auto i = 0u; i < order.size(); i++ )
  {
    q.push_back( qc.create_qubit() );
    c.push_back( qc.create_cbit() );
  }

  std::reverse( order.begin(), order.end() );

  /* Last element in order shows MSB */
  for ( int32_t i = order.size() - 1; i >= 0; i-- )
  {
    if ( i == ( order.size() - 1 ) && gates[i].size() != 0 )
    {
      qc.apply_operator( tweedledum::Op::Ry( gates[i][0].first ), { q[order[i]] } );
    }
    else
    {
      for ( auto g : gates[i] )
      {
        double angle = g.first;
        std::vector<tweedledum::Qubit> qlines;
        for ( auto j = 0u; j < g.second.size(); j++ ) /* insert control qubits */
        {
          auto idx = g.second[j];
          if ( idx % 2 == 0u )
            qlines.push_back( q[order[idx / 2]] );
          else
            qlines.push_back( !q[order[idx / 2]] );
        }
  
        qlines.push_back( q[order[i]] ); /* insert targt qubit */
        if ( angle == M_PI )
          qc.apply_operator( tweedledum::Op::X(), qlines );
        else
          qc.apply_operator( tweedledum::Op::Ry( angle ), qlines );
      }
    }
  }
}

} // namespace angel