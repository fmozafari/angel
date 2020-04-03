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
  \file common.hpp

  \brief Common datastructures used by dependency analysis algorithms

  \author Fereshte Mozafari
  \author Heinz Riener
*/

#pragma once

#include <kitty/partial_truth_table.hpp>

#include <fmt/format.h>

#include <vector>

namespace angel
{

struct dependency_analysis_types
{
  /* pattern */
  enum class pattern_kind
  {
    /* unary */
    EQUAL = 1,
    /* nary */
    XOR   = 2,
    XNOR  = 3,
    AND   = 4,
    NAND  = 5,
  };

  using fanins = std::vector<uint32_t>;
  using pattern = std::pair<pattern_kind, fanins>;

  /* column */
  struct column
  {
    kitty::partial_truth_table tt;
    uint32_t index;
  };

  static std::string pattern_kind_string( pattern_kind const kind )
  {
    switch( kind )
    {
    case pattern_kind::EQUAL:
      return "EQUAL";
    case pattern_kind::XOR:
      return "XOR";
    case pattern_kind::XNOR:
      return "~XOR";
    case pattern_kind::AND:
      return "AND";
    case pattern_kind::NAND:
      return "~AND";
    default:
      std::abort();
    }
  }

  static std::string pattern_string( pattern const& p )
  {
    std::string args;
    for ( const auto& a : p.second )
    {
      args += fmt::format( "{} ", a );
    }
    return fmt::format( "{}( {})", pattern_kind_string( p.first ), args );
  }
}; /* dependency_analysis_types */

} /* namespace angel */
