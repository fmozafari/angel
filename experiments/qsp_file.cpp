#include <angel/angel.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

#include "experiments.hpp"

void run_experiments( std::ofstream& ofs, std::vector<std::string> const& benchmarks, std::string const& name, angel::function_extractor_params ps = {} )
{
  angel::function_extractor extractor{ps};
  for ( const auto &benchmark : benchmarks )
  {
    fmt::print( "[i] processing {}\n", benchmark );
    if ( !extractor.parse( experiments::benchmark_path( benchmark ) ) )
      continue;

    extractor.run( [&]( kitty::dynamic_truth_table const &tt ){
        ofs << kitty::to_binary( tt ) << '\n';
      });
  }
}

int main()
{
  
  for ( auto i = 10u; i < 11u; ++i )
  {
    std::string const filename1 = fmt::format("qsp_cut_functions_EPFL_{}.txt", i);
    std::ofstream ofs_EPFL( filename1 );
    fmt::print( "[i] run experiments for {}-input cut functions\n", i );
    run_experiments( ofs_EPFL, experiments::epfl_benchmarks( ~experiments::epfl::hyp ), fmt::format( "EPFL benchmarks {}", i ),
                     {.num_vars = i} );
                    
    std::string const filename2 = fmt::format("qsp_cut_functions_ISCAS_{}.txt", i);
    std::ofstream ofs_ISCAS( filename2 );
    run_experiments( ofs_ISCAS, experiments::iscas_benchmarks(), fmt::format( "ISCAS benchmarks {}", i ),
                     {.num_vars = i} );

    std::string const filename3 = fmt::format("equal_functions_{}.txt", i);
    std::ofstream ofs_k_equal( filename3 );
    kitty::dynamic_truth_table tt( i );
    for ( auto j = 0; j < i; ++j )
    {
      kitty::create_equals( tt, j );
      ofs_k_equal << kitty::to_binary( tt ) << '\n';
    }

    ofs_EPFL.close();
    ofs_ISCAS.close();
    ofs_k_equal.close();
  }
  
  return 0;
