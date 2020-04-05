/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "interface/types.hpp"
#include <vector>
#include <queue>

namespace bill {

template<typename Solver>
lit_type add_tseytin_and(Solver& solver, lit_type const& a, lit_type const& b)
{
	auto const r = solver.add_variable();
	solver.add_clause(std::vector{~a, ~b, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{a, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{b, lit_type(r, lit_type::polarities::negative)});
	return lit_type(r, lit_type::polarities::positive);
}

template<typename Solver>
lit_type add_tseytin_and(Solver& solver, std::vector<lit_type> const& ls)
{
	auto const r = solver.add_variable();
	std::vector<lit_type> cls;
	for (const auto& l : ls)
		cls.emplace_back(~l);
	cls.emplace_back(lit_type(r, lit_type::polarities::positive));
	solver.add_clause(cls);
	for (const auto& l : ls)
		solver.add_clause(std::vector{l, lit_type(r, lit_type::polarities::negative)});
	return lit_type(r, lit_type::polarities::positive);
}

template<typename Solver>
lit_type add_tseytin_or(Solver& solver, lit_type const& a, lit_type const& b)
{
	auto const r = solver.add_variable();
	solver.add_clause(std::vector{a, b, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{~a, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{~b, lit_type(r, lit_type::polarities::positive)});
	return lit_type(r, lit_type::polarities::positive);
}

template<typename Solver>
lit_type add_tseytin_or(Solver& solver, std::vector<lit_type> const& ls)
{
	auto const r = solver.add_variable();
	std::vector<lit_type> cls(ls);
	cls.emplace_back(lit_type(r, lit_type::polarities::negative));
	solver.add_clause(cls);
	for (const auto& l : ls)
		solver.add_clause(std::vector{~l, lit_type(r, lit_type::polarities::positive)});
	return lit_type(r, lit_type::polarities::positive);
}

template<typename Solver>
lit_type add_tseytin_xor(Solver& solver, lit_type const& a, lit_type const& b)
{
	auto const r = solver.add_variable();
	solver.add_clause(std::vector{~a, ~b, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{~a, b, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{a, ~b, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{a, b, lit_type(r, lit_type::polarities::negative)});
	return lit_type(r, lit_type::polarities::positive);
}

template<typename Solver>
lit_type add_tseytin_equals(Solver& solver, lit_type const& a, lit_type const& b)
{
	auto const r = solver.add_variable();
	solver.add_clause(std::vector{~a, ~b, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{~a, b, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{a, ~b, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{a, b, lit_type(r, lit_type::polarities::positive)});
	return lit_type(r, lit_type::polarities::positive);
}

template<typename Solver>
lit_type add_xor_clause(Solver& solver, std::vector<lit_type> const& clause, lit_type::polarities pol = lit_type::polarities::positive )
{
  std::queue<lit_type> lits;
  bool first = pol == lit_type::polarities::negative;
  for ( const auto& l : clause )
  {
    if ( first )
    {
      lits.push( ~l );
      first = false;
    }
    else
    {
      lits.push( l );
    }
  }

  while ( lits.size() > 1 )
  {
    auto const a = lits.front();
    lits.pop();
    auto const b = lits.front();
    lits.pop();

    lits.push( add_tseytin_xor( solver, a, b ) );
  }

  assert( lits.size() == 1u );
  solver.add_clause( { lits.front() } );

  return lits.front();
}

} /* namespace bill */
