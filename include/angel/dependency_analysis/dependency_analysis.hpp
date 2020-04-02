/* angel: C++ state preparation library
 * Copyright (C) 2018-2019  EPFL
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
  \file dependency_analysis.hpp

  \brief Dependency anaysis algorithms

  \author Fereshte Mozafari
  \author Heinz Riener
*/

namespace angel
{

struct dependency_analysis_algorithm_stats
{
  void report() const
  {
  }
};

enum class pattern_kind
{
  EQUAL = 1,
  XOR   = 2,
  AND   = 3,
};

struct dependency_analysis_result_type
{
  using fanins_type = std::vector<uint32_t>;
  using pattern_type = std::pair<pattern_kind, fanins_type>;

  /* maps an index to a dependency pattern, fanins are encoded as literals */
  std::map<uint32_t, pattern_type> dependencies;
}; /* dependency_analysis_result_type */

class dependency_analysis_algorithm_impl
{
public:
  using function_type = kitty::dynamic_truth_table;

public:
  explicit dependency_analysis_algorithm_impl( dependency_analysis_algorithm_stats &st )
    : st( st )
  {
  }

  dependency_analysis_result_type run( function_type const& function )
  {
    dependency_analysis_result_type result;

    /* TODO */

    return result;
  }

private:
  dependency_analysis_algorithm_stats &st;
}; /* dependency_analysis_algorithm_impl */

dependency_analysis_result_type compute_dependencies( kitty::dynamic_truth_table const &tt )
{
  dependency_analysis_algorithm_stats st;
  dependency_analysis_algorithm_impl da( st );

  auto const result = da.run( tt );

  st.report();

  return result;
}

} /* namespace angel */
