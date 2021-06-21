#pragma once
#define _USE_MATH_DEFINES
#include <cmath> 
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Operators/All.h>
#include <vector>

namespace angel
{

using QCgates_dd_t = std::vector<std::vector<std::pair<double, std::vector<uint32_t>>>>;

template<class Network>
inline void create_qc_forBDD( Network& qc, QCgates_dd_t gates )
{
  std::vector<tweedledum::Qubit> q;
  std::vector<tweedledum::Cbit> c;
  for(auto i=0u; i<gates.size(); i++)
  {
    q.push_back(qc.create_qubit());
    c.push_back(qc.create_cbit());
    if (i==0)
      qc.apply_operator(tweedledum::Op::Ry(gates[i][0].first), {q[i]});

    else if(i==1)
    {
      for(auto g : gates[i])
      { 
        if(g.first == 0) /* inserting CNOT */
          qc.apply_operator(tweedledum::Op::X(), {q[g.second[0]], q[i]});
        else
        {
          double angle = 2 * acos( sqrt( g.first ) );
          std::vector<tweedledum::Qubit> qlines;
          for(auto i=0u; i<g.second.size(); i++) /* insert control qubits */
          {
            auto idx = g.second[i];
            if(idx%2 == 0u)
              qlines.push_back(q[idx/2]);
            else
              qlines.push_back(!q[idx/2]);
          }
          qlines.push_back(q[i]); /* insert target qubit */
          qc.apply_operator(tweedledum::Op::Ry(angle), qlines);
        }  
      }
    }
    else 
    {
      for(auto g : gates[i])
      {
        double angle = 2 * acos( sqrt( g.first ) );
        std::vector<tweedledum::Qubit> qlines;
        for(auto i=0u; i<g.second.size(); i++) /* insert control qubits */
        {
          auto idx = g.second[i];
            if(idx%2 == 0u)
              qlines.push_back(q[idx/2]);
            else
              qlines.push_back(!q[idx/2]);
        }
        qlines.push_back(q[i]); /* insert targt qubit */
        qc.apply_operator(tweedledum::Op::Ry(angle), qlines);
      }
    }

  }
}


} // namespace angel