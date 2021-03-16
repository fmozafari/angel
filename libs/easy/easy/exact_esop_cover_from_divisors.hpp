/* easy: C++ ESOP library
 * Copyright (C) 2019-2020  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*!
  \file exact_esop_cover_from_divisors.hpp

  \brief Computes an exact ESOP cover from a divisor covering problem

  \author Heinz Riener
*/

#pragma once

#include "cubes.hpp"

#include <kitty/kitty.hpp>
#include <bill/sat/solver.hpp>
#include <bill/sat/tseytin.hpp>
#include <bill/sat/xor_clauses.hpp>
#include <algorithm>
#include <vector>
#include <optional>

namespace easy
{

struct compute_esop_cover_from_divisors_parameters
{
  /* maximum number of cubes: 0 means infinity */
  uint32_t max_num_cubes{0u};

  uint32_t num_conflicts{1000u};
};

struct compute_esop_cover_from_divisors_statistics
{
};

struct compute_esop_cover_from_divisors_result_type
{
  // empty cover means false
  // empty cube means true
  std::optional<std::vector<easy::cube>> esop_cover;
};

class compute_esop_cover_from_divisors_impl
{
public:
  explicit compute_esop_cover_from_divisors_impl( compute_esop_cover_from_divisors_parameters const& ps, compute_esop_cover_from_divisors_statistics& st )
    : ps( ps )
    , st( st )
  {
  }

  compute_esop_cover_from_divisors_result_type run( kitty::partial_truth_table const& target, std::vector<kitty::partial_truth_table> const& divisor_functions )
  {
    compute_esop_cover_from_divisors_result_type result;

    /* some special cases */
    {
      if ( kitty::is_const0( ~target ) )
      {
        result.esop_cover = std::vector{easy::cube()}; /* true */
        assert( result.esop_cover->size() == 1u );
        assert( (*result.esop_cover)[0].num_literals() == 0u );
        assert( cnot_cost( *result.esop_cover ) == 0u );
        return result;
      }

      if ( kitty::is_const0( target ) )
      {
        result.esop_cover = std::vector<easy::cube>{}; /* false */
        assert( result.esop_cover->size() == 0u );
        return result;
      }

      if ( divisor_functions.size() == 1u )
      {
        if ( target == divisor_functions[0u] )
        {
          std::vector<easy::cube> cover;
          easy::cube c;
          c.add_literal( 0u, true );
          cover.push_back( c );
          result.esop_cover = cover;
          return result;
        }
        else if ( target == ~divisor_functions[0u] )
        {
          std::vector<easy::cube> cover;
          easy::cube c;
          c.add_literal( 0u, false );
          cover.push_back( c );
          result.esop_cover = cover;
          return result;
        }
      }

      #if 1
      /* check for XOR */
      kitty::partial_truth_table xor_tt( target.num_bits() );
      for ( auto const& d : divisor_functions )
      {
        xor_tt = xor_tt ^ d;
      }

      if ( target == xor_tt )
      {
        std::vector<easy::cube> cover;
        for ( auto i = 0u; i < divisor_functions.size(); ++i )
        {
          easy::cube c;
          c.add_literal( i, true );
          cover.push_back( c );
        }
        result.esop_cover = cover;
        return result;
      }
      /* check for XNOR */
      else if ( ~target == xor_tt )
      {
        std::vector<easy::cube> cover{easy::cube()};
        for ( auto i = 0u; i < divisor_functions.size(); ++i )
        {
          easy::cube c;
          c.add_literal( i, true );
          cover.push_back( c );
        }
        result.esop_cover = cover;
        return result;
      }
#endif

    }

    /* n ... number of variables */
    /* k ... number of cubes/product terms */
    uint32_t const n = divisor_functions.size();
    uint32_t const num_bits = target.num_bits();

    uint32_t best_cost = std::numeric_limits<uint32_t>::max();
    for ( auto k = ps.max_num_cubes != 0 ? ps.max_num_cubes : divisor_functions.size(); k > 0u; --k )
    {
      // fmt::print( "[i] {}-term bounded ESOP synthesis for {}\n", k, kitty::to_binary( target ) );

      /* create a SAT solver */
      bill::solver<bill::solvers::glucose_41> solver;

      /* register 2*n*k p(i,j) and q(i,j) variables and additional k*num_bits auxiliary variables z(l,j) */
      solver.add_variables( 2*n*k + k*num_bits );

      /*  p- and q-variable layout:
       *  (i,j) ----------------------------------------------------------------------------------------------------------------------------------------------> j < n
       *    |     p(0,0)   = 0          q(0,0)   = 1               ...    p(0,n-1)   = 2*(n-1)               q(0,n-1)   = 2*(n-1) + 1
       *    |     p(1,0)   = 2*n        q(1,0)   = 2*n + 1         ...    p(1,n-1)   = 2*n + 2*(n-1)         q(1,n-1)   = 2*n + 2*(n-1) + 1
       *    |     .                     .                                 .                                  .
       *    |     .                     .                                 .                                  .
       *    |     .                     .                                 .                                  .
       *    |     p(k-1,0) = 2*n*(k-1)  q(k-1,0) = 2*n*(k-1) + 1   ...    p(k-1,n-1) = 2*n*(k-1) + 2*(n-1)   q(k-1,n-1) = 2*n*(k-1) + 2*(n-1) + 1
       *    v
       *  i < k
       *
       *  2*n*k elements: q(0,0) == 0, ..., q(k-1,n-1) == 2*n*k - 1
       */

       /* z-variable layout:
        * (l,j) ---------------------------------------------------------------------------------------------------------------------------------------------> j < k
        *   |   z(0,0)          = 2*n*k                z(0,1)          = 2*n*k + 1                         ...   z(0,k-1)          = 2*n*k + k-1 
        *   |   z(1,0)          = (2*n+1)*k            z(1,1)          = (2*n+1)*k + 1                     ...   z(1,k-1)          = (2*n+1)*k + k-1 
        *   |   .                 .                    .                                                         .
        *   |   .                 .                    .                                                         .
        *   |   .                 .                    .                                                         .
        *   |   z(num_bits-1,0) = (2*n+num_bits-1)*k   z(num_bits-1,1) = (2*n+num_bits-1)*k + 1            ...   z(num_bits-1,k-1) = (2*n+num_bits-1)*k + k-1 
        *   v
        * l < num_bits
        *
        * k*num_bits elements: z(0,0) == 2*n*k, ..., z(num_bits-1,k-1) == 2*n*k + k*num_bits - 1
        */

      auto const p = [&]( uint32_t i, uint32_t j ){
        assert( i < k ); /* cube */
        assert( j < n ); /* variable */
        return bill::lit_type( bill::var_type( 2*n*i + 2*j ), bill::lit_type::polarities::positive );
      };

      auto const q = [&]( uint32_t i, uint32_t j ){
        assert( i < k ); /* cube */
        assert( j < n ); /* variable */
        return bill::lit_type( bill::var_type( 2*n*i + 2*j + 1 ), bill::lit_type::polarities::positive );
      };

      auto const z = [&]( uint32_t l, uint32_t j ){
        assert( l < num_bits ); /* minterm */
        assert( j < k ); /* cube */
        return bill::lit_type( bill::var_type( 2*n*k + k*l + j ), bill::lit_type::polarities::positive );
      };

      /* print layout */
      // for ( auto i = 0u; i < k; ++i )
      // {
      //   for ( auto j = 0u; j < n; ++j )
      //   {
      //     fmt::print( "p({},{}) = {}\n", i, j, uint32_t( p( i, j ).variable() ) );
      //   }
      // }
      //
      // for ( auto i = 0u; i < k; ++i )
      // {
      //   for ( auto j = 0u; j < n; ++j )
      //   {
      //     fmt::print( "q({},{}) = {}\n", i, j, uint32_t( q( i, j ).variable() ) );
      //   }
      // }
      //
      // for ( auto l = 0u; l < num_bits; ++l )
      // {
      //   for ( auto j = 0u; j < k; ++j )
      //   {
      //     fmt::print( "z({},{}) = {}\n", l, j, uint32_t( z( l, j ).variable() ) );
      //   }
      // }

      /* add constraints */
      for ( auto l = 0u; l < num_bits; ++l ) /* for each row in the divisors */
      {
        /* positive */
        for ( auto i = 0u; i < k; ++i ) /* for each ESOP cube */
        {
          for ( auto j = 0u; j < n; ++j ) /* for each variable */
          {
            std::vector<bill::lit_type> clause = { ~z( l, i ) };
            clause.push_back( kitty::get_bit( divisor_functions[j], l ) ? ~q( i, j ) : ~p( i, j ) );
            solver.add_clause( clause );
          }
        }

        /* negative */
        for ( auto i = 0u; i < k; ++i ) /* for each ESOP cube */
        {
          std::vector<bill::lit_type> clause = { z( l, i ) };
          for ( auto j = 0u; j < n; ++j ) /* for each variable */
          {
            clause.push_back( kitty::get_bit( divisor_functions[j], l ) ? q( i, j ) : p( i, j ) );
          }
          solver.add_clause( clause );
        }

        /* consider target function */
        std::vector<bill::lit_type> clause;
        for ( auto i = 0u; i < k; ++i ) /* for each ESOP cube */
        {
          clause.push_back( z( l, i ) );
        }
        solver.add_clause( bill::add_xor_clause( solver, clause, bill::lit_type::polarities( !kitty::get_bit( target, l ) ) ) );
      }

      /* at most one cube is allowed to be empty */
      for ( auto i = 0u; i < k-1; ++i ) // for each ESOP cube
      {
        std::vector<bill::lit_type> clause;
        for ( auto j = 0u; j < n; ++j ) /* for each variable */
        {
          clause.push_back( bill::add_tseytin_xor( solver, p( i, j ), q( i, j ) ) );
        }
        solver.add_clause( clause );
      }

      switch ( solver.solve( {}, ps.num_conflicts ) )
      {
      case bill::result::states::satisfiable:
        {
          auto const model = solver.get_model().model();
          auto cover = esop_cover_from_model( model, n, k );
          auto const cost = cnot_cost( cover );
          if ( cost < best_cost )
          {
            best_cost = cost;
            result.esop_cover = cover;
          }
          // print_cover( cover, n, cost, best_cost );

          /* try to minimize literals */
          std::vector<bill::lit_type> lits;
          for ( auto i = 0u; i < k; ++i ) /* for each ESOP cube */
          {
            for ( auto j = 0u; j < n; ++j ) /* for each variable */
            {
              auto const p_value = model[2*n*i + 2*j] == bill::lbool_type::true_;
              auto const q_value = model[2*n*i + 2*j + 1] == bill::lbool_type::true_;

              if ( p_value ^ q_value )
              {
                if ( p_value )
                {
                  lits.push_back( p( i, j ) );
                }
                else if ( q_value )
                {
                  lits.push_back( q( i, j ) );
                }
              }
            }
          }

          std::reverse( std::begin( lits ), std::end( lits ) );

          for ( auto const& l : lits )
          {
            if ( solver.solve( { ~l }, ps.num_conflicts ) == bill::result::states::satisfiable )
            {
              auto const model = solver.get_model().model();

              auto cover = esop_cover_from_model( model, n, k );
              auto const cost = cnot_cost( cover );
              if ( cost < best_cost )
              {
                best_cost = cost;
                result.esop_cover = cover;
              }
              // print_cover( cover, n, cost, best_cost );
            }
          }

          continue;
        }
        break;
      case bill::result::states::unsatisfiable:
      case bill::result::states::undefined:
        return result;
      default:
        std::abort();
      }
    }
    return result;
  }

private:
  void print_cover( std::vector<easy::cube> const& esop, uint32_t num_vars, uint32_t cost, uint32_t best_cost ) const
  {
    for ( const auto& cube : esop )
    {
      cube.print( num_vars );
      std::cout << ' ';
    }
    std::cout << "cost = " << cost << " best cost = " << best_cost << std::endl;
  }

  std::vector<easy::cube> esop_cover_from_model( bill::result::model_type const& model, uint32_t n, uint32_t k )
  {
    std::vector<easy::cube> cover;

    uint32_t empty_cube_counter = 0u;
    for ( auto i = 0u; i < k; ++i ) /* for each ESOP cube */
    {
      easy::cube c;
      bool cancel_cube = false;

      for ( auto j = 0u; j < n; ++j ) /* for each variable */
      {
        auto const p_value = model[2*n*i + 2*j] == bill::lbool_type::true_;
        auto const q_value = model[2*n*i + 2*j + 1] == bill::lbool_type::true_;

        if ( p_value && q_value )
        {
          cancel_cube = true;
        }
        else if ( p_value )
        {
          c.add_literal( j, true );
        }
        else if ( q_value )
        {
          c.add_literal( j, false );
        }
      }

      if ( cancel_cube )
      {
        continue;
      }
      else if ( c.num_literals() == 0u )
      {
        ++empty_cube_counter;
        continue;
      }
      cover.push_back( c );
    }

    /* add at most one empty cube for negation */
    if ( ( empty_cube_counter % 2 ) == 1u )
    {
      cover.push_back( easy::cube() );
    }

    return cover;
  }

  uint32_t cnot_cost( easy::cube const& c, bool is_first = false ) const
  {
    auto const num_literals = c.num_literals();
    switch ( num_literals )
    {
    case 0:
      return 0u;
    case 1:
      return 1;
    default:
      if ( is_first )
        return ( 1 << num_literals );
      else
        return ( 1 << ( num_literals + 1 ) ) - 2u;
    }
  }

  uint32_t cnot_cost (std::vector<easy::cube>& esop) const
  {
    if ( esop.size() == 0u )
    {
      return 0u;
    }

    uint32_t first_cube = 0u;
    uint32_t best_cost_savings = 0u;
    for ( auto i = 0u; i < esop.size(); ++i )
    {
      uint32_t const cost_a = cnot_cost( esop[i], false );
      uint32_t const cost_b = cnot_cost( esop[i], true );
      uint32_t const cost_savings = cost_a > cost_b ? cost_a - cost_b : cost_b - cost_a;
      if ( cost_savings > best_cost_savings )
      {
        first_cube = i;
        best_cost_savings = cost_savings;
      }
    }

    /* reorder ESOP cubes */
    if ( first_cube != 0u )
      std::swap( esop[0u], esop[first_cube] );

    uint32_t cost1 = cnot_cost_mctoffoli(esop);
    auto cost2 = cnot_cost_uniformly(esop);
    if(cost2.first)
      return (cost1<cost2.second) ? cost1 : cost2.second;
    else
      return cost1;
  }

  uint32_t cnot_cost_mctoffoli( std::vector<easy::cube>& esop ) const
  {
    
    auto cost = 0u;
    cost += cnot_cost( esop[0], true );
    for ( auto i = 1u; i < esop.size(); ++i )
    {
      cost += cnot_cost( esop[i], false );
    }

    return cost;
  }

  bool is_mask_subset(easy::cube const& a, easy::cube const& b) const
  {
    easy::cube aflip;
    aflip._mask = a._mask ^ 0xffffffff;
    easy::cube new_cube;
    new_cube._mask = aflip._mask & b._mask;
    return (__builtin_popcount( new_cube._mask )==0u);
  }

  std::pair<bool, uint32_t> cnot_cost_uniformly( std::vector<easy::cube>& esop ) const
  {
    for(auto i=1u; i<esop.size(); i++)
    {
      if(!is_mask_subset(esop[0], esop[i]))
        return std::make_pair(false, 0);
    }
    uint32_t cost = pow(2, esop[0].num_literals());
    return std::make_pair(true, cost);
  }

private:
  compute_esop_cover_from_divisors_parameters const ps;
  compute_esop_cover_from_divisors_statistics& st;
};

inline compute_esop_cover_from_divisors_result_type compute_exact_esop_cover_from_divisors( kitty::partial_truth_table const& target, std::vector<kitty::partial_truth_table> const& divisor_functions,
                                                                                     compute_esop_cover_from_divisors_parameters const& ps,
                                                                                     compute_esop_cover_from_divisors_statistics& st )
{
  return compute_esop_cover_from_divisors_impl( ps, st ).run( target, divisor_functions );
}

inline compute_esop_cover_from_divisors_result_type compute_exact_esop_cover_from_divisors( kitty::partial_truth_table const& target, std::vector<kitty::partial_truth_table> const& divisor_functions )
{
  compute_esop_cover_from_divisors_parameters ps;
  compute_esop_cover_from_divisors_statistics st;
  return compute_exact_esop_cover_from_divisors( target, divisor_functions, ps, st );
}

} // namespace easy

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
