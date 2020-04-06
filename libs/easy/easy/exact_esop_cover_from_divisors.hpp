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

#include <kitty/partial_truth_table.hpp>
#include <bill/sat/solver.hpp>
#include <bill/sat/tseytin.hpp>

namespace easy
{

struct compute_esop_cover_from_divisors_parameters
{
  /* maximum number of cubes */
  uint32_t max_num_cubes{4u};
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
      if ( kitty::is_const0( target ) )
      {
        result.esop_cover = std::vector{easy::cube()}; /* true */
        return result;
      }

      if ( kitty::is_const0( ~target ) )
      {
        result.esop_cover = std::vector<easy::cube>{}; /* false */
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
        else if ( target != divisor_functions[0u] )
        {
          std::vector<easy::cube> cover;
          easy::cube c;
          c.add_literal( 0u, false );
          cover.push_back( c );
          result.esop_cover = cover;
        }
      }
    }

    /* n ... number of variables */
    /* k ... number of cubes/product terms */
    uint32_t const n = divisor_functions.size();
    uint32_t const num_bits = target.num_bits();

    for ( auto k = 1u; k <= ps.max_num_cubes; ++k )
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
        *   |   .
        *   |   .
        *   |   .
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
        return bill::lit_type( bill::var_type( 2*n*k + (k-1)*l + j ), bill::lit_type::polarities::positive );
      };

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
        bill::add_xor_clause( solver, clause, bill::lit_type::polarities( kitty::get_bit( target, l ) ) );
      }

      switch( solver.solve() )
      {
      case bill::result::states::satisfiable:
        {
          auto const model = solver.get_model().model();
          std::vector<easy::cube> cover;

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
            cover.push_back( c );
          }

          result.esop_cover = cover;
          return result;
        }
        break;
      case bill::result::states::unsatisfiable:
        break;
      default:
        std::abort();
      }
    }
    return result;
  }

private:
  compute_esop_cover_from_divisors_parameters const ps;
  compute_esop_cover_from_divisors_statistics& st;
};

compute_esop_cover_from_divisors_result_type compute_exact_esop_cover_from_divisors( kitty::partial_truth_table const& target, std::vector<kitty::partial_truth_table> const& divisor_functions,
                                                                                     compute_esop_cover_from_divisors_parameters const& ps,
                                                                                     compute_esop_cover_from_divisors_statistics& st )
{
  return compute_esop_cover_from_divisors_impl( ps, st ).run( target, divisor_functions );
}

compute_esop_cover_from_divisors_result_type compute_exact_esop_cover_from_divisors( kitty::partial_truth_table const& target, std::vector<kitty::partial_truth_table> const& divisor_functions )
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
