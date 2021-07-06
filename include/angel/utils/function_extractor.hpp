/* angel
 * Copyright (C) 2019  EPFL
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
  \file function_extractor.hpp
  \brief Function extractor

  \author Heinz Riener
*/

#pragma once

#include <kitty/kitty.hpp>
#include <lorina/lorina.hpp>
#include <mockturtle/mockturtle.hpp>

namespace angel
{

struct function_extractor_params
{
  uint32_t num_vars = 6u;

  /* skip cuts with less leaves than num_vars */
  bool exact_size = true;
}; /* function_extractor_params */

class function_extractor
{
public:
  explicit function_extractor( function_extractor_params ps = {} )
      : ps( ps )
  {
  }

  bool parse( std::string const& filename )
  {
    lorina::diagnostic_engine diag;
    return ( lorina::read_aiger( filename, mockturtle::aiger_reader( aig ), &diag ) == lorina::return_code::success );
  }

  template<typename Fn>
  void run( Fn&& fn )
  {
    mockturtle::depth_view depth_aig( aig );
    mockturtle::fanout_view fanout_aig( depth_aig );

    mockturtle::cut_manager<decltype( fanout_aig )> mgr( ps.num_vars );
    fanout_aig.foreach_gate( [&]( const auto& n )
                             {
                               auto const leaves = mockturtle::reconv_driven_cut( mgr, fanout_aig, n );

                               mockturtle::cut_view cut{ fanout_aig, leaves, aig.make_signal( n ) };

                               mockturtle::default_simulator<kitty::dynamic_truth_table> sim( leaves.size() );
                               if ( ps.exact_size && leaves.size() != ps.num_vars )
                                 return;

                               auto const result = mockturtle::simulate_nodes<kitty::dynamic_truth_table>( cut, sim );

                               cut.foreach_po( [&]( const auto& s )
                                               {
                                                 auto const tt = result[cut.get_node( s )];
                                                 auto const it = tts.find( tt );
                                                 if ( it == std::end( tts ) )
                                                 {
                                                   tts.insert( tt );
                                                   fn( tt );
                                                 }
                                                 else
                                                 {
                                                   kitty::dynamic_truth_table tt_( tt.num_vars() );
                                                   std::string truth_table = "0";
                                                   for ( auto i = 1u; i < pow( 2, ( tt.num_vars() ) ); i++ )
                                                     truth_table += '0';
                                                   kitty::create_from_binary_string( tt_, truth_table );

                                                   fn( tt_ );
                                                 }
                                               } );
                             } );
  }

protected:
  function_extractor_params ps;

  mockturtle::aig_network aig;
  std::set<kitty::dynamic_truth_table> tts;
}; /* function_extractor */

} // namespace angel
