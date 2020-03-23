#include <angel/utils/function_extractor.hpp>

#include <angel/quantum_state_preparation/qsp_tt_general.hpp>
#include <angel/reordering/random_reordering.hpp>
#include <angel/reordering/all_reordering.hpp>
#include <angel/reordering/no_reordering.hpp>
#include <angel/reordering/considering_deps_reordering.hpp>
#include <angel/dependency_analysis/resub_synthesis.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <angel/utils/stopwatch.hpp>

#include "experiments.hpp"

int main()
{
    experiments::experiment<std::string, uint32_t, double, double, double, double> exp("qsp_cuts", "benchmark", "#functions",
                                                                               "Avg-CNOTs-base", "Avg-CNOTs-deps-1randOrder", "Avg-CNOTs-deps-allOrders", "time-allOrders");

    for (const auto &benchmark : experiments::epfl_benchmarks(~experiments::hyp))
    {
        fmt::print("[i] processing {}\n", benchmark);
        angel::function_extractor_params ps;
        ps.num_vars = 6;
        angel::function_extractor extractor{experiments::benchmark_path(benchmark), ps};
        auto const success = extractor.parse();
        if (!success)
            continue;

        //if(benchmark!="sin")
            //continue;

        uint32_t function_counter = 0u;
        angel::qsp_general_stats qsp_stats1;
        angel::qsp_general_stats qsp_stats2;
        angel::qsp_general_stats qsp_stats3;
        extractor.run([&](kitty::dynamic_truth_table const &tt) {
            (void)tt;
            if (tt.num_vars() == 6)
            {
                // if(benchmark=="log2")
                // {
                //     kitty::print_binary(tt);
                //     std::cout<<std::endl;
                // }
                ++function_counter;
                //std::cout << tt.num_vars() << ' '; kitty::print_hex( tt ); std::cout << std::endl;

                if(function_counter<250)
                {
                    //kitty::print_binary(tt);
                    //std::cout<<std::endl;
                    tweedledum::netlist<tweedledum::mcmt_gate> ntk;
                    angel::NoDeps deps_alg1;
                    angel::NoReordering orders1;
                    angel::qsp_tt_general(ntk, deps_alg1, orders1, tt, qsp_stats1);
                    angel::ResubSynthesisDeps deps_alg2;
                    angel::RandomReordering orders2(5);
                    angel::qsp_tt_general(ntk, deps_alg2, orders2, tt, qsp_stats2);
                    angel::ResubSynthesisDeps deps_alg3;
                    //angel::RandomReordering orders3(20);
                    angel::ConsideringDepsReordering orders3(5);
                    angel::qsp_tt_general(ntk, deps_alg3, orders3, tt, qsp_stats3);
                }
            }
        });
        //qsp_stats.report();

        auto sum_f = 0; /// CNOTs: first
        for (auto n = 0u; n < qsp_stats1.gates_count.size(); n++)
        {
            sum_f += qsp_stats1.gates_count[n].first;
        }
        double avg_f1 = sum_f / double(qsp_stats1.gates_count.size());

        sum_f = 0; /// CNOTs: first
        for (auto n = 0u; n < qsp_stats2.gates_count.size(); n++)
        {
            sum_f += qsp_stats2.gates_count[n].first;
        }
        double avg_f2 = sum_f / double(qsp_stats2.gates_count.size());

        sum_f = 0; /// CNOTs: first
        for (auto n = 0u; n < qsp_stats3.gates_count.size(); n++)
        {
            sum_f += qsp_stats3.gates_count[n].first;
        }
        double avg_f3 = sum_f / double(qsp_stats3.gates_count.size());

        auto imp1 = ((avg_f1 - avg_f2) / avg_f1) * 100;
        auto imp2 = ((avg_f1 - avg_f3) / avg_f1) * 100;

        exp(benchmark, function_counter, avg_f1, imp1, imp2, (qsp_stats3.total_time) );
    }

    exp.save();
    exp.table();

    return 0;
}
