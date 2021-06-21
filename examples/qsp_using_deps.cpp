#include <angel/angel.hpp>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/IR/Instruction.h>
#include <kitty/kitty.hpp>
#include <iostream>
#include<fstream>

using namespace angel;

int main()
{
    tweedledum::Circuit ntk;
    std::string truth_table{"1100" "0000" "0011" "0111"};
    //std::string truth_table{"10000001"};
    //std::ifstream file_inp;
    //file_inp.open("../examples/test.txt");
    //std::string truth_table;
    //file_inp>> truth_table;
    
    int32_t const num_vars = std::log2( truth_table.size() );
    kitty::dynamic_truth_table tt(num_vars);
    kitty::create_from_binary_string(tt, truth_table);
    kitty::print_binary(tt);
    std::cout<<std::endl;

    typename angel::pattern_deps_analysis::parameter_type pattern_ps;
    typename angel::pattern_deps_analysis::statistics_type pattern_st;
    angel::pattern_deps_analysis pattern( pattern_ps, pattern_st );

    typename angel::esop_deps_analysis::parameter_type esop_ps;
    typename angel::esop_deps_analysis::statistics_type esop_st;
    angel::esop_deps_analysis esop( esop_ps, esop_st );

    typename angel::no_deps_analysis::parameter_type no_dependencies_ps;
    typename angel::no_deps_analysis::statistics_type no_dependencies_st;
    angel::no_deps_analysis no_deps( no_dependencies_ps, no_dependencies_st );
    
  /* reordering strategies */
    angel::no_reordering no_reorder;
    angel::exhaustive_reordering all_orders;
    angel::random_reordering random( 0xcafeaffe, num_vars > 1u ? ( num_vars * num_vars ) : 1u );

    angel::state_preparation_parameters qsp_ps;
    angel::state_preparation_statistics qsp_st;
    angel::qsp_deps<decltype(ntk), decltype( esop ), decltype( random )> qsp( ntk, esop, random, qsp_ps, qsp_st );
    qsp(tt);
    qsp_st.report();
    
    return 0;
}