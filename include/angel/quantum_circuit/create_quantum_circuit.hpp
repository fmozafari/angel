#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Operators/All.h>
#include <vector>

namespace angel
{

struct target_qubit
{
  uint32_t index;
  enum gType {NOT, Ry};
  gType gt;
  double angle;
};

using MC_gates_t = std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>;
using gates_sqs_t = std::vector<std::pair<target_qubit, std::vector<uint32_t>>>;

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

template<class Network>
inline void create_qc_for_sparse_uqsp( Network& qc, gates_sqs_t gates, uint32_t q_count, uint64_t & cnot_count)
{
  std::vector<tweedledum::Qubit> q;
  std::vector<tweedledum::Cbit> c;

  /* create qubits and cbits */
  for ( auto i = 0u; i < q_count; i++ )
  {
    q.push_back( qc.create_qubit() );
    c.push_back( qc.create_cbit() );
  }

  qc.apply_operator( tweedledum::Op::X(), {q[q_count-1]} );
  gates.pop_back(); /* last computation for ancilla qubit doesn't need
  */
  for(auto [target, controls]: gates)
  {
    std::vector<tweedledum::Qubit> qlines;
    for ( auto j = 0u; j < controls.size(); j++ ) /* insert control qubits */
    {
      auto idx = controls[j];
      if ( idx % 2 == 0u )
        qlines.push_back( q[idx / 2] );
      else
        qlines.push_back( !q[idx / 2] );
    }
    qlines.push_back( q[target.index] ); /* insert targt qubit */

    if(target.gt == target_qubit::NOT)
      qc.apply_operator( tweedledum::Op::X(), qlines );
    else
      qc.apply_operator( tweedledum::Op::Ry( target.angle ), qlines );

    if(controls.size()==1 && (target.gt == target_qubit::NOT))
      cnot_count += 1;
    else if(controls.size()==0)
      cnot_count += 0;
    else if(controls.size()==1)
      cnot_count += 2;
    else if(controls.size()==2)
      cnot_count += 6;
    else if(controls.size()==3)
      cnot_count += (8*6+2);
    else if(controls.size()==4)
      cnot_count += 10*6;
    else
    {
      auto c_part1 = floor(controls.size()/2);
      auto c_part2 = controls.size() - c_part1;
      auto count1 = 2 * (2*(c_part1-1)+2*(c_part1-2));
      auto count2 = 2 * (2*(c_part2-1)+2*(c_part2-2));
      cnot_count += (count1+count2);
    }
    
  }

}

} // namespace angel