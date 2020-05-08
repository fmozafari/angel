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
  std::string const filename = "qsp_cut_functions.txt";
  std::ofstream ofs( filename );
  for ( auto i = 4u; i <= 6u; ++i )
  {
    fmt::print( "[i] run experiments for {}-input cut functions\n", i );
    run_experiments( ofs, experiments::epfl_benchmarks( ~experiments::epfl::hyp ), fmt::format( "EPFL benchmarks {}", i ),
                     {.num_vars = i} );
    run_experiments( ofs, experiments::iscas_benchmarks(), fmt::format( "ISCAS benchmarks {}", i ),
                     {.num_vars = i} );
  }
  ofs.close();
  return 0;
}
