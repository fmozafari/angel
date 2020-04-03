/* angel: C++ state preparation library
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
  \file dependency_analysis2.hpp

  \brief Dependency analysis algorithm

  \author Fereshte Mozafari
  \author Heinz Riener
*/

#pragma once

#include "common.hpp"

#include <kitty/implicant.hpp>
#include <kitty/partial_truth_table.hpp>

#include <fmt/format.h>

#include <map>
#include <vector>

namespace angel
{

struct dependency_analysis2_params
{
};

struct dependency_analysis2_stats
{
  void report() const
  {
  }

  void reset()
  {
  }
};

struct dependency_analysis2_result_type
{
};

class dependency_analysis2_impl
{
public:
  explicit dependency_analysis2_impl( dependency_analysis2_params& const ps, dependency_analysis2_stats& st )
    : ps( ps )
    , st( st )
  {
  }

  dependency_analysis2_result_type run( function_type const& function )
  {
    dependency_analysis2_result_type result;
    return result;
  }

  dependency_analysis2_params& const ps;
  dependency_analysis2_stats& st
};

dependency_analysis2_result_type compute_dependencies( kitty::dynamic_truth_table const &tt, dependency_analysis_params const& ps, dependency_analysis_stats& st )
{
  return dependency_analysis2_impl( ps, st ).run( tt );
}

} /* namespace angel */
