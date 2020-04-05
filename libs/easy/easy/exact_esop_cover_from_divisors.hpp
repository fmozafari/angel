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
};

struct compute_esop_cover_from_divisors_statistics
{
};

struct compute_esop_cover_from_divisors_result_type
{
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
    (void)target;
    (void)divisor_functions;

    assert( false && "not implemented" );

    compute_esop_cover_from_divisors_result_type result;
    return result;
  }

private:
  compute_esop_cover_from_divisors_parameters const ps;
  compute_esop_cover_from_divisors_statistics& st;
};

std::optional<std::vector<easy::cube>> compute_exact_esop_cover_from_divisors( kitty::partial_truth_table const& target, std::vector<kitty::partial_truth_table> const& divisor_functions,
                                                                               compute_esop_cover_from_divisors_parameters const& ps,
                                                                               compute_esop_cover_from_divisors_statistics& st )
{
  return compute_esop_cover_from_divisors_impl( ps, st ).run( target, divisor_functions ).esop_cover;
}

std::optional<std::vector<easy::cube>> compute_exact_esop_cover_from_divisors( kitty::partial_truth_table const& target, std::vector<kitty::partial_truth_table> const& divisor_functions )
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
