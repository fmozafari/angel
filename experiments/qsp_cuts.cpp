#include <angel/utils/function_extractor.hpp>

#include "experiments.hpp"

int main()
{
  experiments::experiment<std::string, uint32_t> exp( "qsp_cuts", "benchmark", "#functions" );

  for ( const auto& benchmark : experiments::epfl_benchmarks( ~experiments::hyp ) )
  {
    fmt::print( "[i] processing {}\n", benchmark );
    
    angel::function_extractor extractor{experiments::benchmark_path( benchmark )};
    auto const success = extractor.parse();
    if ( !success )
      continue;

    uint32_t function_counter = 0u;
    extractor.run( [&]( kitty::dynamic_truth_table const& tt ){
        (void) tt;
        ++function_counter;
        // std::cout << tt.num_vars() << ' '; kitty::print_hex( tt ); std::cout << std::endl;
      });

    exp( benchmark, function_counter );    
  }

  exp.save();
  exp.table();
  
  return 0;
}
