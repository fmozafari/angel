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
    angel::function_extractor_params ps;
    ps.num_vars = 6;
    ps.exact_size = true;
    angel::function_extractor extractor{ps};

    auto seed = 1;
    auto idx = ps.num_vars;
    while (idx>1)
    {
        seed *= idx;
        idx--;
    }

    experiments::experiment<std::string, uint32_t, 
                            uint32_t, double, uint32_t, double, double,
                            uint32_t, double, uint32_t, double, double,
                            uint32_t, double, uint32_t, double, double, double, double > exp("qsp_cuts", "benchmark", "#functions",
                                                                               "totalC-1", "AvgC-1", "totalS-1", "AvgS-1", "time-1",
                                                                               "totalC-2", "AvgC-2", "totalS-2", "AvgS-2", "time-2",
                                                                               "totalC-3", "AvgC-3", "totalS-3", "AvgS-3", "time-3", "imp-2", "imp-3");

    for ( const auto &benchmark : experiments::epfl_benchmarks( ~experiments::hyp ) )
    {
        fmt::print( "[i] processing {}\n", benchmark );
        if ( !extractor.parse( experiments::benchmark_path( benchmark ) ) )
            continue;

        // if(benchmark!="adder")
        //     break;

        uint32_t function_counter = 0u;
        angel::qsp_general_stats qsp_stats1;
        angel::qsp_general_stats qsp_stats2;
        angel::qsp_general_stats qsp_stats3;
        extractor.run([&](kitty::dynamic_truth_table const &tt) 
        {
            (void)tt;
            
            auto n = tt.num_vars();
            ++function_counter;

            //if(function_counter<350)
            //{
                {
                    tweedledum::netlist<tweedledum::mcmt_gate> ntk;
                    angel::NoDeps deps_alg1;
                    angel::NoReordering orders1;
                    angel::qsp_tt_general(ntk, deps_alg1, orders1, tt, qsp_stats1);
                }

                {
                    tweedledum::netlist<tweedledum::mcmt_gate> ntk;
                    angel::ResubSynthesisDeps deps_alg2;
                    angel::RandomReordering orders2(seed, 1);
                    //angel::RandomReordering orders2(seed,n*n);
                    angel::qsp_tt_general(ntk, deps_alg2, orders2, tt, qsp_stats2);
                }

                {
                    tweedledum::netlist<tweedledum::mcmt_gate> ntk;
                    angel::ResubSynthesisDeps deps_alg3;
                    angel::RandomReordering orders3(seed,n*n);
                    //angel::AllReordering orders3;
                    angel::qsp_tt_general(ntk, deps_alg3, orders3, tt, qsp_stats3);
                }   
            //}
        });

        auto sum_f = 0; /// CNOTs: first
        auto sum_s = 0; /// SQgates: second
        for (auto n = 0u; n < qsp_stats1.gates_count.size(); n++)
        {
            sum_f += qsp_stats1.gates_count[n].first;
            sum_s += qsp_stats1.gates_count[n].second;
        }
        double avg_f1 = sum_f / double(qsp_stats1.gates_count.size());
        double avg_s1 = sum_s / double(qsp_stats1.gates_count.size());

        sum_f = 0; /// CNOTs: first
        sum_s = 0; /// SQgates: second
        for (auto n = 0u; n < qsp_stats2.gates_count.size(); n++)
        {
            sum_f += qsp_stats2.gates_count[n].first;
            sum_s += qsp_stats2.gates_count[n].second;
        }
        double avg_f2 = sum_f / double(qsp_stats2.gates_count.size());
        double avg_s2 = sum_f / double(qsp_stats2.gates_count.size());

        sum_f = 0; /// CNOTs: first
        sum_s = 0; /// SQgates: second
        for (auto n = 0u; n < qsp_stats3.gates_count.size(); n++)
        {
            sum_f += qsp_stats3.gates_count[n].first;
            sum_s += qsp_stats3.gates_count[n].second;
        }
        double avg_f3 = sum_f / double(qsp_stats3.gates_count.size());
        double avg_s3 = sum_s / double(qsp_stats3.gates_count.size());

        auto imp1 = ((avg_f1 - avg_f2) / avg_f1) * 100;
        auto imp2 = ((avg_f1 - avg_f3) / avg_f1) * 100;

        exp(benchmark, function_counter, qsp_stats1.total_cnots, avg_f1, qsp_stats1.total_rys+qsp_stats1.total_nots, avg_s1, angel::to_seconds(qsp_stats1.total_time),
            qsp_stats2.total_cnots, avg_f2, qsp_stats2.total_rys+qsp_stats2.total_nots, avg_s2, angel::to_seconds(qsp_stats2.total_time),
            qsp_stats3.total_cnots, avg_f3, qsp_stats3.total_rys+qsp_stats3.total_nots, avg_s3, angel::to_seconds(qsp_stats3.total_time), imp1, imp2 );
    }

    exp.save();
    exp.table();

    return 0;
}
